#pragma once
#include <opencv2/opencv.hpp>

class ColorFilter {
public:
    ColorFilter();

    void set_hsv_range(int hue_min, int hue_max, int sat_min, int sat_max, int val_min, int val_max);
    cv::Mat apply_filter(const cv::Mat& input);
    cv::Point2f find_highest_point(const cv::Mat& filtered_image);

private:
    cv::Scalar hsv_lower_;
    cv::Scalar hsv_upper_;
};