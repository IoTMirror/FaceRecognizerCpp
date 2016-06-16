#include "Detector.h"

using namespace std;
using namespace cv;

Detector::Detector(string classifier_config)
{
    if (!classifier.load(classifier_config)) { printf("Error loading cascade"); }
}


vector<Mat> Detector::Extract(Mat image)
{
    vector<Mat> extracted_faces;
    Mat gray_image;
    cvtColor(image, gray_image, CV_BGR2GRAY);
    equalizeHist(gray_image, gray_image);
    vector<Rect> faces;
    classifier.detectMultiScale(gray_image, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(50, 50));
    for (int i = 0; i < faces.size(); i++)
    {
        Rect roi = faces[i];
        if (roi.width > 0 && roi.height > 0)
        {
            Mat faceMat = Mat(image, roi);
            extracted_faces.push_back(faceMat);
        }
    }
    return extracted_faces;
}

Mat Detector::GetLargest(std::vector<Mat> images) const
{
    Mat largest;
    if (images.size() > 0)
    {
        int largestSize = images[0].size[0] * images[0].size[1];
        largest = images[0];
        for (auto img : images)
        {
            int size = img.size[0] * img.size[1];
            if (size > largestSize)
            {
                largestSize = size;
                largest = img;
            }
        }
    }
    return largest;
}

Mat Detector::ExtractLargest(Mat image)
{
    auto extracted_faces = Extract(image);
    auto largest = GetLargest(extracted_faces);
    return largest;
}