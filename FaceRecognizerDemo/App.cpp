#include "App.h"
#include "dearImgui/imgui_impl_sdl_gl3.h"
#include "dearImgui/imgui.h"
#include <ctime>
#include "Preprocessor.h"
#include <fstream>

using namespace std;

// TODO: Read this from a config file
string detector_config_file = "C:/Projects/Cpp/FaceRecognizer/Release/haarcascade_frontalface_default.xml";
string recognizer_data_directory = "C:/Projects/Cpp/FaceRecognizer/Release/data";
string user_file = "C:/Projects/Cpp/FaceRecognizer/Release/data/userlist.dat";

App::App() :
    running(true),
    capture(CV_CAP_ANY),
    detector(detector_config_file),
    recognizer(recognizer_data_directory, 50)
{
    {
        ifstream userfile(user_file);
        string line;
        while (getline(userfile, line))
        {
            users.push_back(line);
        }
    }
    detector.min_size = cv::Size(200, 200);
    clear_color = ImColor(114, 144, 154);

    // Setup SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    window = SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    gl_context = SDL_GL_CreateContext(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);
    // Generate a number for our textureID's unique handle
    glGenTextures(1, &cameraFrameTexture);

    // Bind to our texture handle
    glBindTexture(GL_TEXTURE_2D, cameraFrameTexture);

    // Set texture interpolation methods for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    tex = new cv::ogl::Texture2D();
}

void App::HandleInput(SDL_Event& event)
{
    ImGui_ImplSdlGL3_ProcessEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
    {
        running = false;
    } break;
    case SDL_KEYDOWN:
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
        {
            running = false;
        } break;
        case SDLK_SPACE:
        {
            debug_window_open ^= 1;
        } break;
        }
    } break;
    }
}

void App::GetCameraFrame()
{
    capture.grab();
    capture.retrieve(cameraFrame);
    displayFrame = cameraFrame;
    displayMonochrome = false;
}

cv::CascadeClassifier face_cascade;
vector<cv::Rect> detectAndDisplay(cv::Mat frame)
{
    face_cascade.load(detector_config_file);
    vector<cv::Rect> faces;
    cv::Mat frame_gray;
    cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    //-- Detect faces
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0, cv::Size(30, 30));
    return faces;
}

cv::Mat displayedDebug;

void App::GetLoginFrame()
{
    switch (logging_in_state)
    {
    case LoggingInState::None: break;
    case LoggingInState::Starting:
        GetCameraFrame();
        process_time = time(0);
        logging_in_state = LoggingInState::PhotoTaken;
        debug_message = "login photo taken";
    case LoggingInState::PhotoTaken:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);

            auto found = detector.ExtractLargest(cameraFrame);
            if (found.size[0] > 0)
            {
                displayFrame = found.clone();
                displayedDebug = found;
            }
            else
            {
                logging_in_state = LoggingInState::Starting;
            }

            logging_in_state = LoggingInState::FaceExtracted;
        }
        break;
    case LoggingInState::FaceExtracted:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);
            displayFrame = Preprocess(displayFrame, 200, 200);
            logging_in_state = LoggingInState::Preprocessed;
            debug_message = "login photo face extracted";
        }
        break;
    case LoggingInState::Preprocessed:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);
            vector<int> user_ids;
            for(int i = 0; i < users.size(); i++)
            {
                user_ids.push_back(i);
            }
            recognized_user = recognizer.Recognize(displayFrame, user_ids);
            logging_in_state = LoggingInState::None;
        }
        break;
    default: break;
    }
}

void App::GetTeachingFrame()
{
    switch (teaching_state)
    {
    case TeachingState::None: break;
    case TeachingState::Starting:
        process_time = time(0);
        teaching_state = TeachingState::UserCreated;
        debug_message = to_string(teaching_photos_taken) + "/4: teaching photo taken";
    case TeachingState::UserCreated:
        GetCameraFrame();
        teaching_state = TeachingState::PhotoTaken;
        break;
    case TeachingState::PhotoTaken:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);
            auto found = detector.ExtractLargest(cameraFrame);
            if (found.size[0] > 0)
            {
                displayFrame = found.clone();
                displayedDebug = found;
            }
            else
            {
                logging_in_state = LoggingInState::Starting;
            }
            debug_message = to_string(teaching_photos_taken) + "/4 teaching face extracted";
            teaching_state = TeachingState::FaceExtracted;
        }
        break;
    case TeachingState::FaceExtracted:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);
            displayFrame = Preprocess(displayFrame, 200, 200);
            teaching_state = TeachingState::Preprocessed;
            debug_message = to_string(teaching_photos_taken) + "/4 teaching photo preprocessed";
        }
        break;
    case TeachingState::Preprocessed:
        if (difftime(time(0), process_time) > process_delay)
        {
            process_time = time(0);
            recognizer.Train(teaching_user_id, displayFrame);
            if (teaching_photos_taken >= 4)
            {
                teaching_state = TeachingState::None;
                teaching_photos_taken = 0;
            }
            else
            {
                teaching_state = TeachingState::UserCreated;
                teaching_photos_taken++;
            }
        }
        break;
    default: break;
    }
}

void App::Tick()
{
    if (logging_in_state != LoggingInState::None)
    {
        GetLoginFrame();
    }
    else if (teaching_state != TeachingState::None)
    {
        GetTeachingFrame();
    }
    else
    {
        GetCameraFrame();
    }
    tex->copyFrom(displayFrame);
    //pushMat(cameraFrameTexture, displayFrame, displayMonochrome);
    ImGui_ImplSdlGL3_NewFrame(window);
    auto frame_size = ImVec2(cameraFrame.size[1], cameraFrame.size[0]);
    auto image_size = ImVec2(displayFrame.size[1], displayFrame.size[0]);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
    ImGui::SetNextWindowSize(frame_size);
    ImGui::Begin("Capture", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::Image(reinterpret_cast<void*>(tex->texId()), image_size);
    ImGui::End();

    {
        ImGui::SetNextWindowSize(ImVec2(frame_size.x, 140));
        ImGui::SetNextWindowPos(ImVec2(0, frame_size.y + 20));
        ImGui::Begin("Controls", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button("Login"))
        {
            logging_in_state = LoggingInState::Starting;
            debug_message = "Started Login";
        }
        static char username[128];
        ImGui::InputText("Username", username, 128);
        if (ImGui::Button("Register"))
        {
            teaching_state = TeachingState::Starting;
            string uname = string(username);
            if (uname.length() != 0)
            {
                teaching_user_id = users.size();
                users.push_back(uname);
                debug_message = "Started Teaching";
                ofstream userfilestream(user_file);
                for(auto name : users)
                {
                    userfilestream << name << endl;
                }
            }
        }
        ImGui::End();
    }

    if (debug_window_open)
    {
        ImGui::SetNextWindowSize(ImVec2(330, 180));
        ImGui::SetNextWindowPos(ImVec2(950, 0), ImGuiSetCond_Always);
        ImGui::Begin("Debug Info", &debug_window_open);
        ImGui::SliderFloat("Delay", &process_delay, 0, 5);
        ImGui::Text(debug_message.c_str());
        if (recognized_user != -1 && users.size() != 0)
        {
            ImGui::Text(("Recognized user: " + users[recognized_user]).c_str());
        }
        static float f = 0.0f;
        if (ImGui::Button("Test Window")) test_window_open ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    if (test_window_open)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&test_window_open);
    }
}

void App::Render() const
{
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    SDL_GL_SwapWindow(window);
}

App::~App()
{
    capture.release();
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
