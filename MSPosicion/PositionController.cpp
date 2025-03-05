// PositionController.cpp
#include "PositionController.h"
#include <drogon/HttpResponse.h>

void PositionController::createPosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        callback(drogon::HttpResponse::newHttpJsonResponse(Json::Value("Invalid JSON")));
        return;
    }
    Position pos = Position::fromJson(*json);
    positions.push_back(pos);
    callback(drogon::HttpResponse::newHttpJsonResponse(pos.toJson()));
}

void PositionController::getPosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id) {
    for (const auto &pos : positions) {
        if (pos.id == id) {
            callback(drogon::HttpResponse::newHttpJsonResponse(pos.toJson()));
            return;
        }
    }
    // ✅ Corrección: Se agrega ContentType en newHttpResponse()
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k404NotFound);
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(response);
}

void PositionController::updatePosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id) {
    auto json = req->getJsonObject();
    if (!json) {
        callback(drogon::HttpResponse::newHttpJsonResponse(Json::Value("Invalid JSON")));
        return;
    }
    for (auto &pos : positions) {
        if (pos.id == id) {
            pos = Position::fromJson(*json);
            callback(drogon::HttpResponse::newHttpJsonResponse(pos.toJson()));
            return;
        }
    }
    // ✅ Corrección: Se agrega ContentType en newHttpResponse()
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k404NotFound);
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(response);
}

void PositionController::deletePosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id) {
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        if (it->id == id) {
            positions.erase(it);
            // ✅ Corrección: Se agrega ContentType en newHttpResponse()
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(drogon::k204NoContent);
            response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            callback(response);
            return;
        }
    }
    // ✅ Corrección: Se agrega ContentType en newHttpResponse()
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k404NotFound);
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(response);
}

void PositionController::listPositions(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    Json::Value jsonArray(Json::arrayValue);
    for (const auto &pos : positions) {
        jsonArray.append(pos.toJson());
    }
    callback(drogon::HttpResponse::newHttpJsonResponse(jsonArray));
}
