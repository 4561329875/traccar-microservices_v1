#include "crow.h"
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <ctime>
#include <vector>
#include <future>
#include <omp.h>
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
                std::cerr << "Error: La tabla 'users' no existe en la base de datos." << std::endl;
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

// Función para obtener usuarios de la base de datos utilizando OpenMP para procesar los resultados
std::string get_users_from_db() {
    systemMetrics metrics("get_users_from_db");
    metrics.resetCounters();
    
    std::string result;
    try {
        pqxx::connection C(conn_str);
        pqxx::work W(C);
        pqxx::result R = W.exec("SELECT id, name, email FROM tc_users");

        result = "Users:\n";
        
        // Usar un vector para almacenar los resultados parciales
        std::vector<std::string> partial_results(R.size());
        
        // Usar OpenMP para procesar los resultados en paralelo
        #pragma omp parallel for
        for (size_t i = 0; i < R.size(); i++) {
            auto row = R[i];
            std::string user_info = "ID: " + std::string(row["id"].c_str()) + ", ";
            user_info += "Name: " + std::string(row["Name"].c_str()) + ", ";
            user_info += "Email: " + std::string(row["email"].c_str()) + "\n";
            partial_results[i] = user_info;
        }
        
        // Unir los resultados parciales
        for (const auto& user_info : partial_results) {
            result += user_info;
        }
    } catch (const std::exception &e) {
        result = std::string("Error: ") + e.what();
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para generar un token JWT válido utilizando un hilo separado
std::string create_jwt_token(const std::string &username) {
    systemMetrics metrics("create_jwt_token");
    metrics.resetCounters();
    
    // Crear un std::promise para recibir el resultado del hilo
    std::promise<std::string> token_promise;
    std::future<std::string> token_future = token_promise.get_future();
    
    // Crear un hilo para generar el token JWT
    std::thread jwt_thread([&token_promise, &username]() {
        // Obtener la hora actual y calcular la expiración (24 horas después)
        std::time_t now = std::time(nullptr);
        std::time_t exp = now + 86400;  // 86400 segundos = 24 horas

        // Crear el payload en formato JSON
        std::string payload = "{\"sub\":\"" + username + "\",\"iat\":" + std::to_string(now) +
                             ",\"exp\":" + std::to_string(exp) + "}";

        // Generar el JWT usando generate_jwt
        char* token = generate_jwt(payload.c_str(), "tu_secreto_aqui");
        std::string jwtToken(token);
        
        // Establecer el valor en la promesa
        token_promise.set_value(jwtToken);
        
        // Si generate_jwt asigna memoria para token, libera la memoria si es necesario
        // free(token);
    });
    
    // Esperar a que el hilo termine
    jwt_thread.join();
    
    // Obtener el token JWT del futuro
    std::string token = token_future.get();
    
    metrics.calculate();
    metrics.printMetrics();
    
    return token;
}

// Función para crear un usuario utilizando el JWT generado
std::string create_user(const crow::json::rvalue& json) {
    systemMetrics metrics("create_user");
    metrics.resetCounters();
    
    std::string result;
    try {
        if (!json.has("name") || !json.has("email") || !json.has("hashedassword")) {
            result = "❌ Error: name, email y hashedpassword son requeridos";
        } else {
            std::string username = json["name"].s();
            std::string email = json["email"].s();
            std::string password = json["hashedpassword"].s();
            
            // Generar un token JWT válido para el usuario usando un hilo separado
            std::string token = create_jwt_token(username);

            std::cout << "🔹 Insertando usuario: " << username << ", " << email << std::endl;

            pqxx::connection C(conn_str);
            pqxx::work W(C);
            W.exec("INSERT INTO tc_users (name, hashedpassword, email, token) VALUES (" + 
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

// Función para leer archivos desde el directorio "public/" con OpenMP
std::string read_file(const std::string& filename) {
    systemMetrics metrics("read_file(" + filename + ")");
    metrics.resetCounters();
    
    std::string result;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        result = "File not found: " + filename;
    } else {
        // Obtener el tamaño del archivo
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Reservar espacio para el contenido del archivo
        result.resize(size);
        
        // Leer el archivo por bloques en paralelo si es suficientemente grande
        if (size > 4096) {
            const size_t chunk_size = 4096;
            const size_t num_chunks = (size + chunk_size - 1) / chunk_size;
            
            #pragma omp parallel for
            for (size_t i = 0; i < num_chunks; i++) {
                size_t offset = i * chunk_size;
                size_t length = std::min(chunk_size, size - offset);
                
                std::ifstream chunk_file(filename, std::ios::in | std::ios::binary);
                chunk_file.seekg(offset);
                chunk_file.read(&result[offset], length);
            }
        } else {
            // Para archivos pequeños, leer directamente
            file.read(&result[0], size);
        }
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return result;
}

// Función para validar el token JWT utilizando un hilo separado
bool validate_jwt_token(const std::string& token) {
    systemMetrics metrics("validate_jwt_token");
    metrics.resetCounters();
    
    // Crear un std::promise para recibir el resultado del hilo
    std::promise<bool> validation_promise;
    std::future<bool> validation_future = validation_promise.get_future();
    
    // Crear un hilo para validar el token JWT
    std::thread jwt_validation_thread([&validation_promise, &token]() {
        int isValid = verify_jwt(token.c_str(), "tu_secreto_aqui");
        validation_promise.set_value(isValid == 1);  // verify_jwt devuelve 1 si el token es válido
    });
    
    // Esperar a que el hilo termine
    jwt_validation_thread.join();
    
    // Obtener el resultado de la validación del futuro
    bool isValid = validation_future.get();
    
    metrics.calculate();
    metrics.printMetrics();
    
    return isValid;
}

// Función para validar múltiples tokens JWT en paralelo
std::vector<bool> validate_multiple_tokens(const std::vector<std::string>& tokens) {
    systemMetrics metrics("validate_multiple_tokens");
    metrics.resetCounters();
    
    std::vector<bool> results(tokens.size());
    
    #pragma omp parallel for
    for (size_t i = 0; i < tokens.size(); i++) {
        results[i] = validate_jwt_token(tokens[i]);
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return results;
}

// Función para manejar la autenticación concurrente con múltiples hilos
void handle_authentication(const std::string& token, crow::response& response) {
    systemMetrics metrics("handle_authentication");
    metrics.resetCounters();
    
    // Usar un hilo para validar el token
    std::thread auth_thread([&token, &response]() {
        bool isValid = validate_jwt_token(token);
        
        std::lock_guard<std::mutex> lock(mtx);  // Bloquear el acceso a recursos compartidos
        if (isValid) {
            response = crow::response{200, "Autenticación exitosa"};
        } else {
            response = crow::response{401, "Token JWT inválido"};
        }
    });
    
    // Esperar a que el hilo termine
    auth_thread.join();
    
    metrics.calculate();
    metrics.printMetrics();
}

// Nueva función para procesar autenticación con permisos en paralelo
void process_authentication_with_permissions(const std::string& token, const std::string& permission, crow::response& response) {
    systemMetrics metrics("process_authentication_with_permissions");
    metrics.resetCounters();
    
    // Crear hilos para validar el token y verificar permisos en paralelo
    std::promise<bool> token_validation_promise;
    std::future<bool> token_validation_future = token_validation_promise.get_future();
    
    std::thread token_validation_thread([&token_validation_promise, &token]() {
        bool isValid = validate_jwt_token(token);
        token_validation_promise.set_value(isValid);
    });
    
    std::promise<bool> permission_promise;
    std::future<bool> permission_future = permission_promise.get_future();
    
    std::thread permission_thread([&permission_promise, &permission]() {
        // Simulación de verificación de permisos
        // En un caso real, podría consultar una base de datos o un servicio
        bool hasPermission = true;
        
        // Simulamos algún procesamiento
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        permission_promise.set_value(hasPermission);
    });
    
    // Esperar a que los hilos terminen
    token_validation_thread.join();
    permission_thread.join();
    
    // Obtener los resultados
    bool isValid = token_validation_future.get();
    bool hasPermission = permission_future.get();
    
    // Preparar la respuesta
    std::lock_guard<std::mutex> lock(mtx);
    if (!isValid) {
        response = crow::response{401, "Token JWT inválido"};
    } else if (!hasPermission) {
        response = crow::response{403, "Permiso denegado"};
    } else {
        response = crow::response{200, "Permiso concedido"};
    }
    
    metrics.calculate();
    metrics.printMetrics();
}

int main() {
    systemMetrics generalMetrics("Programa_Completo");
    generalMetrics.resetCounters();
    
    // Configurar el número de hilos para OpenMP basado en el hardware disponible
    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    std::cout << "Usando " << num_threads << " hilos para OpenMP." << std::endl;
    
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

    // Endpoint para login de usuario utilizando email y password
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
                    std::string db_password = std::string(R[0]["hashedpassword"].c_str());
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

    // Ruta para autenticación con JWT utilizando hilos
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
            
            // Crear un hilo para procesar la autenticación y permisos en paralelo
            std::thread auth_thread(process_authentication_with_permissions, token, permission, std::ref(response));
            auth_thread.join();  // Esperar a que el hilo termine
        }
        
        metrics.calculate();
        metrics.printMetrics();
        
        return response;
    });

    // Nueva ruta para validar múltiples tokens en paralelo - Corregida para tu versión de Crow
    CROW_ROUTE(app, "/auth/validate-bulk").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        systemMetrics metrics("endpoint_validate_bulk");
        metrics.resetCounters();
        
        auto json = crow::json::load(req.body);
        
        if (!json || !json.has("tokens")) {
            return crow::response(400, "Invalid JSON or missing tokens");
        }
        
        // Extraer tokens desde el JSON
        std::vector<std::string> tokens;
        try {
            // Intentar acceder a "tokens" como array
            for (size_t i = 0; ; i++) {
                tokens.push_back(json["tokens"][i].s());
            }
        } catch (const std::exception& e) {
            // Si hay una excepción, es posible que no sea un array o que hayamos llegado al final
            if (tokens.empty()) {
                return crow::response(400, "tokens debe ser un array no vacío");
            }
        }
        
        // Validar múltiples tokens en paralelo
        std::vector<bool> validation_results = validate_multiple_tokens(tokens);
        
        // Crear respuesta en formato JSON usando la API de Crow
        crow::json::wvalue result_json;
        
        // Construir un array de resultados compatible con tu versión de Crow
        std::vector<crow::json::wvalue> results_array;
        
        for (size_t i = 0; i < tokens.size(); i++) {
            crow::json::wvalue token_result;
            token_result["token"] = tokens[i];
            token_result["valid"] = validation_results[i];
            results_array.push_back(std::move(token_result));
        }
        
        result_json["results"] = std::move(results_array);
        
        metrics.calculate();
        metrics.printMetrics();
        
        return crow::response(200, result_json);
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
