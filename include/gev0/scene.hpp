#pragma once

#include "objects.hpp"
#include <functional>
#include <vector>

namespace engine {
    // Game scene (collection of objects for rendering)
    struct Scene {
        std::vector<Object *>       objects;
        std::function<void(float)>  update = nullptr;
    };

    // helper functions for managing the scene data
    // The only ones needed to be used is load and unload.
    namespace scene {
        // load a scene object to view
        void load(Scene *);
        // unload the current scene object to view
        void unload();

#ifdef NDEBUG
        void _inspector();
        void _loop(float, bool = false);
#else
        void _loop(float);
#endif
    }
}
