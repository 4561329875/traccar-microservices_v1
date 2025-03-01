#include <crow.h>
#include <vector>
#include <string>
#include <algorithm>
#include "conexionPostrgrest.c"

#include "obtenerTodosDispositivos.h"
#include "obtenerDispositivosCondicion.h"
#include "ObteDisConUni.h"
#include "GuardarDispositivo.h"
#include "ActualizarDispositivo.h"
#include "EliminarDispositivo.h"

#include "ObtenerTodosConductores.h"
#include "ObtenerCondiciConductores.h"
#include "ObtenerCondiciUniConductores.h"
#include "GuardarConductor.h"
#include "ActualizarConductor.h"
#include "EliminarConductor.h"

#include "ActualizarAcumuladores.h"

#include "ManejadorDeIMG.h"

#include <libpq-fe.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <map>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

#include <ctime>

#include <fstream>  // Para manejo de archivos
#include "benchmark.h"



#define UPLOAD_FOLDER "/app/media"
using json = nlohmann::json;
PGconn *conn ;





crow::json::wvalue get_devices(const crow::request& req, int path_id = -1) {

    //iniciar medidor
    systemMetrics sm("Prueba de rendimiento");

    sm.resetCounters();
    //


    bool all = req.url_params.get("all") ? std::string(req.url_params.get("all")) == "true" : false;
    int userId = req.url_params.get("userId") ? std::stoi(req.url_params.get("userId")) : -1;


    std::vector<int> ids;
    auto parametros=req.url_params.keys();

    if (req.url_params.get_list("id",false).size() > 0) {
        for (const auto& val : req.url_params.get_list("id",false)) {
            ids.push_back(std::stoi(val));
        }
    }
    if (path_id != -1) {
        ids.push_back(path_id);
    }

    std::vector<std::string> uniqueIds;
    if (req.url_params.get_list("uniqueId",false).size() > 0) {
        for (const auto& val : req.url_params.get_list("uniqueId",false)) {
            uniqueIds.push_back(val);
        }
    }





    std::vector<std::string> condiciones;

    if(userId!=-1){//para user id
        condiciones.push_back("u.userid="+std::to_string(userId));
    }

    if(ids.size()>0){ //para ids
        std::string sec = "(";
        for (size_t i = 0; i < ids.size(); ++i) {
            sec =sec+ "d.id="+std::to_string( ids[i]);
            if (i < ids.size() - 1) {
                sec += " OR ";
            }
        }
        sec += ")";
        condiciones.push_back(sec);
    }

    if(uniqueIds.size()>0){ //para uniqueIds
        std::string sec = "(";
        for (size_t i = 0; i < uniqueIds.size(); ++i) {
            sec += "d.uniqueid='" + uniqueIds[i] +"'" ;
            if (i < uniqueIds.size() - 1) {
                sec += " OR ";
            }
        }
        sec += ")";
        condiciones.push_back(sec);
    }

    if (all == true || condiciones.size()==0) {      //comportamiento por defecto
        char *json_result = consultarDispositivos(conn);
        auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

        free(json_result);  // Libera la memoria después de usarla

        //justo antes de cualquier return gravas los resultas
        sm.calculate(); // Calcula las métricas después de la ejecución

    // Guardar en CSV
    std::ofstream file("metrics.csv", std::ios::app); // Abre en modo 'append' para no sobrescribir

    // Si el archivo está vacío, escribe la cabecera
    if (file.tellp() == 0) {
        file << "Nombre,Tiempo (s),Tiempo (ms),Memoria Dif (KB),Pico Memoria (KB),Uso CPU (%)\n";
    }

    // Escribir las métricas en el archivo CSV
    file << sm.getDurationInSeconds() << ","
         << sm.getDurationInMiliseconds() << ","
         << sm.getDifMemoryKb() << ","
         << sm.getPeakDifMemoryKb() << ","
         << sm.getCpuPercent() << "\n";

    file.close();
        //


        return json_obj;
    }else{                                                         //queri especifico

        std::string resultado = condiciones[0];
        for (size_t i = 1; i < condiciones.size(); ++i) {
            resultado += " AND " + condiciones[i];
        }

        const char *concicionC=resultado.c_str();

        char *json_result = consultarDispositivosCon(conn,concicionC);


        auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

        free(json_result);  // Libera la memoria después de usarla

        //justo antes de cualquier return gravas los resultas
        sm.calculate(); // Calcula las métricas después de la ejecución

    // Guardar en CSV
    std::ofstream file("metrics.csv", std::ios::app); // Abre en modo 'append' para no sobrescribir

    // Si el archivo está vacío, escribe la cabecera
    if (file.tellp() == 0) {
        file << "Nombre,Tiempo (s),Tiempo (ms),Memoria Dif (KB),Pico Memoria (KB),Uso CPU (%)\n";
    }

    // Escribir las métricas en el archivo CSV
    file << sm.getDurationInSeconds() << ","
         << sm.getDurationInMiliseconds() << ","
         << sm.getDifMemoryKb() << ","
         << sm.getPeakDifMemoryKb() << ","
         << sm.getCpuPercent() << "\n";

    file.close();
        //

        return json_obj;

    }
    //justo antes de cualquier return gravas los resultas
        sm.calculate(); // Calcula las métricas después de la ejecución

    // Guardar en CSV
    std::ofstream file("metrics.csv", std::ios::app); // Abre en modo 'append' para no sobrescribir

    // Si el archivo está vacío, escribe la cabecera
    if (file.tellp() == 0) {
        file << "Nombre,Tiempo (s),Tiempo (ms),Memoria Dif (KB),Pico Memoria (KB),Uso CPU (%)\n";
    }

    // Escribir las métricas en el archivo CSV
    file << sm.getDurationInSeconds() << ","
         << sm.getDurationInMiliseconds() << ","
         << sm.getDifMemoryKb() << ","
         << sm.getPeakDifMemoryKb() << ","
         << sm.getCpuPercent() << "\n";

    file.close();
        //

    return crow::json::wvalue("");
}


