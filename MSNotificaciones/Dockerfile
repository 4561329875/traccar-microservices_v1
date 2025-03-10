# Etapa 1: Build
FROM debian:bullseye AS builder

# Instalar dependencias de compilación y librerías necesarias
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libssl-dev \
    libjsoncpp-dev \
    libsqlite3-dev \
    libmysqlclient-dev \
    libpq-dev \
    libpqxx-dev

# Clonar, compilar e instalar Drogon (necesario para el proyecto)
RUN git clone https://github.com/an-tao/drogon.git /opt/drogon
WORKDIR /opt/drogon
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install
# Actualizar el caché de ld para encontrar las nuevas librerías
RUN ldconfig

# Copiar el código fuente del microservicio al contenedor
WORKDIR /app
COPY . /app

# Compilar el microservicio
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Etapa 2: Runtime
FROM debian:bullseye-slim

# Instalar librerías en tiempo de ejecución
RUN apt-get update && apt-get install -y \
    libcurl4 \
    openssl \
    libjsoncpp1 \
    libsqlite3-0 \
    libpq5 && \
    rm -rf /var/lib/apt/lists/*

# Copiar las librerías instaladas en la etapa de build
COPY --from=builder /usr/local/lib /usr/local/lib
# Asegurarse que el sistema conozca las librerías en /usr/local/lib
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf && ldconfig

# Copiar el binario del microservicio
COPY --from=builder /app/build/notify /usr/local/bin/notify

# Exponer el puerto en el que escucha (según lo configurado en main.cc)
EXPOSE 8080

# Ejecutar el microservicio
ENTRYPOINT ["/usr/local/bin/notify"]
