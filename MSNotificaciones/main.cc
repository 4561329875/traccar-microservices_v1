#include <drogon/drogon.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <memory>
#include <curl/curl.h>
#include <iostream>
#include <string>

// Función para enviar correo usando libcurl vía SMTP
bool sendEmail(const std::string &smtpUrl, 
               const std::string &from, 
               const std::string &to, 
               const std::string &subject, 
               const std::string &body)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error al inicializar CURL" << std::endl;
        return false;
    }

    // Construir el mensaje de correo
    std::string payload = 
        "To: " + to + "\r\n" +
        "From: " + from + "\r\n" +
        "Subject: " + subject + "\r\n" +
        "\r\n" +
        body + "\r\n";

    // Variable para llevar el control de la lectura del payload
    size_t payloadPosition = 0;
    auto payloadSize = payload.size();

    // Función de callback para leer el payload
    auto payloadSource = [](char *ptr, size_t size, size_t nmemb, void *userp) -> size_t {
        std::pair<const std::string*, size_t>* data = 
            reinterpret_cast<std::pair<const std::string*, size_t>*>(userp);
        size_t maxSize = size * nmemb;
        size_t remaining = data->first->size() - data->second;
        size_t copySize = (remaining < maxSize) ? remaining : maxSize;
        if(copySize > 0)
        {
            memcpy(ptr, data->first->c_str() + data->second, copySize);
            data->second += copySize;
        }
        return copySize;
    };

    std::pair<const std::string*, size_t> payloadData(&payload, 0);

    // Configurar CURL
    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    // Para enviar comandos SMTP se usa la opción UPLOAD
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // Establecer la dirección de origen
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

    // Lista de destinatarios
    struct curl_slist *recipients = nullptr;
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // Configurar la función de lectura del payload
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &payloadData);

    // Realizar el envío
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Error en curl_easy_perform(): " 
                  << curl_easy_strerror(res) << std::endl;
    }

    // Liberar recursos
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

// Controlador de Drogon para manejar las solicitudes de envío de correo
class EmailController : public drogon::HttpController<EmailController>
{
public:
    METHOD_LIST_BEGIN
    // Define el endpoint /send para solicitudes POST
    METHOD_ADD(EmailController::sendEmailNotification, "/send", drogon::Post);
    METHOD_LIST_END

    void sendEmailNotification(const drogon::HttpRequestPtr &req,
                               std::function<void(const std::shared_ptr<drogon::HttpResponse> &)> &&callback)
    {
        auto json = req->getJsonObject();
        if (!json)
        {
            std::shared_ptr<drogon::HttpResponse> resp = drogon::HttpResponse::newHttpJsonResponse("JSON inválido");
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }

        try {
            // Extraer campos necesarios
            std::string to = (*json)["to"].asString();
            std::string subject = (*json)["subject"].asString();
            std::string body = (*json)["body"].asString();

            // Ajustar la dirección 'from' a tu correo de Gmail
            std::string from = "notificationservice8080@gmail.com";

            // Usar SMTP de Gmail en el puerto 587 (STARTTLS)
            std::string smtpUrl = "smtp://smtp.gmail.com:587";

            // Inicializar CURL
            CURL *curl = curl_easy_init();
            if (!curl) {
                std::shared_ptr<drogon::HttpResponse> resp = drogon::HttpResponse::newHttpJsonResponse("Error al inicializar CURL");
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
                return;
            }

            // Configurar autenticación SMTP (tu correo + contraseña de aplicación)
            curl_easy_setopt(curl, CURLOPT_USERNAME, "notificationservice8080@gmail.com");
            curl_easy_setopt(curl, CURLOPT_PASSWORD, "qeng cmas tneh vpvk");
            // Usar TLS/SSL
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

            bool success = sendEmail(smtpUrl, from, to, subject, body);
            curl_easy_cleanup(curl);

            if (success)
            {
                std::shared_ptr<drogon::HttpResponse> resp = drogon::HttpResponse::newHttpJsonResponse("Correo enviado correctamente");
                callback(resp);
            }
            else
            {
                std::shared_ptr<drogon::HttpResponse> resp = drogon::HttpResponse::newHttpJsonResponse("Error al enviar el correo");
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
            }
        }
        catch (const std::exception &e)
        {
            std::shared_ptr<drogon::HttpResponse> resp = drogon::HttpResponse::newHttpJsonResponse("Excepción: " + std::string(e.what()));
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    }
};

int main()
{
    // Configurar el puerto y la dirección de escucha
    drogon::app().addListener("0.0.0.0", 9085);
    // Ejecutar la aplicación Drogon
    drogon::app().run();
    return 0;
}
