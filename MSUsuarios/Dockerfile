# 🌟 Fase 1: Compilación (Builder)
FROM ubuntu:22.04 AS builder

# Evitar prompts interactivos en apt-get
ENV DEBIAN_FRONTEND=noninteractive

# Instalar solo las dependencias necesarias para la compilación
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libboost-all-dev \
    libpqxx-dev \
    cmake \
    g++ \
    git \
    curl \
    libssl-dev \
    libcurl4-openssl-dev \
    libasio-dev \
    iputils-ping \
    ca-certificates && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Configurar el buffer de Git para evitar problemas con la red
RUN git config --global http.postBuffer 524288000

# Descargar e instalar Crow Framework
RUN git clone https://github.com/CrowCpp/Crow /crow && \
    cd /crow && \
    git submodule update --init --recursive && \
    cmake . && make && make install

# Configurar directorio de trabajo y copiar código fuente
WORKDIR /usr/src/app
COPY . .

# Compilar el servicio
RUN g++ -std=c++17 -o user_service main.cpp jwt_funciones.c -lpqxx -lboost_system -lpthread

# 🌟 Fase 2: Ejecución
FROM ubuntu:22.04

# Instalar solo las dependencias necesarias para la ejecución
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpqxx-dev \
    libboost-system-dev \
    libssl-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Configurar directorio de trabajo
WORKDIR /usr/src/app

# Copiar el binario desde la fase de compilación
COPY --from=builder /usr/src/app/user_service .

# Exponer el puerto
EXPOSE 8080

# Definir comando de inicio
CMD ["./user_service"]

