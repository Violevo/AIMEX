#pragma once
#include "makcu.h"
#include <opencv2/opencv.hpp>
#include <memory>

class USBSender {
public:
    USBSender();
    ~USBSender();

    bool initialize();
    bool connect_device(int vendor_id = 0, int product_id = 0); // Parameters kept for compatibility but not used
    bool send_mouse_movement(const cv::Point2f& movement);
    void disconnect();
    bool is_connected() const;

private:
    std::unique_ptr<makcu::Device> device_;
    bool connected_;

    // Helper method to find and connect to first available MAKCU device
    bool find_and_connect();
};