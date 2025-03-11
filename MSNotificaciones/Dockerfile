# Etapa 1: Construcción
FROM debian:bullseye AS builder

# Instalar dependencias del sistema
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libssl-dev \
    libjsoncpp-dev \
    libsqlite3-dev \
    libmariadb-dev \
    libmariadb-dev-compat \
    libpq-dev \
    libpqxx-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

# Clonar Drogon con la opción --recursive para incluir Trantor
RUN git clone --recursive https://github.com/an-tao/drogon.git /opt/drogon

WORKDIR /opt/drogon

# Compilar Drogon
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install

# Copiar el código del microservicio
WORKDIR /app
COPY . /app

# Compilar el microservicio
RUN rm -rf build && mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Etapa 2: Imagen Final
FROM debian:bullseye-slim

# Instalar solo las librerías necesarias en tiempo de ejecución
RUN apt-get update && apt-get install -y \
    libcurl4 \
    openssl \
    libjsoncpp-dev \
    libsqlite3-0 \
    libpq5 \
    uuid-runtime \
    && rm -rf /var/lib/apt/lists/*



# Copiar librerías de Drogon desde la etapa de compilación
COPY --from=builder /usr/local/lib /usr/local/lib
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf && ldconfig

# Copiar el binario del microservicio
COPY --from=builder /app/build/notify /usr/local/bin/notify

# Copiar archivos de configuración
COPY config.json /app/config.json
COPY config.yaml /app/config.yaml

# Exponer el puerto donde escucha Drogon
EXPOSE 9085

# Ejecutar el microservicio
ENTRYPOINT ["/usr/local/bin/notify"]

