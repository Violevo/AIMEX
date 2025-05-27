#include "web_server.h"
#include "config.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

WebServer::WebServer(int port) : running_(false), port_(port) {}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    if (running_) return true;

    setup_routes();

    server_thread_ = std::thread([this]() {
        running_ = true;
        std::cout << "Web server starting on port " << port_ << std::endl;
        server_.listen("0.0.0.0", port_);
        running_ = false;
        });

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return true;
}

void WebServer::stop() {
    if (running_) {
        server_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        running_ = false;
    }
}

bool WebServer::is_running() const {
    return running_;
}

void WebServer::setup_routes() {
    // Serve static files
    server_.set_mount_point("/", "./web");

    // API endpoint to get current configuration
    server_.Get("/api/config", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(get_config_json(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        });

    // API endpoint to update configuration
    server_.Post("/api/config", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        if (update_config_from_json(req.body)) {
            res.set_content("{\"status\":\"success\"}", "application/json");
        }
        else {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Invalid JSON\"}", "application/json");
        }
        });

    // Handle CORS preflight requests
    server_.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        });
}

std::string WebServer::get_config_json() {
    json config_json;
    config_json["ndi_source_name"] = g_config.ndi_source_name;
    config_json["hue_min"] = g_config.hue_min;
    config_json["hue_max"] = g_config.hue_max;
    config_json["saturation_min"] = g_config.saturation_min;
    config_json["saturation_max"] = g_config.saturation_max;
    config_json["value_min"] = g_config.value_min;
    config_json["value_max"] = g_config.value_max;
    config_json["mouse_sensitivity"] = g_config.mouse_sensitivity;
    config_json["web_port"] = g_config.web_port;
    config_json["vendor_id"] = g_config.vendor_id;
    config_json["product_id"] = g_config.product_id;

    return config_json.dump(2);
}

bool WebServer::update_config_from_json(const std::string& json_str) {
    try {
        json config_json = json::parse(json_str);

        if (config_json.contains("ndi_source_name"))
            g_config.ndi_source_name = config_json["ndi_source_name"];
        if (config_json.contains("hue_min"))
            g_config.hue_min = config_json["hue_min"];
        if (config_json.contains("hue_max"))
            g_config.hue_max = config_json["hue_max"];
        if (config_json.contains("saturation_min"))
            g_config.saturation_min = config_json["saturation_min"];
        if (config_json.contains("saturation_max"))
            g_config.saturation_max = config_json["saturation_max"];
        if (config_json.contains("value_min"))
            g_config.value_min = config_json["value_min"];
        if (config_json.contains("value_max"))
            g_config.value_max = config_json["value_max"];
        if (config_json.contains("mouse_sensitivity"))
            g_config.mouse_sensitivity = config_json["mouse_sensitivity"];
        if (config_json.contains("vendor_id"))
            g_config.vendor_id = config_json["vendor_id"];
        if (config_json.contains("product_id"))
            g_config.product_id = config_json["product_id"];

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}