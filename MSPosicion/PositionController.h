#pragma once
#include <drogon/HttpController.h>
#include "Position.h"
#include <vector>

class PositionController : public drogon::HttpController<PositionController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PositionController::createPosition, "/api/positions", drogon::Post);  // Prefijo /api agregado
    ADD_METHOD_TO(PositionController::getPosition, "/api/positions/{1:int}", drogon::Get);
    ADD_METHOD_TO(PositionController::updatePosition, "/api/positions/{1:int}", drogon::Put);
    ADD_METHOD_TO(PositionController::deletePosition, "/api/positions/{1:int}", drogon::Delete);
    ADD_METHOD_TO(PositionController::listPositions, "/api/positions", drogon::Get);
    METHOD_LIST_END

    void createPosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void getPosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id);
    void updatePosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id);
    void deletePosition(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, int id);
    void listPositions(const drogon::HttpRequestPtr &req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);

private:
    std::vector<Position> positions;
};
