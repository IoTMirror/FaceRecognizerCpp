#pragma once
#include <SDL/SDL.h>
#include "opencv2/opencv.hpp"
#include "GL/gl3w.h"
#include "dearImgui/imgui.h"
#include "opencv2/core/opengl.hpp"
#include "Recognizer.h"

#include "Detector.h"

namespace LoggingInState
{
    enum AppState
    {
        None,
        Starting,
        PhotoTaken,
        FaceExtracted,
        Preprocessed
    };
}

namespace TeachingState
{
    enum AppState
    {
        None,
        Starting,
        UserCreated,
        PhotoTaken,
        FaceExtracted,
        Preprocessed
    };
}

struct App
{
    bool running = true;

    App();
    void HandleInput(SDL_Event& event);
    void Tick();
    void Render() const;
    ~App();

private:
    SDL_Window* window;
    SDL_GLContext gl_context;

    GLuint cameraFrameTexture;
    cv::Mat cameraFrame;
    cv::Mat displayFrame;
    bool displayMonochrome;
    cv::VideoCapture capture;
    cv::ogl::Texture2D* tex;

    int teaching_photos_taken = 0;
    int teaching_user_id = 1;

    std::vector<std::string> users;
    int recognized_user = 0;

    

    Detector detector;
    Recognizer recognizer;

    bool debug_window_open = false;
    bool test_window_open = false;
    ImVec4 clear_color;
    std::string debug_message;

    time_t process_time;
    float process_delay = 2;

    LoggingInState::AppState logging_in_state = LoggingInState::None;
    TeachingState::AppState teaching_state = TeachingState::None;

    void GetCameraFrame();
    void GetLoginFrame();
    void GetTeachingFrame();
};
