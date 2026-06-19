#pragma once

#include <cstdint>

namespace engine
{
    struct Texture
    {
        uint32_t id = 0;
        const char *path = nullptr;
        Texture(const char *) noexcept;
        ~Texture();
        Texture(const Texture &) = delete;
        Texture operator=(const Texture &) = delete;
        Texture(Texture &&) = delete;
        Texture operator=(Texture &&) = delete;
    };
}
