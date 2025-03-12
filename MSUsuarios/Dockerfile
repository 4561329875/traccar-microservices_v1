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
WORKDIR /tmp
RUN git clone https://github.com/CrowCpp/Crow /tmp/crow && \
    cd /tmp/crow && \
    git submodule update --init --recursive && \
    cmake . && make && make install && \
    rm -rf /tmp/crow
# Configurar directorio de trabajo
WORKDIR /usr/src/app
# Copiar los archivos de código fuente preservando la estructura de directorios
COPY main2.cpp /usr/src/app/
COPY lib/ /usr/src/app/lib/
COPY public/ /usr/src/app/public/
# Verificar que los archivos existen (para depuración)
RUN ls -la /usr/src/app/ && ls -la /usr/src/app/lib/
# Compilar el servicio con optimizaciones
RUN g++ -std=c++17 -O2 -fopenmp -o user_service main2.cpp lib/benchmark.cpp lib/jwt_funciones.c -lpqxx -lboost_system -lpthread -lcrypto -lssl

# 🌟 Fase 2: Ejecución
FROM ubuntu:22.04
# Evitar prompts interactivos
ENV DEBIAN_FRONTEND=noninteractive
# Instalar solo las dependencias necesarias para la ejecución
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpqxx-dev \
    libboost-system-dev \
    libssl-dev \
    libcurl4-openssl-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*
# Configurar directorio de trabajo
WORKDIR /usr/src/app
# Copiar el binario desde la fase de compilación
COPY --from=builder /usr/src/app/user_service .
# Copiar el directorio "public" con los archivos HTML
COPY --from=builder /usr/src/app/public ./public
# Copiar la carpeta lib (si es necesaria en tiempo de ejecución)
COPY --from=builder /usr/src/app/lib ./lib
# Crear un directorio para los logs de benchmark
RUN mkdir -p /usr/src/app/logs && chmod 777 /usr/src/app/logs
# Crear el directorio para los tokens y declarar el volumen para persistirlos
RUN mkdir -p /usr/src/app/tokens && chmod 777 /usr/src/app/tokens
VOLUME [ "/usr/src/app/tokens" ]
# Exponer el puerto
EXPOSE 8080
# Variables de entorno para configuración (ejemplo)
ENV DB_HOST=postgres-container \
    DB_PORT=5432 \
    DB_NAME=userdb \
    DB_USER=postgres \
    DB_PASSWORD=postgres \
    SERVICE_PORT=8080
# Usuario no root para mayor seguridad
RUN groupadd -r appuser && useradd -r -g appuser appuser
RUN chown -R appuser:appuser /usr/src/app
USER appuser
# Punto de entrada para el servicio
CMD ["./user_service"]

