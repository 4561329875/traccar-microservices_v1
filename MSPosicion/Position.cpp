#include "Position.h"

Json::Value Position::toJson() const {
    Json::Value json;
    json["id"] = id;
    json["protocol"] = protocol;
    json["deviceId"] = deviceId;
    json["serverTime"] = serverTime.toDbStringLocal();
    json["deviceTime"] = deviceTime.toDbStringLocal();
    json["fixTime"] = fixTime.toDbStringLocal();
    json["valid"] = valid;
    json["latitude"] = latitude;
    json["longitude"] = longitude;
    json["altitude"] = altitude;
    json["speed"] = speed;
    json["course"] = course;
    json["address"] = address;
    
    // ✅ Corregido: Parsear attributes como JSON en lugar de dejarlo en null
    Json::CharReaderBuilder reader;
    std::unique_ptr<Json::CharReader> jsonReader(reader.newCharReader());
    Json::Value attributesJson;
    std::string errors;
    
    if (!attributes.empty() && jsonReader->parse(attributes.c_str(), attributes.c_str() + attributes.size(), &attributesJson, &errors)) {
        json["attributes"] = attributesJson;
    } else {
        json["attributes"] = Json::Value(Json::objectValue); // Retorna un JSON vacío si está mal formateado
    }
    
    json["accuracy"] = accuracy;
    json["network"] = network;
    
    return json;
}

Position Position::fromJson(const Json::Value &json) {
    Position pos;
    pos.id = json["id"].asInt();
    pos.protocol = json["protocol"].asString();
    pos.deviceId = json["deviceId"].asInt();
    pos.serverTime = trantor::Date::fromDbStringLocal(json["serverTime"].asString());
    pos.deviceTime = trantor::Date::fromDbStringLocal(json["deviceTime"].asString());
    pos.fixTime = trantor::Date::fromDbStringLocal(json["fixTime"].asString());
    pos.valid = json["valid"].asBool();
    pos.latitude = json["latitude"].asDouble();
    pos.longitude = json["longitude"].asDouble();
    pos.altitude = json["altitude"].asFloat();
    pos.speed = json["speed"].asFloat();
    pos.course = json["course"].asFloat();
    pos.address = json["address"].asString();
    
    // ✅ Corregido: Serializar attributes correctamente
    Json::StreamWriterBuilder writer;
    pos.attributes = Json::writeString(writer, json["attributes"]); 
    
    pos.accuracy = json["accuracy"].asDouble();
    pos.network = json["network"].asString();
    return pos;
}
