# Etapa de construcción
FROM node:20-alpine AS build

WORKDIR /app

# Copia los archivos de dependencias
COPY package.json package-lock.json ./
RUN npm install

# Copia el código fuente
COPY . ./

# Realiza el build
RUN npm run build

# Etapa de producción: Servir los archivos estáticos
FROM node:20-alpine

WORKDIR /app

# Instala 'serve' globalmente
RUN npm install -g http-server
# Copia los archivos generados desde la etapa anterior
COPY --from=build /app/build /app/build

# Expone el puerto 3000 por defecto (puede ser modificado mediante variable de entorno)
EXPOSE 3000

# Comando para iniciar 'serve', usando la variable de entorno PORT si está definida
CMD ["http-server", "build", "-p", "3000", "--verbose"]
