 
# Usa una imagen base con Node.js
FROM node:20-alpine

# Establece el directorio de trabajo dentro del contenedor
WORKDIR /app

# Copia los archivos del proyecto al contenedor
COPY package.json ./

# Instala las dependencias
RUN npm install

# Copia el resto del código
COPY . ./

# Expone el puerto en el que corre la aplicación (ajústalo según tu proyecto)
EXPOSE 3000

# Comando para iniciar la aplicación
CMD ["npm", "start"]
