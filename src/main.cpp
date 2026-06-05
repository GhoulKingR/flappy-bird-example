#include "components.hpp"
#include "controls.hpp"
#include <cstdint>
#include <functional>
#include <objects.hpp>
#include <scene.hpp>
#include <engine.hpp>
#include <cstdlib>

class Bird : public engine::Object {
public:
    Bird()
        : engine::Object("Bird")
    {
        components.transform = {
                {1, 1},
                {-100, 64},
                0
            };

        components.addComponent(
            engine::component::Sprite(
                32, 24,
                {
                    "assets/sprites/redbird-upflap.png",
                    "assets/sprites/redbird-midflap.png",
                    "assets/sprites/redbird-downflap.png"
                }
            )
        );
        components.addComponent(
            engine::component::Physics()
        );
    }

    const float GRAVITY_SCALE = 40.0f;
    engine::vec2<float> velocity {0, 0};
    void update(float deltaTime) override {
        if (engine::controls::isActionJustPressed("fly")) {
            velocity.y = 250.0f;
        }

        auto physics = components.get<engine::component::Physics>().front().get();
        const auto gravity = physics.gravity;
        velocity.y += -gravity * GRAVITY_SCALE * deltaTime;
        components.transform.translate += velocity * deltaTime;
    }
};

class Background : public engine::Object {
public:
    Background()
        : engine::Object("Background")
    {
        components.transform ={
                {1, 1},
                {0, 0},
                0
            };
        components.addComponent(
            engine::component::Sprite(
                288, 512,
                {
                    "assets/sprites/background-night.png",
                    "assets/sprites/background-day.png"
                }
            )
        );
    }
};

class Ground : public engine::Object {
    const float FLOOR_SPEED = 150.0f;

public:
    Ground()
        : engine::Object("Ground")
    {
        components.transform = {
                {1, 1},
                {0, -200},
                0
            };
        components.addComponent(
            engine::component::Sprite(
                336, 112,
                {"assets/sprites/base.png"}
            )
        );
        components.addComponent(
            engine::component::Sprite(
                336, 112,
                {"assets/sprites/base.png"}
            )
        );
        components.addComponent(
            engine::component::Sprite(
                336, 112,
                {"assets/sprites/base.png"}
            )
        );

        uint32_t push_x = 0;
        for (auto sprite : components.get<engine::component::Sprite>()) {
            sprite.get().transform.translate.x += push_x;
            push_x += 336;
        }
    }

    void update(float delta) override {
        for (auto sprite : components.get<engine::component::Sprite>()) {
            auto &s = sprite.get();
            auto left = engine::vec2(-1.0f, 0.0f);
            auto velocity = left * FLOOR_SPEED * delta;
            s.transform.translate += velocity;

            if (s.transform.translate.x <= -336.0f) {
                s.transform.translate +=
                    engine::vec2{ 336.0f * 2.f, 0.0f };
            }
        }
    }
};

class MainScene : public engine::Scene {
public:
    Background bg;
    Bird bird;
    Ground g1;

    MainScene() {
        objects.push_back(&bg);
        objects.push_back(&bird);
        objects.push_back(&g1);
    }
};

int main() {
    engine::init("Flappy bird", 288, 512);
    engine::controls::registerAction("fly", SDLK_SPACE);

    MainScene scn;
    engine::loadScene(&scn);

    engine::start();
    engine::cleanup();
    return EXIT_SUCCESS;
}
