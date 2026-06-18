#include "components.hpp"
#include "controls.hpp"
#include "textures.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <functional>
#include <initializer_list>
#include <objects.hpp>
#include <ranges>
#include <scene.hpp>
#include <engine.hpp>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

constexpr static float  FLOOR_SPEED = 150.0f;

namespace ecc = engine::component;

class Bird : public engine::Object
{
    // components loaded here so they can persist for the lifetime of the component
    std::array<engine::Texture, 3> textures{"assets/sprites/redbird-upflap.png",
                                            "assets/sprites/redbird-midflap.png",
                                            "assets/sprites/redbird-downflap.png"};

    ecc::Timer&             timer           = newComponent<ecc::Timer>();
    ecc::Sprite&            sprite          = newComponent<ecc::Sprite>(32, 24, std::vector{&textures[0], &textures[1], &textures[2]});
    constexpr static float  GRAVITY_SCALE   = 40.0f;
    engine::vec2<float>     velocity          {0, 0};
    ecc::Physics&           physics         = newComponent<ecc::Physics>();
    ecc::collision::Box&    hitBox          = physics.newCollisionShape<ecc::collision::Box>(this);

    // reference to MainScene::gameover. A shared scene-wide variable
    bool &gameover; 

public:
    Bird(bool &gameover) : engine::Object("Bird"), gameover(gameover)
    {
        // register components as pointers
        transform.translate = {-100, 64};
        hitBox.size         = {32.0f, 24.0f};
        update              =
            [this, &gameover](float deltaTime) {
                if (!gameover)
                {
                    if (engine::controls::isActionJustPressed("fly"))
                    {
                        velocity.y = 250.0f;
                        sprite.current_texture = 2;
                        timer.setTimeout([this](){ sprite.current_texture--; }, 500, 2);
                    }

                    auto coll = hitBox.checkCollision();
                    if (coll == nullptr)
                    {
                        const auto gravity   = physics.gravity;
                        velocity.y          += -gravity * GRAVITY_SCALE * deltaTime;
                    }
                    else
                        gameover = true;
                    transform.translate += velocity * deltaTime;
                }
            };
    }

    // reset bird when called
    void reset()
    {
        transform.translate = {-100, 64};
        velocity            = {0, 0};
    }
};

class Background : public engine::Object
{
    // components
    std::array<engine::Texture, 2>  textures{"assets/sprites/background-night.png",
                                             "assets/sprites/background-day.png"};
    ecc::Sprite&      sprite      = newComponent<ecc::Sprite>(288, 512, std::vector{&textures[0], &textures[1]});

public:
    // init stuffs
    Background() : engine::Object("Background")
    {
        sprite.transform.scale.y = -1.0f;   // for some reason the background sprite now appears upside down
                                            // after a recent commit.
    }
};

class Ground : public engine::Object
{
    // amount of sprites to use for the infinite moving floor implementation
    constexpr static int    SPRITE_SIZE = 2;
    
    // components
    std::vector<std::reference_wrapper<ecc::Sprite>> sprites = std::ranges::to<std::vector<std::reference_wrapper<ecc::Sprite>>>(
        std::ranges::views::iota(0, SPRITE_SIZE) |
        std::ranges::views::transform([this](auto i) -> ecc::Sprite& {
            auto &ref                   = newComponent<ecc::Sprite>(336, 112, std::vector{&groundTexture});
            ref.transform.translate.x   = i * 336;
            return ref;
        })
    );
    ecc::Physics&         physics = newComponent<ecc::Physics>();
    ecc::collision::Box&  hitBox  = physics.newCollisionShape<ecc::collision::Box>(this);
    engine::Texture groundTexture{"assets/sprites/base.png"};

    bool &gameover; // MainScene::gameover reference
    

public:
    // initialize stuffs
    Ground(bool &gameover) : engine::Object("Ground"), gameover(gameover)
    {
        transform.translate = {0, -200};
        hitBox.size = engine::vec2{336.0f, 112.0f};
        update      =
            [this, &gameover](float delta) {
                if (!gameover)
                {
                    for (auto &s : sprites)
                    {
                        const auto left          = engine::vec2(-1.0f, 0.0f);
                        auto velocity            = left * FLOOR_SPEED * delta;
                        s.get().transform.translate   += velocity;
                        if (s.get().transform.translate.x <= -336.0f)
                            s.get().transform.translate.x += 336.0f * 2.f;
                    }
                }
            };
    }
};

