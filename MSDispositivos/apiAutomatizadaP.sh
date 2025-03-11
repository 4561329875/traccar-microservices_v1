#!/bin/bash

BASE_URL="http://localhost:9082/api"

# Crear 5 dispositivos
echo "Creando 5 dispositivos..."
curl -X POST "$BASE_URL/devices" -H "Content-Type: application/json" -d '{"id": 11, "name": "Dispositivo 1", "uniqueId": "D1"}'
echo -e "\n"
curl -X POST "$BASE_URL/devices" -H "Content-Type: application/json" -d '{"id": 21, "name": "Dispositivo 2", "uniqueId": "D2"}'
echo -e "\n"
curl -X POST "$BASE_URL/devices" -H "Content-Type: application/json" -d '{"id": 31, "name": "Dispositivo 3", "uniqueId": "D3"}'
echo -e "\n"
curl -X POST "$BASE_URL/devices" -H "Content-Type: application/json" -d '{"id": 41, "name": "Dispositivo 4", "uniqueId": "D4"}'
echo -e "\n"
curl -X POST "$BASE_URL/devices" -H "Content-Type: application/json" -d '{"id": 51, "name": "Dispositivo 5", "uniqueId": "D5"}'
echo -e "\n"

# Obtener todos los dispositivos
echo "Obteniendo todos los dispositivos..."
curl -X GET "$BASE_URL/devices"
echo -e "\n"

# Obtener un dispositivo por ID
echo "Obteniendo dispositivo con ID 1..."
curl -X GET "$BASE_URL/devices/1"
echo -e "\n"

# Crear 5 conductores
echo "Creando 5 conductores..."
curl -X POST "$BASE_URL/drivers" -H "Content-Type: application/json" -d '{"id": 11, "name": "Conductor 1", "uniqueId": "C1","attributes": {}}'
echo -e "\n"
curl -X POST "$BASE_URL/drivers" -H "Content-Type: application/json" -d '{"id": 21, "name": "Conductor 2", "uniqueId": "C2","attributes": {}}'
echo -e "\n"
curl -X POST "$BASE_URL/drivers" -H "Content-Type: application/json" -d '{"id": 31, "name": "Conductor 3", "uniqueId": "C3","attributes": {}}'
echo -e "\n"
curl -X POST "$BASE_URL/drivers" -H "Content-Type: application/json" -d '{"id": 41, "name": "Conductor 4", "uniqueId": "C4","attributes": {}}'
echo -e "\n"
curl -X POST "$BASE_URL/drivers" -H "Content-Type: application/json" -d '{"id": 51, "name": "Conductor 5", "uniqueId": "C5","attributes": {}}'
echo -e "\n"

# Obtener todos los conductores
echo "Obteniendo todos los conductores..."
curl -X GET "$BASE_URL/drivers"
echo -e "\n"

# Obtener un conductor por ID
echo "Obteniendo conductor con ID 1..."
curl -X GET "$BASE_URL/drivers/1"
echo -e "\n"

# Actualizar un dispositivo
echo "Actualizando el dispositivo con ID 2..."
curl -X PUT "$BASE_URL/devices/21" -H "Content-Type: application/json" -d '{"name": "Dispositivo Actualizado"}'
echo -e "\n"

# Eliminar un dispositivo
echo "Eliminando el dispositivo con ID 3..."
curl -X DELETE "$BASE_URL/devices/31"
echo -e "\n"

# Actualizar un conductor
echo "Actualizando el conductor con ID 1..."
curl -X PUT "$BASE_URL/drivers/11" -H "Content-Type: application/json" -d '{"name": "Conductor Modificado"}'
echo -e "\n"

# Eliminar un conductor
echo "Eliminando el conductor con ID 2..."
curl -X DELETE "$BASE_URL/drivers/21"
echo -e "\n"

# Actualizar acumuladores
#echo "Actualizando acumuladores para dispositivo 1..."
#curl -X PUT "$BASE_URL/drivers/2/accumulators" -H "Content-Type: application/json" -d '{"deviceId": 2,"totalDistance": 100,"hours": 50 }'
#echo -e "\n"

# Subir imagen a un dispositivo
echo "Subiendo imagen para dispositivo 1..."
curl -X POST "$BASE_URL/devices/11/image" -H "Content-Type: image/jpg" --data-binary @carroAmarilloD1.jpg
echo -e "\n"

# Obtener imagen
echo "Obteniendo imagen del dispositivo 1..."
curl -X GET "$BASE_URL/media/C1/device.jpg" --output dispositivo.jpg
echo -e "\n"
