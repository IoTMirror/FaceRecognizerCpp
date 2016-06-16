#include "Recognizer.h"
#include <fstream>

using namespace std;
using namespace cv;

Recognizer::Recognizer(string data_directory, double threshold) :
    data_directory(data_directory),
    threshold(threshold)
{
}

Ptr<face::LBPHFaceRecognizer> Recognizer::LoadRecognizer(int uid)
{
    Ptr<face::LBPHFaceRecognizer> recognizer = face::createLBPHFaceRecognizer(1, 8, 8, 8, threshold);
    string filename = data_directory + "/" + to_string(uid) + ".lbph";
    if (std::ifstream(filename))
    {
        recognizer->load(filename);
    }
    return recognizer;
}

int Recognizer::Recognize(Mat image, vector<int> users_to_check)
{
    int bestUID = -1;
    double bestDistance = numeric_limits<double>::max();

    for (int uid : users_to_check)
    {
        auto recognizer = LoadRecognizer(uid);
        int label;
        double distance;
        recognizer->predict(image, label, distance);
        if (label != -1 && distance < bestDistance)
        {
            bestUID = label;
            bestDistance = distance;
        }
        recognizer.release();
    }
    return bestUID;
}

void Recognizer::Train(int userId, Mat image)
{
    auto recognizer = LoadRecognizer(userId);
    vector<Mat> images;
    images.push_back(image);
    vector<int> labels;
    labels.push_back(userId);
    recognizer->update(images, labels);
    recognizer->save(data_directory + "/" + to_string(userId) + ".lbph");
    recognizer.release();
}

