#pragma once
#include <httplib.h>
#include <thread>
#include <atomic>
#include <string>

class WebServer {
public:
    WebServer(int port);
    ~WebServer();

    bool start();
    void stop();
    bool is_running() const;

private:
    void setup_routes();
    std::string get_config_json();
    bool update_config_from_json(const std::string& json);

    httplib::Server server_;
    std::thread server_thread_;
    std::atomic<bool> running_;
    int port_;
};