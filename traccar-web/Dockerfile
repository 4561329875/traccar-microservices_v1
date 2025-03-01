# Etapa de construcción
FROM node:20-alpine

WORKDIR /app

# Copia los archivos de dependencias
COPY package.json package-lock.json ./
RUN npm install

# Copia el código fuente
COPY . ./

# Realiza el build
RUN npm run build


# Expone el puerto 3000 por defecto (puede ser modificado mediante variable de entorno)
EXPOSE 3000

# Comando para iniciar 'serve', usando la variable de entorno PORT si está definida
CMD ["npm", "run", "preview"]
