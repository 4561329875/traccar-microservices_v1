#include "DatabaseConnection.h"
#include <stdexcept>
#include <fstream>
#include <json/json.h>

DatabaseConnection::DatabaseConnection() {
    try {
        // Leer configuración desde config.json
        std::ifstream configFile("config.json");
        Json::Value config;
        configFile >> config;

        // Construir cadena de conexión
        std::string connStr = "dbname=" + config["dbname"].asString() +
                              " user=" + config["user"].asString() +
                              " password=" + config["password"].asString() +
                              " host=" + config["host"].asString() +
                              " port=" + config["port"].asString();

        // Conectar a la base de datos
        connection = std::make_shared<pqxx::connection>(connStr);

        if (!connection->is_open()) {
            throw std::runtime_error("No se pudo abrir la conexión a la base de datos");
        }
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error al conectar con la base de datos: ") + e.what());
    }
}

std::shared_ptr<pqxx::connection> DatabaseConnection::getConnection() {
    return connection;
}
