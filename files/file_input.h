#pragma once

#include <iostream>
#include "LiteMath.h"

using namespace LiteMath;

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class InputJson {
public:
    static void read_camera(std::string path, float3* position, float3* lookAt, float* FOV) {
        std::ifstream f(path);
        json data = json::parse(f);

        try {
            *position = get_float3_json_field(data, "position");
            *lookAt = get_float3_json_field(data, "lookAt");
            *FOV = get_float_json_field(data, "FOV");
        }
        catch (std::string error) {
            std::cout << error;
        }
    }

private:
    static float3 get_float3_json_field(json data, std::string field) {
        if (data.contains(field) && std::size(data[field]) == 3) {
            data = data[field];
            return float3(data[0], data[1], data[2]);
        }
        else {
            throw "Something wrong with " + field;
        }
    }

    static float get_float_json_field(json data, std::string field) {
        if (data.contains(field)) {
            data = data[field];
            return data;
        }
        else {
            throw "Something wrong with " + field;
        }
    }
};