#!/bin/bash

# ✅ Parámetros
SMTP_SERVER="smtp://smtp.gmail.com:587"
SMTP_USER="notificationservice8080@gmail.com"
SMTP_PASS="qeng cmas tneh vpvk"
TO_EMAIL="hallofrank2003@gmail.com"
SUBJECT="Prueba"
BODY="Este es un mensaje de prueba."

# 📌 Archivo temporal para el contenido del correo
EMAIL_FILE=$(mktemp)
echo -e "To: $TO_EMAIL\nFrom: $SMTP_USER\nSubject: $SUBJECT\n\n$BODY" > "$EMAIL_FILE"

# 📨 OPCIÓN 1: Enviar sin el microservicio (directo con `curl`)
echo "🔹 Enviando correo."
curl --url "$SMTP_SERVER" --ssl-reqd \
     --mail-from "$SMTP_USER" \
     --mail-rcpt "$TO_EMAIL" \
     --upload-file "$EMAIL_FILE" \
     --user "$SMTP_USER:$SMTP_PASS" \
     --verbose

# 📨 OPCIÓN 2: Enviar usando el microservicio
echo "🔹"
curl -X POST http://127.0.0.1:9085/send -H "Content-Type: application/json" \
     -d "{\"to\":\"$TO_EMAIL\", \"subject\":\"$SUBJECT\", \"body\":\"$BODY\"}"

# 🗑️ Eliminar archivo temporal
rm "$EMAIL_FILE"

