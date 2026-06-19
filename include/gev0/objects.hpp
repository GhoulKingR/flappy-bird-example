#pragma once

#include "components.hpp"
#include <functional>
#include <memory>
#include <string>
#include <cstdint>

namespace engine
{
    struct Object
    {
        std::vector<std::unique_ptr<engine::component::IComponent>> _components;
        std::string                                                 name;
        component::Transform                                        transform;
        std::function<void(float)>                                  update = nullptr;
        Object(const char *name = nullptr);

        template<component::TComponent Component, typename... Args>
        Component &newComponent(Args&&... args)
        {
            auto _obj = std::make_unique<Component>(args...);
            auto &ref = *_obj.get();
            auto &obj = _components.emplace_back(std::move(_obj));
            return ref;
        }

    private:
        static inline uint32_t objectCount = 0;
    };

    namespace object
    {
        void _draw(Object *)        noexcept;
#ifdef NDEBUG
        void _inspector(Object *)   noexcept;
#endif
    }
}
