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
        components.transform.translate = {-100, 64};
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
        components.transform.translate = {0, -200};
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

class Score : public engine::Object {
public:
    engine::component::Sprite *s1, *s2;

    Score()
        : engine::Object("Score")
    {
        components.transform.translate.y = 222;
        components.addComponent(
            engine::component::Sprite(
                24, 36,
                {
                    "assets/sprites/0.png",
                    "assets/sprites/1.png",
                    "assets/sprites/2.png",
                    "assets/sprites/3.png",
                    "assets/sprites/4.png",
                    "assets/sprites/5.png",
                    "assets/sprites/6.png",
                    "assets/sprites/7.png",
                    "assets/sprites/8.png",
                    "assets/sprites/9.png",
                }
            )
        );
        components.addComponent(
            engine::component::Sprite(
                24, 36,
                {
                    "assets/sprites/0.png",
                    "assets/sprites/1.png",
                    "assets/sprites/2.png",
                    "assets/sprites/3.png",
                    "assets/sprites/4.png",
                    "assets/sprites/5.png",
                    "assets/sprites/6.png",
                    "assets/sprites/7.png",
                    "assets/sprites/8.png",
                    "assets/sprites/9.png",
                }
            )
        );

        auto _s = components.get<engine::component::Sprite>();
        const auto sprites = std::vector(_s.begin(), _s.end());
        s1 = &(sprites[0].get());
        s2 = &(sprites[1].get());
        s1->transform.translate.x = -11;
        s2->transform.translate.x =  11;
    }

    uint8_t score = 23;
    void update(float) override {
        if (score < 99) {
            s1->current_texture = static_cast<uint32_t>(score / 10);
            s2->current_texture = static_cast<uint32_t>(score % 10);
        }
    }
};

class MainScene : public engine::Scene {
public:
    Background bg;
    Bird bird;
    Ground g1;
    Score score;

    MainScene() {
        objects.push_back(&bg);
        objects.push_back(&score);
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
