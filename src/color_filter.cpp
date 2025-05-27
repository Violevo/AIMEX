#include "color_filter.h"
#include "config.h"

ColorFilter::ColorFilter() {
    // Initialize with default values
    set_hsv_range(g_config.hue_min, g_config.hue_max,
        g_config.saturation_min, g_config.saturation_max,
        g_config.value_min, g_config.value_max);
}

void ColorFilter::set_hsv_range(int hue_min, int hue_max, int sat_min, int sat_max, int val_min, int val_max) {
    hsv_lower_ = cv::Scalar(hue_min, sat_min, val_min);
    hsv_upper_ = cv::Scalar(hue_max, sat_max, val_max);
}

cv::Mat ColorFilter::apply_filter(const cv::Mat& input) {
    cv::Mat hsv, filtered;

    // Convert BGR to HSV
    cv::cvtColor(input, hsv, cv::COLOR_BGR2HSV);

    // Apply color range filter
    cv::inRange(hsv, hsv_lower_, hsv_upper_, filtered);

    // Apply morphological operations to clean up noise
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(filtered, filtered, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(filtered, filtered, cv::MORPH_CLOSE, kernel);

    return filtered;
}

cv::Point2f ColorFilter::find_highest_point(const cv::Mat& filtered_image) {
    // Find all contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(filtered_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return cv::Point2f(-1, -1); // No objects found
    }

    // Find the largest contour
    double max_area = 0;
    int max_contour_idx = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > max_area) {
            max_area = area;
            max_contour_idx = i;
        }
    }

    // Find the topmost point of the largest contour
    cv::Point highest_point = contours[max_contour_idx][0];
    for (const auto& point : contours[max_contour_idx]) {
        if (point.y < highest_point.y) { // Lower y value = higher position
            highest_point = point;
        }
    }

    return cv::Point2f(highest_point.x, highest_point.y);
}