#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace engine {
    namespace controls {
        void registerAction(std::string action, SDL_Keycode key_code);
        void update(SDL_Event &event);
        bool isActionJustPressed(const char *);
        void clearFrameStates();
    }
}
