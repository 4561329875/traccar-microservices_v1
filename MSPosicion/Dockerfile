# -----------------------------------------
# Etapa 1: Compilación
# -----------------------------------------
FROM ubuntu:22.04 AS builder

# Instalar dependencias necesarias para compilar C++ y CMake
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    libpq-dev \
    libpqxx-dev \
    git \
    uuid-dev \
    zlib1g-dev \
    libssl-dev \
    libjsoncpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Instalar Drogon
RUN git clone --branch v1.8.2 --depth 1 https://github.com/drogonframework/drogon.git /tmp/drogon && \
    cd /tmp/drogon && \
    git submodule update --init --recursive && \
    mkdir build && cd build && \
    cmake .. && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/drogon

# Crear directorio de la aplicación
WORKDIR /app

# Copiar archivos del proyecto al contenedor
COPY . /app

# Crear directorio de compilación y compilar
RUN rm -rf build && \
    mkdir -p build && \
    cmake -B build -S . -DCMAKE_PREFIX_PATH=/usr/local/lib/cmake && \
    cmake --build build

# -----------------------------------------
# Etapa 2: Ejecución
# -----------------------------------------
FROM ubuntu:22.04 AS runner

# Instalar solo las dependencias necesarias para la ejecución
RUN apt-get update && apt-get install -y \
    libpq-dev \
    libpqxx-dev \
    libssl-dev \
    libjsoncpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio de la aplicación
WORKDIR /app

# Copiar el binario compilado desde la etapa de compilación
COPY --from=builder /app/build/microservice-positions /app/microservice-positions

# Exponer el puerto en el que corre el servicio
EXPOSE 8080

# Definir el comando de ejecución del contenedor
CMD ["/app/microservice-positions"]