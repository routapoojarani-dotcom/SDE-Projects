CREATE KEYSPACE IF NOT EXISTS movierecs WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
CREATE TABLE IF NOT EXISTS movierecs.ratings (user_id int, movie_id int, rating float,
timestamp bigint, primary key (user_id, movie_id));
COPY movierecs.ratings (user_id, movie_id, rating, timestamp) FROM '/data/ratings_small.csv' WITH HEADER = TRUE;