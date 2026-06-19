#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "SDL3/SDL_audio.h"
#include "common.hpp"
#include "textures.hpp"

namespace engine
{
    struct Object;

    namespace component
    {
        // Transform is not a component. Every object and component has a
        // transform field in them.
        struct Transform
        {
            vec2<float> scale{1, 1};
            vec2<float> translate{0, 0};
            float       rotate = 0;
            glm::mat4   model() const noexcept;
            Transform() = default;
            Transform(vec2<float> scale, vec2<float> translate, float rotate);

#ifdef NDEBUG
            void        inspector(const char *prefix = nullptr) noexcept;
#endif
        };

        // Interface for components
        struct IComponent
        {
            IComponent()                                = default;
            virtual ~IComponent()                       = default;
            IComponent(const IComponent&)               = delete;
            IComponent &operator=(const IComponent&)    = delete;
            IComponent(IComponent&&)                    = delete;
            IComponent &operator=(IComponent&&)         = delete;
            virtual void draw(const glm::mat4&) noexcept {}
#ifdef NDEBUG
            virtual void inspector(uint32_t) noexcept {}
#endif
        };

        template<typename T>
        concept TComponent = std::is_base_of_v<IComponent, T> &&
                            !std::is_same_v<IComponent, T>;

        struct Sound : public IComponent
        {
            void draw(const glm::mat4&) noexcept override;
            void addSound(const char *name, const char *src, bool _looping=false);
            void play(const char *);
            void stop(const char *);
            ~Sound();

        private:
            struct Data
            {
                SDL_AudioSpec       _spec;
                bool                _looping;
                uint32_t            _length;
                SDL_AudioStream*    _stream;
                uint8_t*            _buffer;
            };
            std::unordered_map<std::string, Data> _sounds;
        };

        // Sprite component. Allows you to display sprites on the screen
        struct Sprite : public IComponent
        {
            bool        hidden = false;
            Transform   transform;
            uint32_t    current_texture = 0;
            void draw(const glm::mat4 &) noexcept override;
            Sprite(int w, int h, std::vector<Texture *>);

        private:
            vec2<int>               size{0, 0};
            static inline uint32_t  objCount = 0;
            std::vector<Texture *>  textures;

#ifdef NDEBUG
        public:
            void inspector(uint32_t) noexcept override;
#endif
        };

        // Timer component: Allows you to set timers with callbacks
        struct Timer : public IComponent
        {
            void setTimeout(std::function<void()>, uint32_t duration_ms, uint32_t times = 1) noexcept;
            void draw(const glm::mat4 &) noexcept override; // hijack `draw` function for event polling
        private:
            std::chrono::time_point<std::chrono::system_clock>  target;
            uint32_t                                            count = 0,
                                                                _duration = 0;
            std::function<void()>                               lambda = nullptr;
        };

        namespace collision
        {
            // Collision shape interface
            struct ICollisionShape
            {
                ICollisionShape()                                   = default;
                virtual ~ICollisionShape()                          = default;
                ICollisionShape(const ICollisionShape&)             = delete;
                ICollisionShape &operator=(const ICollisionShape&)  = delete;
                ICollisionShape(ICollisionShape&&)                  = delete;
                ICollisionShape &operator=(ICollisionShape&&)       = delete;

                virtual ICollisionShape* checkCollision() const noexcept = 0;
#ifdef NDEBUG
                virtual void draw(const glm::mat4 &) const noexcept {}
                virtual void inspector() noexcept {}
#endif
            };

            // Box collision shape
            struct Box : public ICollisionShape
            {
                Box(Object *);
                Transform        transform;
                Object*          parent;
                vec2<float>      size {0.0f, 0.0f};
                ICollisionShape* checkCollision() const noexcept override;
#ifdef NDEBUG
                ~Box();
                void draw(const glm::mat4 &) const noexcept override;
                void inspector() noexcept override;
#endif

                // This is required even when it's not needed because it can cause
                // runtime overflow errors. I still don't understand how this was an
                // issue, but it took me days to figure out.
                //
                // Note from the future: this happens because NDEBUG isn't used in the
                // game development code. So the size of the struct on the game side
                // and on the engine becomes different.
                // TODO: document this ^
            private:
                uint32_t id;
                static inline uint32_t counter = 0;
            };

        }

        template<typename T>
        concept TCollisionShape = std::is_base_of_v<collision::ICollisionShape, T> &&
                                 !std::is_same_v<collision::ICollisionShape, T>;

        // Physics component: Handles both physics related data, and
        // collision detection
        struct Physics : public IComponent
        {
            bool hidden = false;
            float gravity = 9.8;

            // Add a new collision shape to the physics component
            template<TCollisionShape Shape, typename... Args>
            Shape &newCollisionShape(Args&&... args)
            {
                auto _obj = std::make_unique<Shape>(args...);
                auto &ref = *_obj.get();
                auto &obj = collisionShapes.emplace_back(std::move(_obj));
                return ref;
            }

#ifdef NDEBUG
            void draw(const glm::mat4 &) noexcept override;
            void inspector(uint32_t) noexcept override;
#endif
        private:
            std::vector<std::unique_ptr<collision::ICollisionShape>> collisionShapes;
        };
    }

}
// I want to eventually separate the draw and inspector methods away from their components
// into a sort-of dedicated system. So it'll be closer to an entity component system.
// Not sure if I should do that or not, because it's essentially the same thing as what's here.
// I don't know really, so:
//
// TODO: Find out if it's worth it to move the draw and inspector methods to dedicated system
// and implement that if it is.
