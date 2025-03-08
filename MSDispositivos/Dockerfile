 

 #complilacion
# Usa Debian 12slim como imagen base
FROM debian:12-slim AS build

# Actualiza e instala dependencias necesarias
RUN apt update
RUN apt install -y \
    g++ cmake make wget libboost-all-dev libasio-dev

RUN apt install -y libpq-dev postgresql-client
RUN apt install -y nlohmann-json3-dev

RUN apt install -y linux-perf

RUN wget -q https://github.com/CrowCpp/Crow/releases/download/v1.2.0/Crow-1.2.0-Linux.deb
RUN dpkg -i Crow-1.2.0-Linux.deb
RUN  rm Crow-1.2.0-Linux.deb

RUN rm -rf /var/lib/apt/lists/*


# Crea el directorio de la aplicación
WORKDIR /app

COPY . ./
# Compila la API
RUN cmake . && make



#ejecucion
FROM debian:12-slim AS runtime

# Actualiza e instala dependencias necesarias
RUN apt update
#RUN apt install -y linux-perf   #solo en casos de nesesitar medir rendimiento con perf
RUN apt install -y --no-install-recommends\
    libpq-dev postgresql-client libgomp1 &&\
    apt clean && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /app/MicorsevicioDispocitivosDrivers /app/
# Expone el puerto 8080
EXPOSE 8080

CMD ./MicorsevicioDispocitivosDrivers