class Score : public engine::Object
{
    static constexpr int DIGITS = 3;    // only 3 digits for the score. So from 000 - 999
    static constexpr auto disp_factor = DIGITS % 2 == 0 ? .5f : 1.f;      // if odd add by 1 else add by half
    std::vector<std::reference_wrapper<ecc::Sprite>> sprites = std::ranges::to<std::vector<std::reference_wrapper<ecc::Sprite>>>(
        std::ranges::views::iota(0, DIGITS) |
        std::ranges::views::transform(
            [this](auto i) -> ecc::Sprite& {
                auto &ref = newComponent<ecc::Sprite>(
                                24, 36,
                                std::vector{&textures[0], &textures[1],
                                    &textures[2], &textures[3],
                                    &textures[4], &textures[5],
                                    &textures[6], &textures[7],
                                    &textures[8], &textures[9]});

                constexpr int half = DIGITS / 2;
                if (DIGITS % 2 && i == half)
                    // Do nothing. This prevents further condition block from moving 
                    // the sprite that's supposed to be at the center of the score
                    // when the number of digits is odd
                    (void)0;
                else
                    ref.transform.translate.x = (
                        (i - half) * disp_factor * 24 -     // 24 is the width of the sprite.
                        ((i - half) < 0 ? -1 : 1)           // Displace by 1 pixel to keep things looking cleaner and
                                                            // more compact
                    );

                return ref;
            }
        )
    );
    std::array<engine::Texture, 10> textures{"assets/sprites/0.png",
                                             "assets/sprites/1.png",
                                             "assets/sprites/2.png",
                                             "assets/sprites/3.png",
                                             "assets/sprites/4.png",
                                             "assets/sprites/5.png",
                                             "assets/sprites/6.png",
                                             "assets/sprites/7.png",
                                             "assets/sprites/8.png",
                                             "assets/sprites/9.png"};

public:
    uint16_t score = 0;
    Score() : engine::Object("Score")
    {
        transform.translate.y = 222;
        update                =
            [this](float) {
                auto _s = score;
                for (int i = DIGITS - 1; i >= 0; i--)
                {
                    sprites[i].get().current_texture = _s % 10;
                    _s /= 10;
                }
            };
    }
};

class Pipes : public engine::Object
{
    static constexpr int PIPE_COUNT = 4;
    static constexpr float SPACING = 200;   // distance between pipes
    static constexpr int PIPE_HEIGHT = 320;

    Score&          score;
    bool&           gameover;
    engine::Texture texture {"assets/sprites/pipe-red.png" };

    std::vector<std::reference_wrapper<ecc::Sprite>>          pipes          = std::ranges::to<std::vector<std::reference_wrapper<ecc::Sprite>>>(
        std::ranges::views::iota(0, PIPE_COUNT) |
        std::ranges::views::transform(
            [this](auto i) -> ecc::Sprite& {
                auto &ref                   = newComponent<ecc::Sprite>(52, 320, std::vector{ &texture });
                ref.transform.rotate        = i % 2 ? 180.0f : 0.f;   // i % 2 checks if it's the top pipe. true if it's an odd number.
                                                                      // false if it's an even number
                ref.transform.translate.y   = i % 2 ? 256.0f : -256.0f;
                ref.transform.translate.x   = static_cast<int>(i / 2) * SPACING - 100.0f;
                return ref;
            }
        )
    );
    ecc::Physics&                                             physics        = newComponent<ecc::Physics>();
    std::vector<std::reference_wrapper<ecc::collision::Box>>  collisionBoxes = std::ranges::to<std::vector<std::reference_wrapper<ecc::collision::Box>>>(
        std::ranges::views::iota(0, PIPE_COUNT) |
        std::ranges::views::transform(
            [this](auto i) -> ecc::collision::Box& {
                auto &boxRef                    = physics.newCollisionShape<ecc::collision::Box>(this);
                boxRef.size                     = {52, 320};
                boxRef.transform.translate.y    = i % 2 ? 256.0f : -256.0f;
                boxRef.transform.translate.x    = static_cast<int>(i / 2) * SPACING - 100.0f;
                return boxRef;
            }
        )
    );

public:
    Pipes(Score &score, bool &gameover)
    : engine::Object("Pipes"), score(score), gameover(gameover)
    {
        update =
            [this, &gameover, &score](float delta) {
                if (!gameover)
                {
                    // move all the ppes back at the same speed, and move them to the other side of the screen
                    // when it crosses the screen behind the player.
                    auto add = 0.0f;
                    int bottomPipe = 0;

                    for (auto i : std::ranges::views::iota(0, PIPE_COUNT))
                    {
                        auto &b = collisionBoxes[i].get();
                        auto &p = pipes[i].get();
                        bool isTop = i % 2;

                        const auto left = engine::vec2(-1.0f, 0.0f);
                        auto velocity = left * FLOOR_SPEED * delta;

                        p.transform.translate += velocity;
                        b.transform.translate += velocity;
                        if (p.transform.translate.x <= -170.0f)
                        {
                            p.transform.translate.x += SPACING * (PIPE_COUNT / 2.f);
                            b.transform.translate.x += SPACING * (PIPE_COUNT / 2.f);
                            add += 0.5f;    // too lazy to make a proper pipe-cross-player detection and this was good enough
                                            // so here it is.

                            if (isTop)
                            {
                                auto gap = bottomPipe + PIPE_HEIGHT + (rand() % (250 - 150)) + 100;
                                p.transform.translate.y = gap;
                                b.transform.translate.y = gap;
                            }
                            else
                            {
                                bottomPipe = 0 - (rand() % (PIPE_HEIGHT / 2) - 120) - (PIPE_HEIGHT * 0.75f);
                                p.transform.translate.y = bottomPipe;
                                b.transform.translate.y = bottomPipe;
                            }
                        }

                    }
                    score.score += add;
                }
            };
    }

