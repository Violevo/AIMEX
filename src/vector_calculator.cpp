#include "vector_calculator.h"
#include <cmath>

VectorCalculator::VectorCalculator() : frame_center_(0, 0) {}

void VectorCalculator::set_frame_center(int width, int height) {
    frame_center_ = cv::Point2f(width / 2.0f, height / 2.0f);
}

cv::Point2f VectorCalculator::calculate_vector(const cv::Point2f& target_point) {
    if (target_point.x < 0 || target_point.y < 0) {
        return cv::Point2f(0, 0); // Invalid point
    }

    // Calculate vector from center to target
    cv::Point2f vector = target_point - frame_center_;

    // Invert Y axis (screen coordinates vs. mathematical coordinates)
    vector.y = -vector.y;

    return vector;
}

void VectorCalculator::normalize_vector(cv::Point2f& vector, float max_magnitude) {
    float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);

    if (magnitude > 0) {
        // Normalize to unit vector, then scale by max_magnitude
        vector.x = (vector.x / magnitude) * max_magnitude;
        vector.y = (vector.y / magnitude) * max_magnitude;
    }
}