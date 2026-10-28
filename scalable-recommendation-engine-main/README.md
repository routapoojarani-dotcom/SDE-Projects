# Real-Time Recommendation Pipeline

A production-like recommendation system built with a modern, scalable tech stack. This project demonstrates real-time rating ingestion, distributed data processing, and neural collaborative filtering (NCF) to deliver personalized movie recommendations. Designed to mimic systems at companies like Netflix or TikTok, it leverages Dockerized microservices, event streaming, and deep learning.

## Features
- **Real-Time Rating Ingestion**: Users submit ratings via a REST API, processed asynchronously.
- **Scalable Architecture**: Event-driven with Kafka, persistent storage in Cassandra, and caching in Redis.
- **Advanced Recommendations**: Neural Collaborative Filtering (NCF) model from TensorFlow Recommenders.
- **Big Data Processing**: PySpark for preprocessing large datasets.
- **Containerized Deployment**: Fully Dockerized for easy setup and scalability.

## Tech Stack
- **Backend**: Spring Boot (Java) with Spring Data Cassandra (ORM)
- **Event Streaming**: Apache Kafka
- **Database**: Apache Cassandra
- **Caching**: Redis
- **Data Processing**: PySpark
- **Machine Learning**: TensorFlow Recommenders (NCF)
- **Containerization**: Docker + Docker Compose

## Architecture

```mermaid
graph TD
    A[User] -->|POST /rate| B(Spring Boot API)
    A -->|GET /recommend| B

    B -->|Publishes Rating| C(Apache Kafka)
    B -->|Fetches Recommendations| D(Redis)
    D -->|Returns Cached Results| B

    C -->|Streams Ratings| E(Python Consumer)
    E -->|Stores Ratings| F(Apache Cassandra)
    E -->|Updates Recommendations| D

    F -->|Periodic Batch Read| G(PySpark Preprocessing)
    G -->|Preprocessed Data| H(TensorFlow NCF Model)
    H -->|Trained Model Updates| E

    subgraph Docker Environment
        B
        C
        D
        E
        F
        G
        H
    end
```

1. **Spring Boot API**: Handles `POST /rate` (sends ratings to Kafka) and `GET /recommend` (fetches from Redis).
2. **Kafka**: Queues ratings for async processing.
3. **Cassandra**: Stores ratings persistently.
4. **PySpark**: Preprocesses ratings into training data.
5. **TensorFlow NCF**: Trains a neural model and generates recommendations.
6. **Redis**: Caches recommendations for low-latency access.

## Prerequisites
- Docker & Docker Compose
- Java 17 (for Spring Boot development)
- Python 3.9+ (for consumer scripts)
- Maven (for building Spring Boot)

## Project Structure
```
scalable-recommendation-engine/
├── api/                    # Spring Boot API (Java)
│   ├── src/                # API source code
│   └── Dockerfile          # API container config
├── recommender/            # Python consumer (PySpark + TensorFlow)
│   ├── consumer.py         # Kafka consumer
│   ├── train.py            # PySpark preprocessing and NCF logic
│   └── Dockerfile          # Consumer container config
├── docker-compose.yml      # Docker Compose configuration
└── README.md               # This file
```

## Future Improvements
- Add load balancing for Spring Boot API.
- Implement model retraining endpoint.
- Scale Kafka with multiple partitions and consumers.
- Enhance NCF with user/movie metadata.