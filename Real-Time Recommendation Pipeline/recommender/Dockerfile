FROM python:3.9-slim
WORKDIR /app
RUN apt-get update && \
    apt-get install -y openjdk-17-jdk && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV PATH=$JAVA_HOME/bin:$PATH
COPY requirements.txt .
RUN pip install -r requirements.txt
COPY . .
RUN python train.py
CMD ["python", "consumer.py"]