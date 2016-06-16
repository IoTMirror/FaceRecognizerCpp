#pragma once

#include "Common.h"
#include <string>
#include "opencv2/opencv.hpp"
#undef max

struct FR_API Recognizer
{
    Recognizer(std::string data_directory, double threshold);
    int Recognize(cv::Mat image);
    int Recognize(cv::Mat image, std::vector<int> users_to_check);
    void Train(int userId, cv::Mat image);

    private:
    std::string data_directory;
    double threshold = std::numeric_limits<double>::max();

    cv::Ptr<cv::face::LBPHFaceRecognizer> LoadRecognizer(int userId);
};
