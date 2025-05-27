#include "usb_sender.h"
#include "config.h"
#include <iostream>

USBSender::USBSender() : device_(nullptr), connected_(false) {}

USBSender::~USBSender() {
    disconnect();
}

bool USBSender::initialize() {
    try {
        device_ = std::make_unique<makcu::Device>();
        return true;
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "Failed to initialize MAKCU device: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize MAKCU device: " << e.what() << std::endl;
        return false;
    }
}

bool USBSender::connect_device(int vendor_id, int product_id) {
    // Parameters are kept for compatibility but not used with MAKCU API
    return find_and_connect();
}

bool USBSender::find_and_connect() {
    if (!device_) {
        std::cerr << "Device not initialized" << std::endl;
        return false;
    }

    try {
        // Find available MAKCU devices
        auto devices = makcu::Device::findDevices();

        if (devices.empty()) {
            std::cerr << "No MAKCU devices found" << std::endl;
            return false;
        }

        std::cout << "Found " << devices.size() << " MAKCU device(s)" << std::endl;
        for (const auto& dev : devices) {
            std::cout << "  Port: " << dev.port << ", Description: " << dev.description << std::endl;
        }

        // Connect to the first available device
        std::cout << "Connecting to " << devices[0].port << "..." << std::endl;

        if (!device_->connect(devices[0].port)) {
            std::cerr << "Failed to connect to MAKCU device" << std::endl;
            return false;
        }

        // Get and display device information
        auto deviceInfo = device_->getDeviceInfo();
        std::cout << "Successfully connected to MAKCU device:" << std::endl;
        std::cout << "  Port: " << deviceInfo.port << std::endl;
        std::cout << "  VID: 0x" << std::hex << deviceInfo.vid << std::endl;
        std::cout << "  PID: 0x" << std::hex << deviceInfo.pid << std::dec << std::endl;
        std::cout << "  Version: " << device_->getVersion() << std::endl;

        connected_ = true;
        return true;
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error connecting to MAKCU device: " << e.what() << std::endl;
        return false;
    }
}

bool USBSender::send_mouse_movement(const cv::Point2f& movement) {
    if (!connected_ || !device_ || !device_->isConnected()) {
        return false;
    }

    try {
        // Scale movement by sensitivity
        int scaled_x = static_cast<int>(movement.x * g_config.mouse_sensitivity);
        int scaled_y = static_cast<int>(movement.y * g_config.mouse_sensitivity);

        // Send mouse movement using MAKCU API
        device_->mouseMove(scaled_x, scaled_y);
        return true;
    }
    catch (const makcu::MakcuException& e) {
        std::cerr << "MAKCU Error sending mouse movement: " << e.what() << std::endl;
        connected_ = false; // Mark as disconnected so we can attempt reconnection
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error sending mouse movement: " << e.what() << std::endl;
        connected_ = false;
        return false;
    }
}

bool USBSender::is_connected() const {
    if (!device_) return false;
    return connected_ && device_->isConnected();
}

void USBSender::disconnect() {
    if (device_ && connected_) {
        try {
            device_->disconnect();
            std::cout << "MAKCU device disconnected" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during disconnect: " << e.what() << std::endl;
        }
    }

    connected_ = false;
    device_.reset();
}