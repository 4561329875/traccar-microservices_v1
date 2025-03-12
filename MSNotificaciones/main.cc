#include <drogon/drogon.h>
#include <drogon/HttpController.h>
#include <curl/curl.h>
#include <iostream>
#include <string>

// ✅ Función para enviar correo usando libcurl vía SMTP con STARTTLS
bool sendEmail(const std::string &from, 
               const std::string &to, 
               const std::string &subject, 
               const std::string &body)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "❌ Error al inicializar CURL" << std::endl;
        return false;
    }

    std::string smtpUrl = "smtp://smtp.gmail.com:587";  // ✅ STARTTLS usa este puerto

    std::string payload =
    "To: " + to + "\r\n"
    "From: " + from + "\r\n"
    "Subject: " + subject + "\r\n"
    "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain; charset=UTF-8\r\n"
    "Content-Transfer-Encoding: 7bit\r\n"
    "\r\n" +
    body + "\r\n";


    curl_easy_setopt(curl, CURLOPT_URL, smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);  // ✅ Requerir STARTTLS
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

    struct curl_slist *recipients = nullptr;
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // ✅ Autenticación con Gmail
    curl_easy_setopt(curl, CURLOPT_USERNAME, "notificationservice8080@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "qeng cmas tneh vpvk");

    // 💡 Habilitar STARTTLS correctamente
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);  
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    // 📌 Enviar el contenido del correo
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.size());

    // 🔍 Habilitar logs detallados
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(curl, CURLOPT_STDERR, stderr);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);


    // 🚀 Enviar correo
    CURLcode res = curl_easy_perform(curl);

    // ✅ Limpieza
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "❌ Error en curl_easy_perform(): " << curl_easy_strerror(res) << std::endl;
        return false;
    } else {
        std::cout << "✅ Correo enviado exitosamente" << std::endl;
        return true;
    }
}

// 📌 Controlador de Drogon
class EmailController : public drogon::HttpController<EmailController>
{
public:
    PATH_LIST_BEGIN
        ADD_METHOD_TO(EmailController::sendEmailNotification, "/send", drogon::Post);
    PATH_LIST_END

    void sendEmailNotification(const drogon::HttpRequestPtr &req,
        std::function<void(const std::shared_ptr<drogon::HttpResponse>&)> &&callback)
    {
        std::cout << "📨 Se recibió una solicitud en /send" << std::endl;
        auto json = req->getJsonObject();
        if (!json) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse("❌ JSON inválido");
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }

        try {
            std::string to = (*json)["to"].asString();
            std::string subject = (*json)["subject"].asString();
            std::string body = (*json)["body"].asString();
            
            std::string from = "notificationservice8080@gmail.com";

            std::cout << "📧 Enviando correo a " << to << "..." << std::endl;

            bool success = sendEmail(from, to, subject, body);

            auto resp = success ? 
                drogon::HttpResponse::newHttpJsonResponse("✅ Correo enviado correctamente") :
                drogon::HttpResponse::newHttpJsonResponse("❌ Error al enviar el correo");

            resp->setStatusCode(success ? drogon::k200OK : drogon::k500InternalServerError);
            callback(resp);
        }
        catch (const std::exception &e) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse("❌ Excepción: " + std::string(e.what()));
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    }
};

int main()
{
    drogon::app().addListener("0.0.0.0", 8080);
    std::cout << "🚀 Microservicio de notificaciones corriendo en el puerto 8080..." << std::endl;
    drogon::app().run();
    return 0;
}
