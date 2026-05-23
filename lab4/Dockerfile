FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j "$(nproc)"

CMD ["./build/hotel-booking-api", "--config", "configs/static_config.yaml", "--config-vars", "configs/config_vars.docker.yaml"]
