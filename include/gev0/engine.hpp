#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace engine {
    void init(const char *title, uint32_t w, uint32_t h);
    void start();
    void cleanup();
    glm::mat4 aspectRatio();
#ifdef NDEBUG
    bool drawCollisionShapes();
#endif
    void bindQuad();
    uint32_t indexCount();
}
