FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git wget curl

RUN apt-get install -y nlohmann-json3-dev

WORKDIR /app
COPY src/ ./src/
COPY CMakeLists.txt .

RUN mkdir build && cd build && cmake .. && make

EXPOSE 8080
CMD ["./build/hotel-booking-api"]
