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
#include <string>
#include <vector>

class Bird : public engine::Object {
    engine::component::Physics physics;
    engine::component::Timer timer;
    engine::component::Sprite sprite {
        32, 24,
        {
            "assets/sprites/redbird-upflap.png",
            "assets/sprites/redbird-midflap.png",
            "assets/sprites/redbird-downflap.png"
        }
    };

public:
    Bird() : engine::Object("Bird")
    {
        transform.translate = {-100, 64};
        components.push_back(&sprite);
        components.push_back(&physics);
        components.push_back(&timer);
    }

    constexpr static float GRAVITY_SCALE = 40.0f;
    engine::vec2<float> velocity {0, 0};
    void update(float deltaTime) override {
        if (engine::controls::isActionJustPressed("fly")) {
            velocity.y = 250.0f;
            sprite.current_texture = 2;

            timer.setTimeout([this](){
                sprite.current_texture--;
            }, 500, 2);
        }

        const auto gravity = physics.gravity;
        velocity.y += -gravity * GRAVITY_SCALE * deltaTime;
        transform.translate += velocity * deltaTime;
    }
};

class Background : public engine::Object {
    engine::component::Sprite sprite{
        288, 512,
        std::vector<std::filesystem::path>{
            "assets/sprites/background-night.png",
            "assets/sprites/background-day.png"
        }
    };

public:
    Background() : engine::Object("Background") {
        components.push_back(&sprite);
    }
};

constexpr float FLOOR_SPEED = 150.0f;

class Ground : public engine::Object {
    constexpr static int SPRITE_SIZE = 2;
    std::array<std::unique_ptr<engine::component::Sprite>, SPRITE_SIZE> sprites;

public:
    Ground() : engine::Object("Ground")
    {
        transform.translate = {0, -200};
        for (int i = 0; i < SPRITE_SIZE; i++) {
            sprites[i] = std::make_unique<engine::component::Sprite>(
                336, 112,
                std::vector<std::filesystem::path>{
                    "assets/sprites/base.png"
                }
            );
            sprites[i]->transform.translate.x = i * 336;
            components.push_back(sprites[i].get());
        }
    }

    void update(float delta) override {
        for (auto &s : sprites) {
            const auto left = engine::vec2(-1.0f, 0.0f);
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
    std::array<std::unique_ptr<engine::component::Sprite>, 2> sprites;

    Score() : engine::Object("Score")
    {
        transform.translate.y = 222;
        for (int i = 0; i < 2; i++) {
            sprites[i] = std::make_unique<engine::component::Sprite>(
                24, 36,
                std::vector<std::filesystem::path>{
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
            components.push_back(sprites[i].get());
        }

        sprites[0]->transform.translate.x = -11;
        sprites[1]->transform.translate.x =  11;
    }

    uint8_t score = 23;
    void update(float) override {
        if (score < 99) {
            sprites[0]->current_texture = static_cast<uint32_t>(score / 10);
            sprites[1]->current_texture = static_cast<uint32_t>(score % 10);
        }
    }
};

class Pipes : public engine::Object {
    static constexpr int PIPE_COUNT = 4;
    static constexpr float SPACING = 175;
    std::array<std::unique_ptr<engine::component::Sprite>, PIPE_COUNT> pipes;

public:
    Pipes()
    : engine::Object("Pipes")
    {
        for (int i = 0; i < PIPE_COUNT; i++) {
            pipes[i] = std::make_unique<engine::component::Sprite>(
                52, 320,
                std::vector<std::filesystem::path>{
                    "assets/sprites/pipe-red.png"
                }
            );
            pipes[i]->transform.rotate = i % 2 ? 180.0f : 0.f;
            pipes[i]->transform.translate.y = i % 2 ? 256.0f : -256.0f;
            pipes[i]->transform.translate.x = static_cast<int>(i / 2) * SPACING - 100.0f;
            components.push_back(pipes[i].get());
        }
    }

    void update(float delta) override {
        for (auto &p : pipes) {
            const auto left = engine::vec2(-1.0f, 0.0f);
            auto velocity = left * FLOOR_SPEED * delta;
            p->transform.translate += velocity;

            if (p->transform.translate.x <= -170.0f) {
                p->transform.translate.x +=
                    170.0f * (PIPE_COUNT / 2.f);
            }
        }
    }
};

class MainScene : public engine::Scene {
public:
    Background bg;
    Bird bird;
    Ground g1;
    Score score;
    Pipes pipes;

    MainScene() {
        objects.push_back(&bg);
        objects.push_back(&bird);
        objects.push_back(&pipes);
        objects.push_back(&score);
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
