#pragma once
#include <string>

struct AppConfig {
    // NDI settings
    std::string ndi_source_name = "";

    // Color filtering settings
    int hue_min = 0;
    int hue_max = 180;
    int saturation_min = 50;
    int saturation_max = 255;
    int value_min = 50;
    int value_max = 255;

    // Mouse sensitivity
    float mouse_sensitivity = 1.0f;

    // Web server settings
    int web_port = 8080;

    // USB device settings
    int vendor_id = 0x1234;  // Replace with MACKU device VID
    int product_id = 0x5678; // Replace with MACKU device PID
};

extern AppConfig g_config;