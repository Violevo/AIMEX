#include "ndi_receiver.h"
#include <iostream>
#include <thread>
#include <chrono>

NDIReceiver::NDIReceiver() : receiver_(nullptr), finder_(nullptr), initialized_(false) {}

NDIReceiver::~NDIReceiver() {
    shutdown();
}

bool NDIReceiver::initialize() {
    if (!NDIlib_initialize()) {
        std::cerr << "Failed to initialize NDI library" << std::endl;
        return false;
    }

    // Create finder
    NDIlib_find_create_t find_desc;
    find_desc.show_local_sources = true;
    find_desc.p_groups = nullptr;
    find_desc.p_extra_ips = nullptr;

    finder_ = NDIlib_find_create_v2(&find_desc);
    if (!finder_) {
        std::cerr << "Failed to create NDI finder" << std::endl;
        return false;
    }

    initialized_ = true;
    return true;
}

bool NDIReceiver::connect(const std::string& source_name) {
    if (!initialized_) return false;

    // Wait for sources
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    uint32_t num_sources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(finder_, &num_sources);

    // Find matching source
    const NDIlib_source_t* selected_source = nullptr;
    for (uint32_t i = 0; i < num_sources; i++) {
        if (source_name.empty() || std::string(sources[i].p_ndi_name).find(source_name) != std::string::npos) {
            selected_source = &sources[i];
            break;
        }
    }

    if (!selected_source) {
        std::cerr << "Source not found: " << source_name << std::endl;
        return false;
    }

    // Create receiver
    NDIlib_recv_create_v3_t recv_desc;
    recv_desc.source_to_connect_to = *selected_source;
    recv_desc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
    recv_desc.bandwidth = NDIlib_recv_bandwidth_highest;
    recv_desc.allow_video_fields = true;
    recv_desc.p_ndi_recv_name = "VideoProcessor";

    receiver_ = NDIlib_recv_create_v3(&recv_desc);
    if (!receiver_) {
        std::cerr << "Failed to create NDI receiver" << std::endl;
        return false;
    }

    std::cout << "Connected to: " << selected_source->p_ndi_name << std::endl;
    return true;
}

bool NDIReceiver::receive_frame(cv::Mat& frame) {
    if (!receiver_) return false;

    NDIlib_video_frame_v2_t video_frame;

    switch (NDIlib_recv_capture_v2(receiver_, &video_frame, nullptr, nullptr, 1000)) {
    case NDIlib_frame_type_video:
        // Convert NDI frame to OpenCV Mat
        frame = cv::Mat(video_frame.yres, video_frame.xres, CV_8UC4, video_frame.p_data, video_frame.line_stride_in_bytes);

        // Convert BGRA to BGR
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);

        // Free the frame
        NDIlib_recv_free_video_v2(receiver_, &video_frame);
        return true;

    case NDIlib_frame_type_none:
        // No frame available
        return false;

    default:
        return false;
    }
}

std::vector<std::string> NDIReceiver::get_sources() {
    std::vector<std::string> source_names;

    if (!finder_) return source_names;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    uint32_t num_sources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(finder_, &num_sources);

    for (uint32_t i = 0; i < num_sources; i++) {
        source_names.push_back(sources[i].p_ndi_name);
    }

    return source_names;
}

void NDIReceiver::shutdown() {
    if (receiver_) {
        NDIlib_recv_destroy(receiver_);
        receiver_ = nullptr;
    }

    if (finder_) {
        NDIlib_find_destroy(finder_);
        finder_ = nullptr;
    }

    if (initialized_) {
        NDIlib_destroy();
        initialized_ = false;
    }
}