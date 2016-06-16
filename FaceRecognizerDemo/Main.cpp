#include "SDL/SDL.h"
#include "Detector.h"
#undef main

#include "App.h"

int main()
{
    App app = App();

    while (app.running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            app.HandleInput(event);
        }
        app.Tick();
        app.Render();
    }

    return 0;
}