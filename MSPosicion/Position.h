#pragma once
#include <trantor/utils/Date.h>
#include <json/json.h>
#include <string>

class Position {
public:
    int id;
    std::string protocol;
    int deviceId;
    trantor::Date serverTime;
    trantor::Date deviceTime;
    trantor::Date fixTime;
    bool valid;
    double latitude;
    double longitude;
    float altitude;
    float speed;
    float course;
    std::string address;
    std::string attributes;
    double accuracy;
    std::string network;

    Json::Value toJson() const;
    static Position fromJson(const Json::Value &json);
};