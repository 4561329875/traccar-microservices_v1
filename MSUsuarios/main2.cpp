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

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <cstdint>

extern "C" {
    #include "lib/jwt_funciones.h"
}

const std::string conn_str = "host=servBD port=5432 dbname=db user=user password=example";
std::mutex mtx;  // Mutex para sincronizar el acceso a recursos compartidos

// Declarar una instancia global de Crow (se renombra para evitar conflicto)
//crow::SimpleApp appInstance

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
            user_info += "Name: " + std::string(row["name"].c_str()) + ", ";
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
        
        // Liberar memoria si es necesario
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

// Función para encriptar y guardar el token en un archivo externo
bool encrypt_and_save_token(const std::string &token, const std::string &username,
                            const std::string &key, const std::string &filePath) {
    // Verificar que la clave tenga 32 bytes para AES-256
    if (key.size() != 32) {
        std::cerr << "La clave de encriptación debe tener 32 bytes para AES-256." << std::endl;
        return false;
    }

    // Generar un vector de inicialización (IV) de 16 bytes
    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        std::cerr << "Error al generar IV." << std::endl;
        return false;
    }
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error al crear el contexto de encriptación." << std::endl;
        return false;
    }
    
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.data()), iv) != 1) {
       std::cerr << "Error en EVP_EncryptInit_ex." << std::endl;
       EVP_CIPHER_CTX_free(ctx);
       return false;
    }
    
    // Reservar buffer para el texto cifrado
    std::vector<unsigned char> ciphertext(token.size() + AES_BLOCK_SIZE);
    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(token.data()), token.size()) != 1) {
       std::cerr << "Error en EVP_EncryptUpdate." << std::endl;
       EVP_CIPHER_CTX_free(ctx);
       return false;
    }
    int ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
       std::cerr << "Error en EVP_EncryptFinal_ex." << std::endl;
       EVP_CIPHER_CTX_free(ctx);
       return false;
    }
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    
    // Abrir el archivo en modo binario (append) para guardar el token encriptado.
    std::ofstream out(filePath, std::ios::binary | std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Error al abrir el archivo para escribir." << std::endl;
        return false;
    }
    
    // Escribir en el archivo:
    // 1. Longitud del username (uint32_t) y el username.
    uint32_t uname_len = static_cast<uint32_t>(username.size());
    out.write(reinterpret_cast<const char*>(&uname_len), sizeof(uname_len));
    out.write(username.data(), username.size());
    
    // 2. IV (16 bytes).
    out.write(reinterpret_cast<const char*>(iv), AES_BLOCK_SIZE);
    
    // 3. Longitud del texto cifrado (uint32_t) y el texto cifrado.
    uint32_t ct_len = static_cast<uint32_t>(ciphertext_len);
    out.write(reinterpret_cast<const char*>(&ct_len), sizeof(ct_len));
    out.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext_len);
    
    out.close();
    return true;
}

// Función para desencriptar el token utilizando AES-256-CBC
std::string decrypt_token(const std::vector<unsigned char>& ciphertext, int ciphertext_len, const std::string &key, const unsigned char iv[AES_BLOCK_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error al crear el contexto de decriptación." << std::endl;
        return "";
    }
    std::vector<unsigned char> plaintext(ciphertext_len + AES_BLOCK_SIZE);
    int len;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.data()), iv) != 1) {
        std::cerr << "Error en EVP_DecryptInit_ex (decrypt)." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext_len) != 1) {
        std::cerr << "Error en EVP_DecryptUpdate (decrypt)." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    int plaintext_len = len;
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        std::cerr << "Error en EVP_DecryptFinal_ex (decrypt)." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
}

// Función para buscar y obtener el token encriptado desde el archivo de tokens
std::string get_token_from_file(const std::string &username, const std::string &key, const std::string &filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) {
         std::cerr << "No se pudo abrir el archivo de tokens: " << filePath << std::endl;
         return "";
    }
    std::string found_token = "";
    while (in.peek() != EOF) {
         uint32_t uname_len;
         in.read(reinterpret_cast<char*>(&uname_len), sizeof(uname_len));
         if (in.fail()) break;
         std::string uname;
         uname.resize(uname_len);
         in.read(&uname[0], uname_len);
         if (in.fail()) break;
         unsigned char iv[AES_BLOCK_SIZE];
         in.read(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE);
         if (in.fail()) break;
         uint32_t ct_len;
         in.read(reinterpret_cast<char*>(&ct_len), sizeof(ct_len));
         if (in.fail()) break;
         std::vector<unsigned char> ciphertext(ct_len);
         in.read(reinterpret_cast<char*>(ciphertext.data()), ct_len);
         if (in.fail()) break;
         // Si el nombre de usuario coincide, desencriptar y guardar el token
         if (uname == username) {
             found_token = decrypt_token(ciphertext, ct_len, key, iv);
             // Se puede seguir iterando para obtener el último token si existiesen varios registros.
         }
    }
    in.close();
    return found_token;
}

