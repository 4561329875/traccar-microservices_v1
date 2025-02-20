# Usa Debian 12 como imagen base
FROM debian:12

# Actualiza e instala dependencias necesarias
RUN apt update
RUN apt install -y \
    g++ cmake make wget libboost-all-dev libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# Descarga e instala Crow desde el archivo .deb
RUN wget -q https://github.com/CrowCpp/Crow/releases/download/v1.2.0/Crow-1.2.0-Linux.deb
RUN dpkg -i Crow-1.2.0-Linux.deb
RUN  rm Crow-1.2.0-Linux.deb

# Crea el directorio de la aplicación
WORKDIR /app

# Copia los archivos del proyecto
COPY main.cpp CMakeLists.txt ./

# Compila la API
RUN cmake . && make

# Expone el puerto 8080
EXPOSE 8080

# Ejecuta el servidor
CMD ["./MSPosicion"]
