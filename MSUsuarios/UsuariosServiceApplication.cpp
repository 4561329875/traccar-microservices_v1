#include "crow.h"

int main() {
    // Crea una aplicación Crow
    crow::SimpleApp app;

    // Define una ruta principal para verificar que el servidor esté funcionando
    CROW_ROUTE(app, "/")([]() {
        return "User Service Application is running!";
    });

    // Inicia el servidor en el puerto 8080
    app.port(8080).multithreaded().run();

    return 0;
}