// Función para crear un usuario utilizando el JWT generado y guardar el token en un archivo encriptado
std::string create_user(const crow::json::rvalue& json) {
    systemMetrics metrics("create_user");
    metrics.resetCounters();
    
    std::string result;
    try {
        if (!json.has("name") || !json.has("email") || !json.has("hashedpassword")) {
            result = "❌ Error: name, email y hashedpassword son requeridos";
        } else {
            std::string name = json["name"].s();
            std::string email = json["email"].s();
            std::string password = json["hashedpassword"].s();
            
            // Generar un token JWT válido para el usuario usando un hilo separado
            std::string token = create_jwt_token(name);

            std::cout << "🔹 Creando usuario: " << name << ", " << email << std::endl;
            
            // Insertar usuario en la BD sin el token
            pqxx::connection C(conn_str);
            pqxx::work W(C);
            W.exec("INSERT INTO tc_users (name, hashedpassword, email) VALUES (" + 
                W.quote(name) + ", " + W.quote(password) + ", " + W.quote(email) + ")");
            W.commit();
            
            // Guardar el token en un archivo externo y encriptado.
            const std::string encryption_key = "01234567890123456789012345678901"; 
            const std::string tokenFile = "tokens/tokens.dat";
            
            if (encrypt_and_save_token(token, name, encryption_key, tokenFile)) {
                std::cout << "✅ Token generado y guardado en archivo encriptado" << std::endl;
                result = "{\"message\":\"Usuario creado exitosamente\",\"token\":\"" + token + "\"}";
            } else {
                result = "❌ Error al guardar el token en el archivo.";
            }
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
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        result.resize(size);
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
    
    std::promise<bool> validation_promise;
    std::future<bool> validation_future = validation_promise.get_future();
    
    std::thread jwt_validation_thread([&validation_promise, &token]() {
        int isValid = verify_jwt(token.c_str(), "tu_secreto_aqui");
        validation_promise.set_value(isValid == 1);
    });
    
    jwt_validation_thread.join();
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
    
    std::thread auth_thread([&token, &response]() {
        bool isValid = validate_jwt_token(token);
        std::lock_guard<std::mutex> lock(mtx);
        if (isValid) {
            response = crow::response{200, "Autenticación exitosa"};
        } else {
            response = crow::response{401, "Token JWT inválido"};
        }
    });
    
    auth_thread.join();
    metrics.calculate();
    metrics.printMetrics();
}

// Función para procesar autenticación con permisos en paralelo
void process_authentication_with_permissions(const std::string& token, const std::string& permission, crow::response& response) {
    systemMetrics metrics("process_authentication_with_permissions");
    metrics.resetCounters();
    
    std::promise<bool> token_validation_promise;
    std::future<bool> token_validation_future = token_validation_promise.get_future();
    
    std::thread token_validation_thread([&token_validation_promise, &token]() {
        bool isValid = validate_jwt_token(token);
        token_validation_promise.set_value(isValid);
    });
    
    std::promise<bool> permission_promise;
    std::future<bool> permission_future = permission_promise.get_future();
    
    std::thread permission_thread([&permission_promise, &permission]() {
        bool hasPermission = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        permission_promise.set_value(hasPermission);
    });
    
    token_validation_thread.join();
    permission_thread.join();
    
    bool isValid = token_validation_future.get();
    bool hasPermission = permission_future.get();
    
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
    
    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    std::cout << "Usando " << num_threads << " hilos para OpenMP." << std::endl;
    
    crow::SimpleApp appInstance;
    
    if (!check_database_connection()) {
        return 1;
    }
    
    
     // Ruta para listar usuarios
    CROW_ROUTE(appInstance, "/users/list").methods(crow::HTTPMethod::GET)([]() {
        return get_users_from_db();
    });
    
    CROW_ROUTE(appInstance, "/users/create").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
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
CROW_ROUTE(appInstance, "/users/login").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
    systemMetrics metrics("endpoint_user_login");
    metrics.resetCounters();

    auto json = crow::json::load(req.body);
    crow::response response;

    if (!json || !json.has("email") || !json.has("hashedpassword")) {
        response = crow::response(400, "Invalid JSON or missing email/password");
    } else {
        std::string email = json["email"].s();
        std::string password = json["hashedpassword"].s();

        try {
            pqxx::connection C(conn_str);
            pqxx::work W(C);
            // Buscar el usuario por email, obteniendo el nombre y hashedpassword
            pqxx::result R = W.exec("SELECT name, hashedpassword FROM tc_users WHERE email = " + W.quote(email));

            if (R.empty()) {
                response = crow::response(401, "Usuario no encontrado");
            } else {
                std::string name = std::string(R[0]["name"].c_str());
                std::string db_password = std::string(R[0]["hashedpassword"].c_str());
                if (db_password != password) {
                    response = crow::response(401, "Contraseña incorrecta");
                } else {
                    // Autenticación exitosa: obtener el token desde el archivo de tokens.
                    const std::string encryption_key = "01234567890123456789012345678901";
                    const std::string tokenFile = "./tokens/tokens.dat";
                    std::string token = get_token_from_file(name, encryption_key, tokenFile);
                    if (token.empty()) {
                        response = crow::response(500, "No se encontró token para el usuario");
                    } else {
                        response = crow::response(200, token);
                    }
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
CROW_ROUTE(appInstance, "/auth/jwt").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
    systemMetrics metrics("endpoint_jwt_auth");
    metrics.resetCounters();
    
    auto json = crow::json::load(req.body);
    crow::response response;
    
    if (!json || !json.has("token")) {
        response = crow::response(400, "Invalid JSON or missing token");
    } else {
        std::string token = json["token"].s();
        
        std::thread auth_thread(handle_authentication, token, std::ref(response));
        auth_thread.join();
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return response;
});

    // Ruta para validar permisos en múltiples hilos
CROW_ROUTE(appInstance, "/auth/permissions").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
    systemMetrics metrics("endpoint_permissions");
    metrics.resetCounters();
    
    auto json = crow::json::load(req.body);
    crow::response response;
    
    if (!json || !json.has("token") || !json.has("permission")) {
        response = crow::response(400, "Invalid JSON or missing token/permission");
    } else {
        std::string token = json["token"].s();
        std::string permission = json["permission"].s();
        
        std::thread auth_thread(process_authentication_with_permissions, token, permission, std::ref(response));
        auth_thread.join();
    }
    
    metrics.calculate();
    metrics.printMetrics();
    
    return response;
});
    
    // Ruta para validar múltiples tokens en paralelo
CROW_ROUTE(appInstance, "/auth/validate-bulk").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
    systemMetrics metrics("endpoint_validate_bulk");
    metrics.resetCounters();
    
    auto json = crow::json::load(req.body);
    
    if (!json || !json.has("tokens")) {
        return crow::response(400, "Invalid JSON or missing tokens");
    }
    
    std::vector<std::string> tokens;
    try {
        for (size_t i = 0; ; i++) {
            tokens.push_back(json["tokens"][i].s());
        }
    } catch (const std::exception& e) {
        if (tokens.empty()) {
            return crow::response(400, "tokens debe ser un array no vacío");
        }
    }
    
    std::vector<bool> validation_results = validate_multiple_tokens(tokens);
    crow::json::wvalue result_json;
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
CROW_ROUTE(appInstance, "/").methods(crow::HTTPMethod::GET)([]() {
    systemMetrics metrics("endpoint_root");
    metrics.resetCounters();
    auto response = crow::response{read_file("public/createUser.html")};
    metrics.calculate();
    metrics.printMetrics();
    return response;
});

// Ruta para servir login.html
CROW_ROUTE(appInstance, "/login").methods(crow::HTTPMethod::GET)([]() {
    systemMetrics metrics("endpoint_login_page");
    metrics.resetCounters();
    auto response = crow::response{read_file("public/login.html")};
    metrics.calculate();
    metrics.printMetrics();
    return response;
});

// Ruta genérica para archivos estáticos en "public/"
CROW_ROUTE(appInstance, "/<path>")([](const std::string& path){
    systemMetrics metrics("endpoint_static_" + path);
    metrics.resetCounters();
    std::string full_path = "public/" + path;
    auto response = crow::response{read_file(full_path)};
    metrics.calculate();
    metrics.printMetrics();
    return response;
});


    
    appInstance.port(8080).multithreaded().run();
    
    generalMetrics.calculate();
    generalMetrics.printMetrics();
    
    return 0;
}

