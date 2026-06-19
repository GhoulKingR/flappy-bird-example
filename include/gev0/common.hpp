#pragma once

#include <cstddef>
#include <type_traits>

namespace engine
{
    template <typename T>
    concept ScalarType = std::is_scalar_v<T>;

    template <ScalarType T>
    struct vec2
    {
        union {
            struct { T x, y; };
            T _internal[2];
        };

        constexpr const T* data() const noexcept { return _internal; }
        constexpr T* data() noexcept { return _internal; }
        constexpr vec2(T x = 0, T y = 0) : x(x), y(y) {}

        constexpr vec2 operator*(T val) const noexcept
        { return vec2(x * val, y * val); }

        constexpr vec2 operator-(const vec2& val) const noexcept
        { return vec2(x - val.x, y - val.y); }

        constexpr vec2 operator+(const vec2& val) const noexcept
        { return vec2(x + val.x, y + val.y); }

        constexpr vec2& operator-=(const vec2& val) noexcept
        {
            x -= val.x;
            y -= val.y;
            return *this;
        }

        constexpr vec2& operator+=(const vec2& val) noexcept
        {
            x += val.x;
            y += val.y;
            return *this;
        }
    };
}
