#include "crow.h"

int main() {
    crow::SimpleApp app; // Crea la aplicación

    // Define una ruta para manejar solicitudes GET en "/"
    CROW_ROUTE(app, "/")([]() {
        return "Hola Mundo";
    });

    // Inicia el servidor en el puerto 8080
    app.port(8080).multithreaded().run();
}
