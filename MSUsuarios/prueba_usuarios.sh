#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# URL base
BASE_URL="http://localhost:9081"

echo -e "${BLUE}=== Script de prueba para los endpoints de la API ===${NC}"
echo -e "${BLUE}=== Asegúrate que el servidor esté en ejecución en localhost:9081 ===${NC}\n"

# Función para separar las pruebas
separator() {
    echo -e "\n${BLUE}--------------------------------------------------------${NC}\n"
}

# 1. Probar página principal (GET /)
echo -e "${BLUE}Probando la página principal (GET /)${NC}"
curl -s -o /dev/null -w "%{http_code}" $BASE_URL/
if [ $? -eq 0 ]; then
    echo -e " - ${GREEN}Solicitud exitosa${NC}"
else
    echo -e " - ${RED}Error en la solicitud${NC}"
fi
separator

# 2. Probar página de login (GET /login)
echo -e "${BLUE}Probando la página de login (GET /login)${NC}"
curl -s -o /dev/null -w "%{http_code}" $BASE_URL/login
if [ $? -eq 0 ]; then
    echo -e " - ${GREEN}Solicitud exitosa${NC}"
else
    echo -e " - ${RED}Error en la solicitud${NC}"
fi
separator

# 3. Probar listado de usuarios (GET /users/list)
echo -e "${BLUE}Probando listado de usuarios (GET /users/list)${NC}"
curl -s $BASE_URL/users/list
echo -e "\n"
separator

# 4. Probar creación de usuario (POST /users/create)
echo -e "${BLUE}Probando creación de usuario (POST /users/create)${NC}"
USER_JSON='{"name":"moya","email":"dilanm@example.com","hashedpassword":"2024"}'
RESPONSE=$(curl -s -X POST -H "Content-Type: application/json" -d "$USER_JSON" $BASE_URL/users/create)
echo "$RESPONSE"

# Extraer el token JWT de la respuesta (asumiendo que está en el formato "... Token: [token]")
TOKEN=$(echo "$RESPONSE" | jq -r '.token')
echo -e "\nToken extraído: $TOKEN"
separator

# 5. Probar login de usuario (POST /users/login)
echo -e "${BLUE}Probando login de usuario (POST /users/login)${NC}"
LOGIN_JSON='{"email":"dilanm@example.com","hashedpassword":"2024"}'
curl -s -X POST -H "Content-Type: application/json" -d "$LOGIN_JSON" $BASE_URL/users/login
echo -e "\n"
separator

# 6. Probar autenticación JWT (POST /auth/jwt)
echo -e "${BLUE}Probando autenticación JWT (POST /auth/jwt)${NC}"
if [ -n "$TOKEN" ]; then
    JWT_JSON='{"token":"'$TOKEN'"}'
    curl -s -X POST -H "Content-Type: application/json" -d "$JWT_JSON" $BASE_URL/auth/jwt
else
    JWT_JSON='{"token":"token_invalido"}'
    echo "Usando token inválido porque no se pudo extraer uno válido"
    curl -s -X POST -H "Content-Type: application/json" -d "$JWT_JSON" $BASE_URL/auth/jwt
fi
echo -e "\n"
separator

# 7. Probar validación de permisos (POST /auth/permissions)
echo -e "${BLUE}Probando validación de permisos (POST /auth/permissions)${NC}"
if [ -n "$TOKEN" ]; then
    PERM_JSON='{"token":"'$TOKEN'","permission":"read"}'
    curl -s -X POST -H "Content-Type: application/json" -d "$PERM_JSON" $BASE_URL/auth/permissions
else
    PERM_JSON='{"token":"token_invalido","permission":"read"}'
    echo "Usando token inválido porque no se pudo extraer uno válido"
    curl -s -X POST -H "Content-Type: application/json" -d "$PERM_JSON" $BASE_URL/auth/permissions
fi
echo -e "\n"
separator

# 8. Probar acceso a archivo estático (GET /cualquier_archivo_en_public)
echo -e "${BLUE}Probando acceso a archivo estático (GET /archivo_ejemplo)${NC}"
curl -s -o /dev/null -w "%{http_code}" $BASE_URL/createUser.html
if [ $? -eq 0 ]; then
    echo -e " - ${GREEN}Solicitud exitosa${NC}"
else
    echo -e " - ${RED}Error en la solicitud${NC}"
fi
separator

echo -e "${GREEN}¡Pruebas completadas!${NC}"
