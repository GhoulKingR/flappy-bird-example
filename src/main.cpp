#include "components.hpp"
#include "controls.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <objects.hpp>
#include <ranges>
#include <scene.hpp>
#include <engine.hpp>
#include <cstdlib>
#include <string>
#include <variant>
#include <vector>

constexpr static float  FLOOR_SPEED = 150.0f;

class Bird : public engine::Object
{
    engine::component::Timer            timer;
    engine::component::Sprite           sprite{ 32, 24, {
                                                "assets/sprites/redbird-upflap.png",
                                                "assets/sprites/redbird-midflap.png",
                                                "assets/sprites/redbird-downflap.png"}};
    constexpr static float              GRAVITY_SCALE = 40.0f;
    engine::vec2<float>                 velocity{0, 0};
    engine::component::Physics          physics;
    engine::component::collision::Box   hitBox {this};

public:
    Bird() : engine::Object("Bird")
    {
        transform.translate = {-100, 64};
        components.push_back(&sprite);
        components.push_back(&timer);

        hitBox.size                 = {32.0f, 24.0f};
        physics.collisionShapes.push_back(&hitBox);
        components.push_back(&physics);
    }

    void update(float deltaTime) override
    {
        if (engine::controls::isActionJustPressed("fly"))
        {
            velocity.y = 250.0f;
            sprite.current_texture = 2;
            timer.setTimeout([this](){ sprite.current_texture--; }, 500, 2);
        }

        auto coll = hitBox.checkCollision();
        std::visit([this, deltaTime](auto &_c){
            if (_c == nullptr)
            {
                const auto gravity   = physics.gravity;
                velocity.y          += -gravity * GRAVITY_SCALE * deltaTime;
            }
            else
                velocity.y = std::max(0.0f, velocity.y);
        }, coll);
        transform.translate += velocity * deltaTime;
    }
};

class Background : public engine::Object
{
    engine::component::Sprite sprite {288, 512, {"assets/sprites/background-night.png", "assets/sprites/background-day.png"}};
public:
    Background() : engine::Object("Background")
    { components.push_back(&sprite); }
};

class Ground : public engine::Object
{
    constexpr static int    SPRITE_SIZE = 2;
    std::vector<engine::component::Sprite>  sprites;
    engine::component::Physics              physics;
    engine::component::collision::Box       hitBox{this};

public:
    Ground() : engine::Object("Ground")
    {
        transform.translate = {0, -200};
        sprites.reserve(SPRITE_SIZE);
        for (int i = 0; i < SPRITE_SIZE; i++)
        {
            auto &ref                   = sprites.emplace_back(336, 112, std::vector<std::filesystem::path>{"assets/sprites/base.png"});
            ref.transform.translate.x   = i * 336;
            components.push_back(&ref);
        }
        hitBox.size = engine::vec2{336.0f, 112.0f};
        physics.collisionShapes.push_back(&hitBox);
        components.push_back(&physics);
    }

    void update(float delta) override
    {
        for (auto &s : sprites)
        {
            const auto left          = engine::vec2(-1.0f, 0.0f);
            auto velocity            = left * FLOOR_SPEED * delta;
            s.transform.translate   += velocity;
            if (s.transform.translate.x <= -336.0f)
                s.transform.translate.x += 336.0f * 2.f;
        }
    }
};

class Score : public engine::Object
{
    static constexpr int DIGITS = 2;
    std::vector<engine::component::Sprite> sprites;
    uint8_t score = 23;

public:
    Score() : engine::Object("Score")
    {
        transform.translate.y = 222;
        sprites.reserve(DIGITS);
        for (int i = 0; i < DIGITS; i++)
        {
            auto &ref = sprites.emplace_back(
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
                });
            components.push_back(&ref);
        }
        sprites[0].transform.translate.x = -11;
        sprites[1].transform.translate.x = 11;
    }

    void update(float) override
    {
        if (score < 99)
        {
            sprites[0].current_texture = static_cast<uint32_t>(score / 10);
            sprites[1].current_texture = static_cast<uint32_t>(score % 10);
        }
    }
};

class Pipes : public engine::Object
{
    static constexpr int PIPE_COUNT = 4;
    static constexpr float SPACING = 175;
    engine::component::Physics physics;
    std::vector<engine::component::Sprite> pipes;
    std::vector<engine::component::collision::Box> collisionBoxes;

public:
    Pipes() : engine::Object("Pipes")
    {
        pipes.reserve(PIPE_COUNT);
        collisionBoxes.reserve(PIPE_COUNT);
        for (int i = 0; i < PIPE_COUNT; i++)
        {
            auto &ref                   = pipes.emplace_back(52, 320, std::vector<std::filesystem::path>{ "assets/sprites/pipe-red.png" });
            ref.transform.rotate        = i % 2 ? 180.0f : 0.f;
            ref.transform.translate.y   = i % 2 ? 256.0f : -256.0f;
            ref.transform.translate.x   = static_cast<int>(i / 2) * SPACING - 100.0f;
            components.push_back(&ref);

            auto &boxRef = collisionBoxes.emplace_back(this);
            boxRef.size = {52, 320};
            boxRef.transform.translate.y   = i % 2 ? 256.0f : -256.0f;
            boxRef.transform.translate.x   = static_cast<int>(i / 2) * SPACING - 100.0f;
            physics.collisionShapes.push_back(&boxRef);
        }
        components.push_back(&physics);
    }

    void update(float delta) override
    {
        for (const auto &[b, p] : std::ranges::views::zip(collisionBoxes, pipes))
        {
            const auto left = engine::vec2(-1.0f, 0.0f);
            auto velocity = left * FLOOR_SPEED * delta;

            p.transform.translate += velocity;
            if (p.transform.translate.x <= -170.0f)
                p.transform.translate.x += 170.0f * (PIPE_COUNT / 2.f);

            b.transform.translate += velocity;
            if (b.transform.translate.x <= -170.0f)
                b.transform.translate.x += 170.0f * (PIPE_COUNT / 2.f);
        }
    }
};

class MainScene : public engine::Scene
{
    Bird bird;
    Ground g1;
    Score score;
    Pipes pipes;

public:
    MainScene(Background &bg)
    {
        objects.push_back(&bg);
        objects.push_back(&bird);
        objects.push_back(&pipes);
        objects.push_back(&score);
        objects.push_back(&g1);
    }
};

class Message : public engine::Object
{
    engine::component::Sprite sprite {184, 267, {"assets/sprites/message.png"}};

public:
    Message() : engine::Object("Background message")
    { components.push_back(&sprite); }
};

class LoadScene : public engine::Scene
{
    Message msg;
    MainScene &mscn;

public:
    LoadScene(Background &bg, MainScene &scn)
        : mscn(scn)
    {
        objects.push_back(&bg);
        objects.push_back(&msg);
    }

    void update(float) override
    {
        if (engine::controls::isActionJustPressed("ui_accept"))
            engine::loadScene(&mscn);
    }
};

int main()
{
    engine::init("Flappy bird", 288, 512);
    engine::controls::registerAction("fly", SDLK_SPACE);
    engine::controls::registerAction("ui_accept", SDLK_RETURN);

    Background bg;
    MainScene scn(bg);
    LoadScene scn0(bg, scn);
    engine::loadScene(&scn0);

    engine::start();
    engine::cleanup();
    return EXIT_SUCCESS;
}
