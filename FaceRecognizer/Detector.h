#pragma once
#include "Common.h"
#include "opencv2/opencv.hpp"


struct FR_API Detector {
    Detector(std::string classifier_config);
    std::vector<cv::Rect> Detect(cv::Mat image);
    std::vector<cv::Mat> Extract(cv::Mat image);
    cv::Mat GetLargest(std::vector<cv::Mat> images) const;
    cv::Mat ExtractLargest(cv::Mat image);

    double scale_factor = 1.1;
    int min_neighbours = 6;
    cv::Size min_size;
    cv::Size max_size;

    private:

    cv::CascadeClassifier classifier;
};
