#include "components.hpp"
#include "controls.hpp"
#include <array>
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
        components.addComponent(
            engine::component::Transform(
                {1, 1}, {-100, 64}, 0
            )
        );
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

        const auto gravity = components.physics()->gravity;
        velocity.y += -gravity * GRAVITY_SCALE * deltaTime;
        components.transform()->translate += velocity * deltaTime;
    }
};

class Background : public engine::Object {
public:
    Background()
        : engine::Object("Background")
    {
        components.addComponent(
            engine::component::Transform(
                    {1, 1}, {0, 0}, 0
            )
        );
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
    Ground(const char *name)
        : engine::Object(name)
    {
        components.addComponent(
            engine::component::Transform(
                {1, 1}, {0, -200}, 0
            )
        );
        components.addComponent(
            engine::component::Sprite(
                336, 112,
                {
                    "assets/sprites/base.png"
                }
            )
        );
    }

    void update(float delta) override {
        auto left = engine::vec2(-1.0f, 0.0f);
        auto velocity = left * FLOOR_SPEED * delta;
        components.transform()->translate += velocity;
    }
};

class MainScene : public engine::Scene {
public:
    Background bg;
    Bird bird;
    Ground g1{"Ground 1"};
    Ground g2{"Ground 2"};
    Ground g3{"Ground 3"};

    MainScene() {
        objects.push_back(&bg);
        objects.push_back(&bird);
        objects.push_back(&g1);
        objects.push_back(&g2);
        objects.push_back(&g3);

        g2.getComponents().transform()->translate += engine::vec2{336.0f, 0.0f};
        g3.getComponents().transform()->translate += (
            engine::vec2{ 336.0f * 2.0f, 0.0f });
    }

    void update(float) override {
        auto transforms = std::array<
            std::reference_wrapper<engine::component::Transform>, 3>
        {
            g1.getComponents().transform().value(),
            g2.getComponents().transform().value(),
            g3.getComponents().transform().value(),
        };

        for (auto &tr : transforms) {
            if (tr.get().translate.x <= -336.0f) {
                tr.get().translate += engine::vec2{ 336.0f * 2.f, 0.0f };
            }
        }
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