crow::json::wvalue get_devices_unicos(const crow::request& req, int path_id = -1) {
    int userId = req.url_params.get("userId") ? std::stoi(req.url_params.get("userId")) : -1;


    std::vector<std::string> condiciones;

    if(userId!=-1){//para user id
        condiciones.push_back("userid="+std::to_string(userId));
    }


    std::string sec = "(";
    std::string secInd = std::to_string( path_id);
    sec =sec+ "id="+secInd;


    sec += ")";
    condiciones.push_back(sec);



    std::string resultado = condiciones[0];
    for (size_t i = 1; i < condiciones.size(); ++i) {
        resultado += " AND " + condiciones[i];
    }

    resultado+=" LIMIT 1" ; // para limitar el resultado a un solo elemento
    // Convertir el resultado en un char*
    char* resultChar = new char[resultado.size() + 1];
    std::copy(resultado.begin(), resultado.end(), resultChar);
    resultChar[resultado.size()] = '\0';

    char *json_result = consultarDispositivosConUni(conn,resultChar);
    auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

    free(json_result);  // Libera la memoria después de usarla

    return json_obj;




}




std::string formatDate(const std::string& inputDate) {
    std::tm tm = {};
    std::istringstream ss(inputDate);

    // Usar un formato que ignore la zona horaria y solo analice la fecha y hora
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        std::cerr << "Error al formatear la fecha" << std::endl;
        return "";
    }

    // Convertir a formato "%4d-%2d-%2d %2d:%2d:%2d.%3d"
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    // Agregar milisegundos manualmente desde el string de entrada
    size_t dotPos = inputDate.find('.');
    if (dotPos != std::string::npos) {
        // Extraer milisegundos y agregarlos
        std::string milliseconds = inputDate.substr(dotPos + 1, 3);  // Tomamos 3 dígitos
        os << "." << milliseconds;
    } else {
        os << ".000"; // Si no hay milisegundos, agregar .000
    }

    return os.str();
}



