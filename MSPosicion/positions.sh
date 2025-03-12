BASE_URL="http://localhost:9083"

# Crear 5000 posiciones
echo "Creando 5000 posiciones..."
for i in {1..5000}
do
  curl -X POST "$BASE_URL/api/positions" -H "Content-Type: application/json" -d "{\"id\": $i, \"protocol\": \"TCP\", \"deviceId\": $i, \"serverTime\": \"2025-03-11T12:00:00Z\", \"deviceTime\": \"2025-03-11T12:00:00Z\", \"fixTime\": \"2025-03-11T12:00:00Z\", \"valid\": true, \"latitude\": $i.0, \"longitude\": $i.0, \"altitude\": 100.0, \"speed\": 10.5, \"course\": 180.0, \"address\": \"Direccion $i\", \"attributes\": \"{}\", \"accuracy\": 5.0, \"network\": \"GSM\"}"
  echo -e "\n"
done

# Obtener todas las posiciones
echo "Obteniendo todas las posiciones..."
curl -X GET "$BASE_URL/api/positions"
echo -e "\n"

# Actualizar 5000 posiciones
echo "Actualizando 5000 posiciones..."
for i in {1..5000}
do
  curl -X PUT "$BASE_URL/api/positions/$i" -H "Content-Type: application/json" -d "{\"protocol\": \"UDP\", \"deviceId\": $i, \"serverTime\": \"2025-03-11T12:30:00Z\", \"deviceTime\": \"2025-03-11T12:30:00Z\", \"fixTime\": \"2025-03-11T12:30:00Z\", \"valid\": false, \"latitude\": $i.1, \"longitude\": $i.1, \"altitude\": 150.0, \"speed\": 15.0, \"course\": 90.0, \"address\": \"Direccion Actualizada $i\", \"attributes\": \"{}\", \"accuracy\": 3.5, \"network\": \"LTE\"}"
  echo -e "\n"
done

# Eliminar 5000 posiciones
echo "Eliminando 5000 posiciones..."
for i in {1..5000}
do
  curl -X DELETE "$BASE_URL/api/positions/$i"
  echo -e "\n"
done
