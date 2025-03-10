#include "crow.h"
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <ctime>     // Para std::time
#include "lib/benchmark2.h"

extern "C" {
    #include "lib/jwt_funciones.h"
}

const std::string conn_str = "host=servBD port=5432 dbname=db user=user password=example";
std::mutex mtx;  // Mutex para sincronizar el acceso a recursos compartidos

// Función para verificar la conexión a la base de datos
bool check_database_connection() {
    systemMetrics metrics("check_database_connection");
    metrics.resetCounters();
    
    bool result = false;
    try {
        pqxx::connection C(conn_str);
        if (!C.is_open()) {
            std::cerr << "Error: No se pudo conectar a la base de datos." << std::endl;
            result = false;
        } else {
            pqxx::work W(C);
            pqxx::result R = W.exec("SELECT to_regclass('public.tc_users')");
            if (R[0][0].is_null()) {
                std::cerr << "Error: La tabla 'tc_users' no existe en la base de datos." << std::endl;
                result = false;
            } else {
                std::cout << "Conexión a la base de datos exitosa y la tabla 'tc_users' está disponible." << std::endl;
                result = true;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error de conexión: " << e.what() << std::endl;
        result = false;
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para obtener usuarios de la base de datos
std::string get_users_from_db() {
    systemMetrics metrics("get_users_from_db");
    metrics.resetCounters();
    
    std::string result;
    try {
        pqxx::connection C(conn_str);
        pqxx::work W(C);
        pqxx::result R = W.exec("SELECT id, username, email FROM tc_users");

        result = "Users:\n";
        for (auto row : R) {
            result += "ID: " + std::string(row["id"].c_str()) + ", ";
            result += "Username: " + std::string(row["username"].c_str()) + ", ";
            result += "Email: " + std::string(row["email"].c_str()) + "\n";
        }
    } catch (const std::exception &e) {
        result = std::string("Error: ") + e.what();
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para generar un token JWT válido utilizando generate_jwt (implementado en jwt_funciones.c)
std::string create_jwt_token(const std::string &username) {
    // Obtener la hora actual y calcular la expiración (24 horas después)
    std::time_t now = std::time(nullptr);
    std::time_t exp = now + 86400;  // 86400 segundos = 24 horas

    // Crear el payload en formato JSON. Ajusta el formato según lo que necesites.
    std::string payload = "{\"sub\":\"" + username + "\",\"iat\":" + std::to_string(now) +
                          ",\"exp\":" + std::to_string(exp) + "}";

    // Generar el JWT usando generate_jwt. Se utiliza el mismo secreto para la validación.
    char* token = generate_jwt(payload.c_str(), "tu_secreto_aqui");
    std::string jwtToken(token);

    // Si generate_jwt asigna memoria para token, libera la memoria si es necesario.
    // free(token);

    return jwtToken;
}

// Función para crear un usuario utilizando el JWT generado
std::string create_user(const crow::json::rvalue& json) {
    systemMetrics metrics("create_user");
    metrics.resetCounters();
    
    std::string result;
    try {
        if (!json.has("username") || !json.has("email") || !json.has("password")) {
            result = "❌ Error: username, email y password son requeridos";
        } else {
            std::string username = json["username"].s();
            std::string email = json["email"].s();
            std::string password = json["password"].s();
            // Generar un token JWT válido para el usuario
            std::string token = create_jwt_token(username);

            std::cout << "🔹 Insertando usuario: " << username << ", " << email << std::endl;

            pqxx::connection C(conn_str);
            pqxx::work W(C);
            W.exec("INSERT INTO tc_users (username, hashedpassword, email, token) VALUES (" + 
                W.quote(username) + ", " + W.quote(password) + ", " + W.quote(email) + ", " + W.quote(token) + ")");
            W.commit();

            std::cout << "✅ Usuario creado exitosamente" << std::endl;
            result = "Usuario creado exitosamente. Token: " + token;
        }
    } catch (const std::exception &e) {
        result = std::string("❌ Error al insertar en la BD: ") + e.what();
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para leer archivos desde el directorio "public/"
std::string read_file(const std::string& filename) {
    systemMetrics metrics("read_file(" + filename + ")");
    metrics.resetCounters();
    
    std::string result;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        result = "File not found: " + filename;
    } else {
        result = std::string((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para validar el token JWT utilizando verify_jwt
bool validate_jwt_token(const std::string& token) {
    systemMetrics metrics("validate_jwt_token");
    metrics.resetCounters();
    
    int isValid = verify_jwt(token.c_str(), "tu_secreto_aqui");  // Verifica el JWT usando el mismo secreto
        
    metrics.calculate();
    metrics.printMetrics();
    
    return (isValid == 1);  // verify_jwt devuelve 1 si el token es válido
}

// Función para manejar la autenticación concurrente
void handle_authentication(const std::string& token, crow::response& response) {
    systemMetrics metrics("handle_authentication");
    metrics.resetCounters();
    
    bool isValid = validate_jwt_token(token);
    
    std::lock_guard<std::mutex> lock(mtx);  // Bloquear el acceso a recursos compartidos
    if (isValid) {
        response = crow::response{200, "Autenticación exitosa"};
    } else {
        response = crow::response{401, "Token JWT inválido"};
    }
    
    metrics.calculate();
    metrics.printMetrics();
}

int main() {
    systemMetrics generalMetrics("Programa_Completo");
    generalMetrics.resetCounters();
    
    // Declarar la instancia de la aplicación Crow
    crow::SimpleApp app;

    if (!check_database_connection()) {
        return 1;
    }

    // Ruta para listar usuarios
    CROW_ROUTE(app, "/users/list").methods(crow::HTTPMethod::GET)([]() {
        return get_users_from_db();
    });

    // Ruta para crear usuario
    CROW_ROUTE(app, "/users/create").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        systemMetrics metrics("endpoint_create_user");
        metrics.resetCounters();
        
        auto json = crow::json::load(req.body);
        crow::response response;
        
        if (!json) {
            response = crow::response(400, "Invalid JSON");
        } else {
            auto res = create_user(json);
            response = crow::response{res};
        }
        
        metrics.calculate();
        metrics.printMetrics();
        
        return response;
    });

    // Actualizado: Endpoint para login de usuario (/users/login) usando email y password
    CROW_ROUTE(app, "/users/login").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        systemMetrics metrics("endpoint_user_login");
        metrics.resetCounters();

        auto json = crow::json::load(req.body);
        crow::response response;
    
        if (!json || !json.has("email") || !json.has("password")) {
            response = crow::response(400, "Invalid JSON or missing email/password");
        } else {
            std::string email = json["email"].s();
            std::string password = json["password"].s();

            try {
                pqxx::connection C(conn_str);
                pqxx::work W(C);
                // Buscar el usuario por email
                pqxx::result R = W.exec("SELECT token, hashedpassword FROM tc_users WHERE email = " + W.quote(email));
            
                if (R.empty()) {
                    response = crow::response(401, "Usuario no encontrado");
                } else {
                    std::string db_password = std::string(R[0]["password"].c_str());
                    if (db_password != password) {
                        response = crow::response(401, "Contraseña incorrecta");
                    } else {
                        // Autenticación exitosa: retornamos el token almacenado en la BD.
                        std::string token = std::string(R[0]["token"].c_str());
                        response = crow::response(200, token);
                    }
                }
            } catch (const std::exception &e) {
                response = crow::response(500, "Error al conectar con la BD: " + std::string(e.what()));
            }
        }
    
        metrics.calculate();
        metrics.printMetrics();
        return response;
    });

    // Ruta para autenticación con JWT
    CROW_ROUTE(app, "/auth/jwt").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        systemMetrics metrics("endpoint_jwt_auth");
        metrics.resetCounters();
        
        auto json = crow::json::load(req.body);
        crow::response response;
        
        if (!json || !json.has("token")) {
            response = crow::response(400, "Invalid JSON or missing token");
        } else {
            std::string token = json["token"].s();
            
            // Crear un hilo para manejar la autenticación
            std::thread auth_thread(handle_authentication, token, std::ref(response));
            auth_thread.join();  // Esperar a que el hilo termine
        }
        
        metrics.calculate();
        metrics.printMetrics();
        
        return response;
    });

    // Ruta para validar permisos en múltiples hilos
    CROW_ROUTE(app, "/auth/permissions").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        systemMetrics metrics("endpoint_permissions");
        metrics.resetCounters();
        
        auto json = crow::json::load(req.body);
        crow::response response;
        
        if (!json || !json.has("token") || !json.has("permission")) {
            response = crow::response(400, "Invalid JSON or missing token/permission");
        } else {
            std::string token = json["token"].s();
            std::string permission = json["permission"].s();
            
            // Crear un hilo para validar el token y los permisos
            std::thread auth_thread([&token, &permission, &response]() {
                bool isValid = validate_jwt_token(token);
                
                std::lock_guard<std::mutex> lock(mtx);  // Bloquear el acceso a recursos compartidos
                if (isValid) {
                    bool hasPermission = true;  // Simulación de validación de permisos
                    if (hasPermission) {
                        response = crow::response{200, "Permiso concedido"};
                    } else {
                        response = crow::response{403, "Permiso denegado"};
                    }
                } else {
                    response = crow::response{401, "Token JWT inválido"};
                }
            });
            
            auth_thread.join();  // Esperar a que el hilo termine
        }
        
        metrics.calculate();
        metrics.printMetrics();
        
        return response;
    });

    // Ruta por defecto para servir createUser.html
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([]() {
        systemMetrics metrics("endpoint_root");
        metrics.resetCounters();
        auto response = crow::response{read_file("public/createUser.html")};
        metrics.calculate();
        metrics.printMetrics();
        return response;
    });

    // Ruta para servir login.html
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::GET)([]() {
        systemMetrics metrics("endpoint_login_page");
        metrics.resetCounters();
        auto response = crow::response{read_file("public/login.html")};
        metrics.calculate();
        metrics.printMetrics();
        return response;
    });

    // Ruta genérica para archivos estáticos en "public/"
    CROW_ROUTE(app, "/<path>")([](const std::string& path){
        systemMetrics metrics("endpoint_static_" + path);
        metrics.resetCounters();
        std::string full_path = "public/" + path;
        auto response = crow::response{read_file(full_path)};
        metrics.calculate();
        metrics.printMetrics();
        return response;
    });

    // Iniciar la aplicación
    app.port(8080).multithreaded().run();
    
    // Calcular e imprimir métricas finales
    generalMetrics.calculate();
    generalMetrics.printMetrics();
    
    return 0;
}