crow::json::wvalue crearDisp(const crow::request& req) {

    auto data = json::parse(req.body);



    std::map<std::string, std::string> parPreMap;
    std::map<std::string, std::string> parametersMap = {
        {"id", "id"},
        {"name", "name"},
        {"uniqueId", "uniqueid"},
        {"status", "status"},
        {"disabled", "disabled"},
        {"lastUpdate", "lastupdate"},
        {"positionId", "positionid"},
        {"groupId", "groupid"},
        {"phone", "phone"},
        {"model", "model"},
        {"contact", "contact"},
        {"category", "category"},
        {"attributes", "attributes"}
    };

    for (const auto& param : parametersMap) {

        if(data.contains(param.first)){
        if (data.at(param.first).is_string()) {
            if (param.first == "lastUpdate") {
                // Si el parámetro es 'lastUpdate', formateamos la fecha.
                std::string dateStr = data.at(param.first).get<std::string>();
                std::string formattedDate = "'"+formatDate(dateStr)+"'";  // Formateamos la fecha.
                parPreMap[param.second] = formattedDate;}
            else{
                parPreMap[param.second] = "'"+data.at(param.first).get<std::string>()+"'";}  // Extraer directamente si es string
        } else if(data.at(param.first).is_object()){ // para los objetos
            parPreMap[param.second] = "'"+data.at(param.first).dump()+"'";
        }
        else{
            if((param.first == "groupId"||param.first == "positionId")&& data.at(param.first).get<int>()==0){}
            else{
            parPreMap[param.second] = data.at(param.first).dump();  // Convertir a string si es otro tipo
            }
        }}
    }

    for (const auto& entry : parPreMap) {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }

    std::stringstream parametros, valores;
    parametros << "(";
    valores << "(";
    // Iterar sobre el mapa y agregar las claves (first) separadas por coma
    bool first = true;
    for (const auto& entry : parPreMap) {
        if (!first) {
            parametros << ",";
            valores << ",";            // Agregar coma si no es el primer elemento
        }
        parametros << entry.first;
        valores  << entry.second;      // Agregar la clave
        first = false;
    }

    // Terminar con el paréntesis derecho
    parametros << ")";
    valores << ")";

    std::string parametrosSrt = parametros.str();
    char* parametoschar = new char[parametrosSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(parametrosSrt.begin(), parametrosSrt.end(), parametoschar);
    parametoschar[parametrosSrt.size()] = '\0';

    std::string valoresSrt = valores.str();
    char* valoreschar = new char[valoresSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(valoresSrt.begin(), valoresSrt.end(), valoreschar);
    valoreschar[valoresSrt.size()] = '\0';



    char *json_result = guardarDisp(conn,parametoschar,valoreschar);
    auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

    free(json_result);  // Libera la memoria después de usarla

    return json_obj;

}





crow::json::wvalue actualizarDisp(const crow::request& req, int deviceid) {

    auto data = json::parse(req.body);



    std::map<std::string, std::string> parPreMap;
    std::map<std::string, std::string> parametersMap = {
        {"id", "id"},
        {"name", "name"},
        {"uniqueId", "uniqueid"},
        {"status", "status"},
        {"disabled", "disabled"},
        {"lastUpdate", "lastupdate"},
        {"positionId", "positionid"},
        {"groupId", "groupid"},
        {"phone", "phone"},
        {"model", "model"},
        {"contact", "contact"},
        {"category", "category"},
        {"attributes", "attributes"}
    };

    for (const auto& param : parametersMap) {


        std::cout<<param.first<<":"<<data.at(param.first).dump()<<"\n";
        if(data.contains(param.first)){
        if (data.at(param.first).is_string()) {
            if (param.first == "lastUpdate") {
                // Si el parámetro es 'lastUpdate', formateamos la fecha.
                std::string dateStr = data.at(param.first).get<std::string>();
                std::string formattedDate = "'"+formatDate(dateStr)+"'";  // Formateamos la fecha.
                parPreMap[param.second] = formattedDate;}
            else{
                parPreMap[param.second] = "'"+data.at(param.first).get<std::string>()+"'";}  // Extraer directamente si es string
        } else if(data.at(param.first).is_object()){ // para los objetos
            parPreMap[param.second] = "'"+data.at(param.first).dump()+"'";
        }
        else{
            if((param.first == "groupId"||param.first == "positionId")&& !data.at(param.first).empty() && data.at(param.first).get<int>()==0){
                parPreMap[param.second] ="NULL";
            }
            else{
            parPreMap[param.second] = data.at(param.first).dump();  // Convertir a string si es otro tipo
            }
        }}


    }

    for (const auto& entry : parPreMap) {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }

    std::stringstream parametrosYvalores;


    // Iterar sobre el mapa y agregar las claves (first) separadas por coma
    bool first = true;
    for (const auto& entry : parPreMap) {
        if (!first) {
            parametrosYvalores  << " , ";          // Agregar coma si no es el primer elemento
        }
        parametrosYvalores << entry.first;
        parametrosYvalores << "=";
        parametrosYvalores  << entry.second;      // Agregar la clave
        first = false;
    }


    std::string parametrosSrt = parametrosYvalores.str();
    char* parametoschar = new char[parametrosSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(parametrosSrt.begin(), parametrosSrt.end(), parametoschar);
    parametoschar[parametrosSrt.size()] = '\0';


    std::string strId=std::to_string(deviceid);
    char selecionador[1000] ="id=";
    strcat(selecionador,strId.c_str() );


    char *json_result = actuaDisp(conn,parametoschar,selecionador);
    auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

    free(json_result);  // Libera la memoria después de usarla

    return json_obj;

}




crow::response elimDisp(const crow::request& req, int deviceid) {


    std::string strId=std::to_string(deviceid);
    char selecionador[1000] ="";
    strcat(selecionador,strId.c_str() );

    int valor = EliminarDisp(conn,selecionador);

    if (valor==0){
        return crow::response(204);
    }else{
        return crow::response(500);
    }

}


crow::json::wvalue getDrivers(const crow::request& req, int path_id = -1) {
    bool all = req.url_params.get("all") ? (req.url_params.get("all") == std::string("true")) : false;
    int userId = req.url_params.get("userId") ? std::stoi(req.url_params.get("userId")) : -1;
    int deviceId = req.url_params.get("deviceId") ? std::stoi(req.url_params.get("deviceId")) : -1;
    int groupId = req.url_params.get("groupId") ? std::stoi(req.url_params.get("groupId")) : -1;
    bool refresh = req.url_params.get("refresh") ? (req.url_params.get("refresh") == std::string("true")) : false;



    std::vector<std::string> condiciones;

    if(path_id!=-1){
        condiciones.push_back("d.id="+std::to_string(path_id));
    }
    if(userId!=-1){//para user id
        condiciones.push_back("u.userid="+std::to_string(userId));
    }
    if (deviceId != -1) {
        condiciones.push_back("de.deviceid=" + std::to_string(deviceId));
    }
    if (groupId != -1) {
        condiciones.push_back("g.groupid=" + std::to_string(groupId));
    }

    if (all == true || condiciones.size()==0) {      //comportamiento por defecto
        char *json_result = conAllConductores(conn);
        auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

        free(json_result);  // Libera la memoria después de usarla

        return json_obj;
    }else if(path_id!=-1){
        std::string resultado = condiciones[0];
        for (size_t i = 1; i < condiciones.size(); ++i) {
            resultado += " AND " + condiciones[i];
        }

        // Convertir el resultado en un char*
        char* resultChar = new char[resultado.size() + 1];
        std::copy(resultado.begin(), resultado.end(), resultChar);
        resultChar[resultado.size()] = '\0';

        char *json_result = conCondUniConductores(conn,resultChar);
        auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

        free(json_result);  // Libera la memoria después de usarla

        return json_obj;
    }
    else{                                                         //queri especifico

        std::string resultado = condiciones[0];
        for (size_t i = 1; i < condiciones.size(); ++i) {
            resultado += " AND " + condiciones[i];
        }

        // Convertir el resultado en un char*
        char* resultChar = new char[resultado.size() + 1];
        std::copy(resultado.begin(), resultado.end(), resultChar);
        resultChar[resultado.size()] = '\0';

        char *json_result = conCondConductores(conn,resultChar);
        auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

        free(json_result);  // Libera la memoria después de usarla

        return json_obj;

    }

    return crow::json::wvalue("");
}



crow::json::wvalue crearConduc(const crow::request& req) {

    auto data = json::parse(req.body);



    std::map<std::string, std::string> parPreMap;
    std::map<std::string, std::string> parametersMap = {
        {"id", "id"},
        {"name", "name"},
        {"uniqueId", "uniqueid"},
        {"attributes", "attributes"}
    };

    if(!data.contains("attributes")){
            parPreMap["attributes"] ="'{}'";
        }

    for (const auto& param : parametersMap) {

        if(data.contains(param.first)){
            if (data.at(param.first).is_string()) {

                parPreMap[param.second] = "'"+data.at(param.first).get<std::string>()+"'"; // Extraer directamente si es string
            } else if(data.at(param.first).is_object()){ // para los objetos
                parPreMap[param.second] = "'"+data.at(param.first).dump()+"'";
            }
            else{

                parPreMap[param.second] = data.at(param.first).dump();  // Convertir a string si es otro tipo

            }
        }
    }


    std::stringstream parametros, valores;
    parametros << "(";
    valores << "(";
    // Iterar sobre el mapa y agregar las claves (first) separadas por coma
    bool first = true;
    for (const auto& entry : parPreMap) {
        if (!first) {
            parametros << ",";
            valores << ",";            // Agregar coma si no es el primer elemento
        }
        parametros << entry.first;
        valores  << entry.second;      // Agregar la clave
        first = false;
    }

    // Terminar con el paréntesis derecho
    parametros << ")";
    valores << ")";

    std::string parametrosSrt = parametros.str();
    char* parametoschar = new char[parametrosSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(parametrosSrt.begin(), parametrosSrt.end(), parametoschar);
    parametoschar[parametrosSrt.size()] = '\0';

    std::string valoresSrt = valores.str();
    char* valoreschar = new char[valoresSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(valoresSrt.begin(), valoresSrt.end(), valoreschar);
    valoreschar[valoresSrt.size()] = '\0';



    char *json_result = guardarConductor(conn,parametoschar,valoreschar);
    auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

    free(json_result);  // Libera la memoria después de usarla

    return json_obj;

}


crow::json::wvalue actualizarConduc(const crow::request& req, int deviceid) {

    auto data = json::parse(req.body);



    std::map<std::string, std::string> parPreMap;
    std::map<std::string, std::string> parametersMap = {
        {"id", "id"},
        {"name", "name"},
        {"uniqueId", "uniqueid"},
        {"attributes", "attributes"}
    };

    for (const auto& param : parametersMap) {

        if(data.contains(param.first)){
            if (data.at(param.first).is_string()) {

                parPreMap[param.second] = "'"+data.at(param.first).get<std::string>()+"'"; // Extraer directamente si es string
            } else if(data.at(param.first).is_object()){ // para los objetos
                parPreMap[param.second] = "'"+data.at(param.first).dump()+"'";
            }
            else{

                parPreMap[param.second] = data.at(param.first).dump();  // Convertir a string si es otro tipo

            }
        }
    }


    std::stringstream parametrosYvalores;


    // Iterar sobre el mapa y agregar las claves (first) separadas por coma
    bool first = true;
    for (const auto& entry : parPreMap) {
        if (!first) {
            parametrosYvalores  << " , ";          // Agregar coma si no es el primer elemento
        }
        parametrosYvalores << entry.first;
        parametrosYvalores << "=";
        parametrosYvalores  << entry.second;      // Agregar la clave
        first = false;
    }


    std::string parametrosSrt = parametrosYvalores.str();
    char* parametoschar = new char[parametrosSrt.size() + 1];  // +1 para el terminador '\0'
    std::copy(parametrosSrt.begin(), parametrosSrt.end(), parametoschar);
    parametoschar[parametrosSrt.size()] = '\0';


    std::string strId=std::to_string(deviceid);
    char selecionador[1000] ="id=";
    strcat(selecionador,strId.c_str() );


    char *json_result = actuaConductor(conn,parametoschar,selecionador);
    auto json_obj = crow::json::load(json_result);  // Convierte a JSON real

    free(json_result);  // Libera la memoria después de usarla

    return json_obj;

}



crow::response eliminarConduc(const crow::request& req, int deviceid) {


    std::string strId=std::to_string(deviceid);
    char selecionador[1000] ="";
    strcat(selecionador,strId.c_str() );

    int valor = eliminarConductor(conn,selecionador);

    if (valor==0){
        return crow::response(204);
    }else{
        return crow::response(500);
    }

}



crow::response actualizarAcumu(const crow::request& req, int deviceid) {

    std::string strId=std::to_string(deviceid);
    char charid[1000] ="";
    strcat(charid,strId.c_str() );

    auto data = json::parse(req.body);
    data.erase("deviceId");

    auto valorAnterires= json::parse(conUniAcumuladores(conn,charid));

    valorAnterires.update(data);

    std::string jsonString = valorAnterires.dump();
    const char* jsonChar = jsonString.c_str();
    char *jsonArray = (char*)malloc(strlen(jsonChar) + 5); // +3 para agregar comillas y el terminador de cadena
    if (jsonArray == NULL) {
        fprintf(stderr, "Error al asignar memoria.\n");
        return crow::response(500);
    }

    // Agregar comillas alrededor del JSON
    sprintf(jsonArray, "'%s'", jsonChar);

    int valor = ActualizarAcumuladores(conn,charid,jsonArray);

    if (valor==0){
        return crow::response(204);
    }else{
        return crow::response(500);
    }


}


std::string get_mime_type(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mime_types = {
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".webp", "image/webp"}
    };

    auto it = mime_types.find(extension);
    return (it != mime_types.end()) ? it->second : "application/octet-stream"; // Default binary type
}




void server(){
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/devices").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        return get_devices(req);
    });

    CROW_ROUTE(app, "/api/devices/<int>").methods(crow::HTTPMethod::GET)([](const crow::request& req, int deviceid) {
        return get_devices_unicos(req,deviceid);
    });


    CROW_ROUTE(app, "/api/devices").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        return crearDisp(req);
    });

    CROW_ROUTE(app, "/api/devices/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request& req,int deviceid) {
        return actualizarDisp(req, deviceid);
    });

    CROW_ROUTE(app, "/api/devices/<int>").methods(crow::HTTPMethod::DELETE)([](const crow::request& req,int deviceid) {
        return elimDisp(req, deviceid);
    });



    CROW_ROUTE(app, "/api/drivers").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        return getDrivers(req);
    });

    CROW_ROUTE(app, "/api/drivers/<int>").methods(crow::HTTPMethod::GET)([](const crow::request& req,int deviceid) {
        return getDrivers(req, deviceid);
    });

    CROW_ROUTE(app, "/api/drivers").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        return crearConduc(req);
    });

    CROW_ROUTE(app, "/api/drivers/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request& req,int deviceid) {
        return actualizarConduc(req, deviceid);
    });

    CROW_ROUTE(app, "/api/drivers/<int>").methods(crow::HTTPMethod::DELETE)([](const crow::request& req,int deviceid) {
        return eliminarConduc(req, deviceid);
    });




    //acualizar acumiladores

    CROW_ROUTE(app, "/api/drivers/<int>/accumulators").methods(crow::HTTPMethod::PUT)([](const crow::request& req,int deviceid) {
        return actualizarAcumu(req, deviceid);
    });

    //Guardar img
    CROW_ROUTE(app, "/api/devices/<int>/image").methods(crow::HTTPMethod::POST)([](const crow::request& req, int device_id) {
        // Get content type from the request header
        auto content_type = req.get_header_value("Content-Type");

        // Get the file extension based on content type
        std::string extension = get_extension(content_type.c_str());

        if (extension.empty()) {
            // If the extension is empty, return 415 Unsupported Media Type
            return crow::response(415, "Unsupported Media Type");
        }

        // Extract image data from the request body
        std::vector<uint8_t> image_data(req.body.begin(), req.body.end());

        // Generate the filename using the device ID
        std::string filename = "device_" + std::to_string(device_id) + extension;

        // Use the device ID as the subfolder name for saving the image
        std::string subfolder = std::to_string(device_id);

        // Call save_image from the upload.h, passing the subfolder, filename, and image data
        if (save_image(subfolder.c_str(), filename.c_str(), image_data.data(), image_data.size())) {
            return crow::response(200, "Image saved as " + filename);
        } else {
            return crow::response(500, "Failed to save image");
        }
    });





    CROW_ROUTE(app, "/media/<string>/<string>")
        .methods(crow::HTTPMethod::GET)([](const crow::request&, std::string subfolder, std::string filename) {

            // Construct the full file path
            std::string file_path = std::string(UPLOAD_FOLDER) + "/" + subfolder + "/" + filename;

            // Open the file
            std::ifstream file(file_path, std::ios::binary);
            if (!file) {
                return crow::response(404, "Image not found");
            }

            // Read file contents
            std::ostringstream buffer;
            buffer << file.rdbuf();
            std::string file_content = buffer.str();

            // Determine MIME type based on file extension
            std::string extension = filename.substr(filename.find_last_of("."));
            std::string content_type = get_mime_type(extension);

            // Create response with image data
            crow::response res(file_content);
            res.set_header("Content-Type", content_type);
            return res;
        });



    app.port(8080).multithreaded().run();
}








int main() {
    conn= conectarDB();
    server();

}
