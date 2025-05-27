#pragma once
#include <Processing.NDI.Lib.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>

class NDIReceiver {
public:
    NDIReceiver();
    ~NDIReceiver();

    bool initialize();
    bool connect(const std::string& source_name);
    bool receive_frame(cv::Mat& frame);
    std::vector<std::string> get_sources();
    void shutdown();

private:
    NDIlib_recv_instance_t receiver_;
    NDIlib_find_instance_t finder_;
    bool initialized_;
};