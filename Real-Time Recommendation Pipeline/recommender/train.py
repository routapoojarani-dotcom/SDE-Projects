# Import necessary libraries
from pyspark.sql import SparkSession
from pyspark.sql.functions import col
import tensorflow as tf
import tensorflow_recommenders as tfrs
import numpy as np
from pyspark.sql.types import (
    StructType,
    StructField,
    FloatType,
    IntegerType,
    StringType,
    TimestampType,
)
from io import BytesIO
import uuid
from datetime import datetime
import redis
import json


def create_spark_session(cassandra_host="cassandra", cassandra_port="9042"):
    spark = (
        SparkSession.builder.appName("Movie Recommendation System")
        .config("spark.cassandra.connection.host", cassandra_host)
        .config("spark.cassandra.connection.port", cassandra_port)
        .config(
            "spark.jars.packages",
            "com.datastax.spark:spark-cassandra-connector_2.12:3.2.0",
        )
        .getOrCreate()
    )
    return spark


def load_and_preprocess_data(spark):
    # schema = StructType(
    #     [
    #         StructField("user_id", IntegerType(), False),
    #         StructField("movie_id", IntegerType(), False),
    #         StructField("rating", FloatType(), True),
    #     ]
    # )
    df = (
        spark.read.format("org.apache.spark.sql.cassandra").options(
            table="ratings", keyspace="movierecs"
        )
        # .schema(schema)
        .load()
    )
    df = df.na.drop()
    df = df.withColumn("user_id", col("user_id").cast("string"))
    df = df.withColumn("movie_id", col("movie_id").cast("string"))
    df = df.withColumn("rating", col("rating").cast("float"))

    pandas_df = df.select("user_id", "movie_id", "rating").toPandas()
    ratings = tf.data.Dataset.from_tensor_slices(
        {
            "user_id": pandas_df["user_id"].values,
            "movie_id": pandas_df["movie_id"].values,
            "rating": pandas_df["rating"].values,
        }
    )

    return ratings, pandas_df


def save_model_to_cassandra(spark, model):
    # Save model to bytes
    model_bytes_io = BytesIO()
    model.save(model_bytes_io)
    model_bytes = model_bytes_io.getvalue()

    model_data = [
        (uuid.uuid4(), model_bytes, datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    ]  # Using ID 1 as example
    schema = StructType(
        [
            StructField("model_id", StringType(), False),
            StructField(
                "model_binary", StringType(), True
            ),  # Will be stored as blob in Cassandra
            StructField("created_at", TimestampType(), True),
        ]
    )

    model_df = spark.createDataFrame(model_data, schema)

    # Write to Cassandra
    model_df.write.format("org.apache.spark.sql.cassandra").mode("append").options(
        table="ncf", keyspace="movierecs"
    ).save()

    print(f"Model saved to Cassandra")


class NCFModel(tfrs.Model):
    def __init__(self, unique_user_ids, unique_movie_ids, embedding_size=32):
        super().__init__()

        self.user_embedding = tf.keras.Sequential(
            [
                tf.keras.layers.StringLookup(
                    vocabulary=unique_user_ids, mask_token=None
                ),
                tf.keras.layers.Embedding(len(unique_user_ids) + 1, embedding_size),
            ]
        )

        self.movie_embedding = tf.keras.Sequential(
            [
                tf.keras.layers.StringLookup(
                    vocabulary=unique_movie_ids, mask_token=None
                ),
                tf.keras.layers.Embedding(len(unique_movie_ids) + 1, embedding_size),
            ]
        )

        self.rating_task = tfrs.tasks.Ranking(
            loss=tf.keras.losses.MeanSquaredError(),
            metrics=[tf.keras.metrics.RootMeanSquaredError()],
        )

    def compute_loss(self, features, training=False):
        user_embeddings = self.user_embedding(features["user_id"])
        movie_embeddings = self.movie_embedding(features["movie_id"])
        predictions = tf.reduce_sum(user_embeddings * movie_embeddings, axis=1)
        return self.rating_task(labels=features["rating"], predictions=predictions)

    def call(self, features, training=False):
        """Method for inference"""
        user_embeddings = self.user_embedding(features["user_id"])
        movie_embeddings = self.movie_embedding(features["movie_id"])
        return tf.reduce_sum(user_embeddings * movie_embeddings, axis=1)


def predict_ratings(model, user_ids, movie_ids):
    """Make predictions for given user-movie pairs"""
    # Convert inputs to tensor
    inference_data = {
        "user_id": tf.constant(user_ids, dtype=tf.string),
        "movie_id": tf.constant(movie_ids, dtype=tf.string),
    }

    # Get predictions
    predictions = model(inference_data)
    return predictions.numpy()


def save_model(model, model_path):
    model.save(model_path, save_format="tf")


# 4. Main execution
def main():
    cassandra_host = "cassandra"
    cassandra_port = "9042"

    print(f"Connecting to Cassandra at {cassandra_host}:{cassandra_port}")
    spark = create_spark_session(cassandra_host, cassandra_port)

    print("Loading and preprocessing data...")
    ratings_dataset, pandas_df = load_and_preprocess_data(spark)

    unique_user_ids = np.unique(pandas_df["user_id"].values)
    unique_movie_ids = np.unique(pandas_df["movie_id"].values)

    # Split data
    ratings_dataset = ratings_dataset.shuffle(100_000, seed=42)
    train_size = int(0.8 * pandas_df.shape[0])
    train = ratings_dataset.take(train_size).batch(4096).cache()
    test = ratings_dataset.skip(train_size).batch(4096).cache()

    # Build and train model
    print("Building and training NCF model...")
    model = NCFModel(unique_user_ids, unique_movie_ids)
    model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.01))

    model.fit(train, validation_data=test, epochs=10, verbose=1)

    # Evaluate
    evaluation = model.evaluate(test, return_dict=True)
    print(f"Evaluation results: {evaluation}")

    save_model(model, "./ncf_mode.tf")

    r = redis.Redis(host="redis", port=6379)

    for user_id in unique_user_ids:
        predictions = predict_ratings(
            model, [user_id] * len(unique_movie_ids), unique_movie_ids
        )
        top_movies = unique_movie_ids[
            tf.nn.top_k(predictions, k=10).indices.numpy()
        ].tolist()
        r.setex(
            f"user:{user_id}",
            3600,
            json.dumps({"user_id": user_id, "recommendations": top_movies}),
        )

    spark.stop()


# 5. Function to load and use saved model for inference
def load_and_predict(model_path, user_ids, movie_ids):
    """Load saved model and make predictions"""
    loaded_model = tf.saved_model.load(model_path)

    # Convert inputs to tensor
    inference_data = {
        "user_id": tf.constant(user_ids, dtype=tf.string),
        "movie_id": tf.constant(movie_ids, dtype=tf.string),
    }

    # Get predictions using the loaded model's serving function
    predictions = loaded_model(inference_data)
    return predictions.numpy()


if __name__ == "__main__":
    # Calculate the recommendations and store in redis
    main()
