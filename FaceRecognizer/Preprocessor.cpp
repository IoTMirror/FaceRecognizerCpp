#include "Common.h"
#include "Preprocessor.h"

cv::Mat Preprocess(cv::Mat image, int32_t width, int32_t height)
{
    cv::Mat result;
    resize(image, result, cv::Size(width, height));
    cvtColor(result, result, cv::COLOR_BGR2GRAY);
    equalizeHist(result, result);
    return result;
}
