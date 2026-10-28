from kafka import KafkaConsumer
import redis
import json
from cassandra.cluster import Cluster
from train import (
    create_spark_session,
    load_and_preprocess_data,
    NCFModel,
    predict_ratings,
)
import numpy as np
import tensorflow as tf

cassandra_host = "cassandra"
cassandra_port = "9042"

cluster = Cluster(cassandra_host, cassandra_port)
session = cluster.connect("movierecs")


r = redis.Redis(host="redis", port=6379)
consumer = KafkaConsumer("ratings", bootstrap_servers="kafka:9092")

# count number of rating update
counter = 0

# Consumer Loop
while True:
    for msg in consumer:
        counter += 1
        rating = json.loads(msg.value.decode("utf-8"))
        user_id, movie_id, rating_val, timestamp = (
            rating["user_id"],
            rating["movie_id"],
            rating["rating"],
            rating["timestamp"],
        )

        session.execute(
            """
            INSERT INTO ratings (user_id, movie_id, rating, timestamp)
            VALUES (%s, %s, %s, %s);
""",
            (user_id, movie_id, rating_val, timestamp),
        )

        # update all recommendations every 10 updates
        if counter < 10:
            continue

        spark = create_spark_session(cassandra_host, cassandra_port)
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

        # reset counter
        counter = 0
