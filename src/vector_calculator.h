#pragma once
#include <opencv2/opencv.hpp>

class VectorCalculator {
public:
    VectorCalculator();

    void set_frame_center(int width, int height);
    cv::Point2f calculate_vector(const cv::Point2f& target_point);
    void normalize_vector(cv::Point2f& vector, float max_magnitude = 1.0f);

private:
    cv::Point2f frame_center_;
};