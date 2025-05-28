#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <atomic>

#include "config.h"
#include "ndi_receiver.h"
#include "color_filter.h"
#include "vector_calculator.h"
#include "usb_sender.h"
#include "web_server.h"

// Global configuration instance
AppConfig g_config;

// Global flag for graceful shutdown
std::atomic<bool> g_running(true);

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
}

int main() {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "Application Starting" << std::endl;

    // Initialize components
    NDIReceiver ndi_receiver;
    ColorFilter color_filter;
    VectorCalculator vector_calc;
    USBSender usb_sender;
    WebServer web_server(g_config.web_port);

    // Start web server
    if (!web_server.start()) {
        std::cerr << "Failed to start web server" << std::endl;
        return 1;
    }

    std::cout << "Web interface available at: http://localhost:" << g_config.web_port << std::endl;

    // Initialize NDI
    if (!ndi_receiver.initialize()) {
        std::cerr << "Failed to initialize NDI receiver" << std::endl;
        return 1;
    }

    // Initialize USB
    if (!usb_sender.initialize()) {
        std::cerr << "Failed to initialize Makcu" << std::endl;
        return 1;
    }

    // Main processing loop
    cv::Mat frame, filtered_frame;
    bool ndi_connected = false;
    bool usb_connected = false;

    while (g_running) {
        // Check NDI connection
        if (!ndi_connected) {
            if (!g_config.ndi_source_name.empty()) {
                std::cout << "Attempting to connect to NDI source: " << g_config.ndi_source_name << std::endl;
                if (ndi_receiver.connect(g_config.ndi_source_name)) {
                    ndi_connected = true;
                }
                else {
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    continue;
                }
            }
            else {
                std::cout << "No NDI source configured. Please configure via web interface." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
        }

        // Check USB connection
        if (!usb_connected) {
            std::cout << "Attempting to connect to Makcu..." << std::endl;
            if (usb_sender.connect_device(g_config.vendor_id, g_config.product_id)) {
                usb_connected = true;
            }
            else {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
        }

        // Receive frame from NDI
        if (!ndi_receiver.receive_frame(frame)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Set frame center for vector calculation
        vector_calc.set_frame_center(frame.cols, frame.rows);

        // Apply color filtering
        color_filter.set_hsv_range(g_config.hue_min, g_config.hue_max,
            g_config.saturation_min, g_config.saturation_max,
            g_config.value_min, g_config.value_max);

        filtered_frame = color_filter.apply_filter(frame);

        // Find highest point
        cv::Point2f highest_point = color_filter.find_highest_point(filtered_frame);

        // Calculate vector from center to highest point
        cv::Point2f movement_vector = vector_calc.calculate_vector(highest_point);

        // Normalize vector for mouse movement
        vector_calc.normalize_vector(movement_vector, 50.0f); // Max movement (in pixels) per frame  <-------- REPLACE WITH REAL SMOOTHING THIS IS VERY BAD

        // Send mouse movement if valid point found
        if (highest_point.x >= 0 && highest_point.y >= 0) {
            if (!usb_sender.send_mouse_movement(movement_vector)) {
                std::cerr << "Failed to send mouse movement, attempting to reconnect..." << std::endl;
                usb_connected = false;
            }
            else {
                std::cout << "Mouse movement sent: (" << movement_vector.x << ", " << movement_vector.y << ")" << std::endl;
			}
        }

        // Small delay to prevent overwhelming the system
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    std::cout << "Shutting down..." << std::endl;

    // Cleanup
    web_server.stop();
    ndi_receiver.shutdown();
    usb_sender.disconnect();

    return 0;
}