#include "components.hpp"
#include "controls.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <objects.hpp>
#include <scene.hpp>
#include <engine.hpp>
#include <cstdlib>
#include <utility>

class Bird : public engine::Object {
public:
    engine::component::Physics *physics;
    engine::component::Sprite *sprite;
    engine::component::Timer *timer;

    Bird() : engine::Object("Bird")
    {
        transform.translate = {-100, 64};

        auto _s = std::make_unique<engine::component::Sprite>(
            32, 24,
            std::initializer_list<std::filesystem::path>{
                "assets/sprites/redbird-upflap.png",
                "assets/sprites/redbird-midflap.png",
                "assets/sprites/redbird-downflap.png"
            }
        );
        sprite = _s.get();
        components.emplace_back(std::move(_s));

        auto _p = std::make_unique<engine::component::Physics>();
        physics = _p.get();
        components.push_back(std::move(_p));

        auto _t = std::make_unique<engine::component::Timer>();
        timer = _t.get();
        components.push_back(std::move(_t));
    }

    const float GRAVITY_SCALE = 40.0f;
    engine::vec2<float> velocity {0, 0};
    void update(float deltaTime) override {
        if (engine::controls::isActionJustPressed("fly")) {
            velocity.y = 250.0f;
            sprite->current_texture = 2;

            timer->setTimeout([this](){
                sprite->current_texture--;
            }, 500, 2);
        }

        const auto gravity = physics->gravity;
        velocity.y += -gravity * GRAVITY_SCALE * deltaTime;
        transform.translate += velocity * deltaTime;
    }
};

class Background : public engine::Object {
public:
    Background() : engine::Object("Background")
    {
        components.push_back(std::make_unique<engine::component::Sprite>(
            288, 512,
            std::initializer_list<std::filesystem::path>{
                "assets/sprites/background-night.png",
                "assets/sprites/background-day.png"
            }
        ));
    }
};

class Ground : public engine::Object {
    const float FLOOR_SPEED = 150.0f;
    std::array<engine::component::Sprite*, 2> sprites;

public:
    Ground() : engine::Object("Ground")
    {
        transform.translate = {0, -200};

        for (int i = 0; i < 2; i++) {
            auto _s = std::make_unique<engine::component::Sprite>(
                336, 112,
                std::initializer_list<std::filesystem::path>{"assets/sprites/base.png"}
            );
            _s->transform.translate.x = i * 336;
            sprites[i] = _s.get();
            components.push_back(std::move(_s));
        }
    }

    void update(float delta) override {
        for (auto s : sprites) {
            auto left = engine::vec2(-1.0f, 0.0f);
            auto velocity = left * FLOOR_SPEED * delta;
            s->transform.translate += velocity;

            if (s->transform.translate.x <= -336.0f) {
                s->transform.translate +=
                    engine::vec2{ 336.0f * 2.f, 0.0f };
            }
        }
    }
};

class Score : public engine::Object {
public:
    std::array<engine::component::Sprite *, 2> sprites;

    Score() : engine::Object("Score")
    {
        transform.translate.y = 222;
        for (int i = 0; i < 2; i++) {
            auto _s = std::make_unique<engine::component::Sprite>(
                24, 36,
                std::initializer_list<std::filesystem::path>{
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
            );
            sprites[i] = _s.get();
            components.push_back(std::move(_s));
        }

        sprites[0]->transform.translate.x = -11;
        sprites[1]->transform.translate.x =  11;
        // sprites[0]->hidden = true;
    }

    uint8_t score = 23;
    void update(float) override {
        if (score < 99) {
            sprites[0]->current_texture = static_cast<uint32_t>(score / 10);
            sprites[1]->current_texture = static_cast<uint32_t>(score % 10);
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