    // reset the pipes to the exact starting positions (Duplicate of what's
    // happening in the constructor, but without initializations)
    void reset()
    {
        for (int i = 0; i < PIPE_COUNT; i++)
        {
            auto &ref                       = pipes[i].get();
            ref.transform.rotate            = i % 2 ? 180.0f : 0.f;
            ref.transform.translate.y       = i % 2 ? 256.0f : -256.0f;
            ref.transform.translate.x       = static_cast<int>(i / 2) * SPACING - 100.0f;

            auto &boxRef                    = collisionBoxes[i].get();
            boxRef.size                     = {52, 320};
            boxRef.transform.translate.y    = i % 2 ? 256.0f : -256.0f;
            boxRef.transform.translate.x    = static_cast<int>(i / 2) * SPACING - 100.0f;
        }
    }
};

class Gameover : public engine::Object
{
    engine::Texture texture {"assets/sprites/gameover.png"};
    ecc::Sprite &sprite = newComponent<ecc::Sprite>(192, 42, std::vector{&texture});
public:
    Gameover() : engine::Object("Gameover") {}
};

class MainScene : public engine::Scene
{
    bool        gameover    = false;
    Bird        bird        {gameover};
    Ground      g1          {gameover};
    Score       score;
    Pipes       pipes       {score, gameover};
    Gameover    gv;

public:
    MainScene(Background &bg)
    {
        objects.push_back(&bg);
        objects.push_back(&bird);
        objects.push_back(&pipes);
        objects.push_back(&score);
        objects.push_back(&g1);

        // primary purpose is just for tracking gameover state
        // and handling that aspect.
        update =
            [this](float) {
                static bool alreadyover = false;
                if (!alreadyover && gameover)
                {
                    alreadyover = true;
                    objects.push_back(&gv);
                }
                else if (alreadyover)
                {
                    if (engine::controls::isActionJustPressed("ui_accept"))
                    {
                        alreadyover = false;
                        std::erase(objects, &gv);
                        score.score = 0;
                        bird.reset();
                        pipes.reset();
                        gameover = false;
                    }
                }
            };
    }
};

class Message : public engine::Object
{
    // components loaded here so they can persist for the lifetime of the component
    engine::Texture texture {"assets/sprites/message.png"};
    ecc::Sprite &sprite = newComponent<ecc::Sprite>(184, 267, std::vector{&texture});

public:
    Message() : engine::Object("Background message") {}
};

// Initial load screen. The first scene you see when you open the game
class LoadScene : public engine::Scene
{
    Message msg;
    MainScene &mscn;

public:
    LoadScene(Background &bg, MainScene &scn) : mscn(scn)
    {
        objects.push_back(&bg);
        objects.push_back(&msg);
        update =
            [this](float deltaTime){
                // press enter or space to start playing the game
                if (engine::controls::isActionJustPressed("ui_accept"))
                    engine::scene::load(&mscn);
            };
    }
};

int main()
{
    std::srand(std::time(0));
    // initialize the engine and register some key bindings
    engine::init("Flappy bird", 288, 512);  // TODO: allow passing window flags from here. Likely end up with a custom flag
                                            // setup because I have to also be able to translate the features to the engine's gameview.
                                            // Maybe something similar to vulkan options.
    engine::controls::registerAction("fly", SDLK_SPACE);
    engine::controls::registerAction("ui_accept", SDLK_RETURN);
    engine::controls::registerAction("ui_accept", SDLK_SPACE);

    Background bg;  // background image. Since it exists in every scene, it's here
    MainScene scn(bg);
    LoadScene scn0(bg, scn);
    engine::scene::load(&scn0);

    engine::start();    // the game loop and every runtime stuff happens inside here
}

