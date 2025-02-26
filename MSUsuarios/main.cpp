#include "crow.h"
#include <pqxx/pqxx>
#include <string>
#include <iostream>
extern "C" {
    #include "jwt_funciones.h"
}

// Cadena de conexión ajustada
const std::string conn_str = "host=servBD port=5432 dbname=db user=user password=example";

// Función para verificar conexión a la base de datos y existencia de la tabla
bool check_database_connection() {
    try {
        pqxx::connection C(conn_str);
        if (!C.is_open()) {
            std::cerr << "Error: No se pudo conectar a la base de datos." << std::endl;
            return false;
        }
        pqxx::work W(C);
        pqxx::result R = W.exec("SELECT to_regclass('public.tc_users')");  // Tabla corregida
        if (R[0][0].is_null()) {
            std::cerr << "Error: La tabla 'tc_users' no existe en la base de datos." << std::endl;
            return false;
        }
        std::cout << "Conexión a la base de datos exitosa y la tabla 'tc_users' está disponible." << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error de conexión: " << e.what() << std::endl;
        return false;
    }
}

// Función para obtener usuarios desde la base de datos
std::string get_users_from_db() {
    try {
        pqxx::connection C(conn_str);
        pqxx::work W(C);
        pqxx::result R = W.exec("SELECT id, username, email FROM users");

        std::string result = "Users:\n";
        for (auto row : R) {
            result += "ID: " + std::string(row["id"].c_str()) + ", ";
            result += "Username: " + std::string(row["username"].c_str()) + ", ";
            result += "Email: " + std::string(row["email"].c_str()) + "\n";
        }
        return result;
    } catch (const std::exception &e) {
        return std::string("Error: ") + e.what();
    }
}

// Función para crear un nuevo usuario en la base de datos
std::string create_user(const crow::json::rvalue& json) {
    try {
        if (!json.has("username") || !json.has("email") || !json.has("password")) {
            return "Error: username, email y password son requeridos";
        }
        std::string username = json["username"].s();
        std::string email = json["email"].s();
        std::string password = json["password"].s();

        pqxx::connection C(conn_str);
        pqxx::work W(C);
        W.exec("INSERT INTO users (username, password, email) VALUES (" + 
            W.quote(username) + ", " + W.quote(password) + ", " + W.quote(email) + ")");
        W.commit();

        return "Usuario creado exitosamente: " + username;
    } catch (const std::exception &e) {
        return std::string("Error: ") + e.what();
    }
}

// Función para autenticar usuario y generar JWT
std::string authenticate_user(const crow::json::rvalue& json) {
    try {
        if (!json.has("username") || !json.has("password")) {
            return "Error: username y password son requeridos";
        }
        std::string username = json["username"].s();
        std::string password = json["password"].s();

        pqxx::connection C(conn_str);
        pqxx::work W(C);
        pqxx::result R = W.exec("SELECT id FROM users WHERE username=" + W.quote(username) + " AND password=" + W.quote(password));

        if (R.empty()) {
            return "Error: Usuario o contraseña incorrectos";
        }

        std::string payload = "{\"user\":\"" + username + "\"}";
        const char* secret = "mySecretKey";
        char* token = generate_jwt(payload.c_str(), secret);

        std::string token_str = token ? token : "Error generando JWT";
        free(token);
        return token_str;
    } catch (const std::exception &e) {
        return std::string("Error: ") + e.what();
    }
}

// Middleware para verificar JWT
bool verify_request_jwt(const std::string& token) {
    const char* secret = "mySecretKey";
    return verify_jwt(token.c_str(), secret);
}

int main() {
    crow::SimpleApp app;
    if (!check_database_connection()) {
        return 1;
    }

    CROW_ROUTE(app, "/users/list").methods(crow::HTTPMethod::GET)([]() {
        return get_users_from_db();
    });

    CROW_ROUTE(app, "/users/create").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json)
            return crow::response(400, "Invalid JSON");
        return crow::response{create_user(json)};
    });

    CROW_ROUTE(app, "/users/login").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json)
            return crow::response(400, "Invalid JSON");
        return crow::response{authenticate_user(json)};
    });

    CROW_ROUTE(app, "/secure").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        std::string token = req.get_header_value("Authorization");
        if (token.empty() || !verify_request_jwt(token)) {
            return crow::response(401, "Acceso denegado");
        }
        return crow::response("Acceso autorizado");
    });

    CROW_ROUTE(app, "/")([]() {
        return "User Service Application is running!";
    });

    app.port(8080).multithreaded().run();
    return 0;
}

