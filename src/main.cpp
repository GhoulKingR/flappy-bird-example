#include "components.hpp"
#include "controls.hpp"
#include "textures.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
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

class Bird : public engine::Object
{
    // components loaded here so they can persist for the lifetime of the component
    std::array<engine::Texture, 3> textures{"assets/sprites/redbird-upflap.png",
                                            "assets/sprites/redbird-midflap.png",
                                            "assets/sprites/redbird-downflap.png"};

    engine::component::Timer            timer;
    engine::component::Sprite           sprite          { 32, 24, {&textures[0], &textures[1], &textures[2]} };
    constexpr static float              GRAVITY_SCALE   = 40.0f;
    engine::vec2<float>                 velocity        {0, 0};
    engine::component::Physics          physics;
    engine::component::collision::Box   hitBox          {this};

    // reference to MainScene::gameover. A shared scene-wide variable
    bool &gameover; 

public:
    Bird(bool &gameover) : engine::Object("Bird"), gameover(gameover)
    {
        // register components as pointers
        transform.translate = {-100, 64};
        components.push_back(&sprite);
        components.push_back(&timer);

        hitBox.size                 = {32.0f, 24.0f};
        physics.collisionShapes.push_back(&hitBox);
        components.push_back(&physics);

        update =
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
    engine::component::Sprite       sprite  {288, 512, {&textures[0], &textures[1]}};

public:
    // init stuffs
    Background() : engine::Object("Background")
    {
        components.push_back(&sprite);
        sprite.transform.scale.y = -1.0f;   // for some reason the background sprite now appears upside down
                                            // after a recent commit.
    }
};

class Ground : public engine::Object
{
    // amount of sprites to use for the infinite moving floor implementation
    constexpr static int    SPRITE_SIZE = 2;
    
    // components
    std::vector<engine::component::Sprite>  sprites;    // TODO (Maybe?): delete move and copy constructors for sprite.
                                                        // Tho there isn't really a need to because the reason I would've originally
                                                        // had for deleting the constructors has been solved by the new (as of now)
                                                        // `Texture` class.
    engine::component::Physics              physics;
    engine::component::collision::Box       hitBox{this};
    engine::Texture                         groundTexture{"assets/sprites/base.png"};

    bool &gameover; // MainScene::gameover reference

public:
    // initialize stuffs
    Ground(bool &gameover) : engine::Object("Ground"), gameover(gameover)
    {
        transform.translate = {0, -200};
        sprites.reserve(SPRITE_SIZE);
        for (int i = 0; i < SPRITE_SIZE; i++)
        {
            auto &ref                   = sprites.emplace_back(336, 112, std::vector{&groundTexture});
            ref.transform.translate.x   = i * 336;
            components.push_back(&ref);
        }
        hitBox.size = engine::vec2{336.0f, 112.0f};
        physics.collisionShapes.push_back(&hitBox);
        components.push_back(&physics);

        update = 
            [this, &gameover](float delta) {
                if (!gameover)
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
    }
};

class Score : public engine::Object
{
    static constexpr int DIGITS = 2;    // only 2 digits for the score. So from 00 - 99
    std::vector<engine::component::Sprite>  sprites;
    std::array<engine::Texture, 10>         textures{"assets/sprites/0.png",
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
    uint8_t score = 0;
    Score() : engine::Object("Score")
    {
        transform.translate.y = 222;
        sprites.reserve(DIGITS);
        for (int i = 0; i < DIGITS; i++)
        {
            auto &ref = sprites.emplace_back(24, 36,
                                             std::vector{&textures[0],
                                                         &textures[1],
                                                         &textures[2],
                                                         &textures[3],
                                                         &textures[4],
                                                         &textures[5],
                                                         &textures[6],
                                                         &textures[7],
                                                         &textures[8],
                                                         &textures[9]});
            components.push_back(&ref);
        }
        sprites[0].transform.translate.x = -11;
        sprites[1].transform.translate.x = 11;

        update =
            [this](float) {
                if (score < 100)
                {
                    sprites[0].current_texture = static_cast<uint32_t>(score / 10);
                    sprites[1].current_texture = static_cast<uint32_t>(score % 10);
                }
            };
    }
};

class Pipes : public engine::Object
{
    static constexpr int PIPE_COUNT = 4;
    static constexpr float SPACING = 175;   // distance between pipes

    engine::component::Physics                      physics;
    std::vector<engine::component::Sprite>          pipes;
    std::vector<engine::component::collision::Box>  collisionBoxes;
    engine::Texture                                 texture {"assets/sprites/pipe-red.png" };
    Score&                                          score;
    bool&                                           gameover;

public:
    Pipes(Score &score, bool &gameover) : engine::Object("Pipes"), score(score), gameover(gameover)
    {
        pipes.reserve(PIPE_COUNT);
        collisionBoxes.reserve(PIPE_COUNT);
        for (int i = 0; i < PIPE_COUNT; i++)
        {
            auto &ref                   = pipes.emplace_back(52, 320, std::vector{ &texture });
            ref.transform.rotate        = i % 2 ? 180.0f : 0.f;
            ref.transform.translate.y   = i % 2 ? 256.0f : -256.0f;
            ref.transform.translate.x   = static_cast<int>(i / 2) * SPACING - 100.0f;
            components.push_back(&ref);

            auto &boxRef                    = collisionBoxes.emplace_back(this);
            boxRef.size                     = {52, 320};
            boxRef.transform.translate.y    = i % 2 ? 256.0f : -256.0f;
            boxRef.transform.translate.x    = static_cast<int>(i / 2) * SPACING - 100.0f;
            physics.collisionShapes.push_back(&boxRef);
        }
        components.push_back(&physics);

        update =
            [this, &gameover, &score](float delta) {
                if (!gameover)
                {
                    // move all the ppes back at the same speed, and move them to the other side of the screen
                    // when it crosses the screen behind the player.
                    auto add = 0.0f;
                    for (const auto &[b, p] : std::ranges::views::zip(collisionBoxes, pipes))
                    {
                        const auto left = engine::vec2(-1.0f, 0.0f);
                        auto velocity = left * FLOOR_SPEED * delta;

                        p.transform.translate += velocity;
                        if (p.transform.translate.x <= -170.0f)
                        {
                            p.transform.translate.x += 170.0f * (PIPE_COUNT / 2.f);
                            add += 0.5f;    // too lazy to make a proper pipe-cross-player detection and this was good enough
                                            // so here it is.
                        }

                        // the collision box should also follow the pipes
                        b.transform.translate += velocity;
                        if (b.transform.translate.x <= -170.0f)
                            b.transform.translate.x += 170.0f * (PIPE_COUNT / 2.f);
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
            auto &ref                       = pipes[i];
            ref.transform.rotate            = i % 2 ? 180.0f : 0.f;
            ref.transform.translate.y       = i % 2 ? 256.0f : -256.0f;
            ref.transform.translate.x       = static_cast<int>(i / 2) * SPACING - 100.0f;
            auto &boxRef                    = collisionBoxes[i];
            boxRef.size                     = {52, 320};
            boxRef.transform.translate.y    = i % 2 ? 256.0f : -256.0f;
            boxRef.transform.translate.x    = static_cast<int>(i / 2) * SPACING - 100.0f;
        }
    }
};

class Gameover : public engine::Object
{
    engine::Texture texture {"assets/sprites/gameover.png"};
    engine::component::Sprite sprite{192, 42, {&texture}};

public:
    Gameover() : engine::Object("Gameover")
    { components.push_back(&sprite); }
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
    engine::component::Sprite sprite {184, 267, {&texture}};

public:
    Message() : engine::Object("Background message")
    { components.push_back(&sprite); }
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
    // initialize the engine and register some key bindings
    engine::init("Flappy bird", 288, 512);  // TODO: allow passing window flags from here. Likely end up with a custom flag
                                            // setup because I have to also be able to translate the features to the engine's gameview
    engine::controls::registerAction("fly", SDLK_SPACE);
    engine::controls::registerAction("ui_accept", SDLK_RETURN);
    engine::controls::registerAction("ui_accept", SDLK_SPACE);

    Background bg;  // background image. Since it exists in every scene, it's here
    MainScene scn(bg);
    LoadScene scn0(bg, scn);
    engine::scene::load(&scn0);

    engine::start();    // the game loop and every runtime stuff happens inside here
    engine::cleanup();
    return EXIT_SUCCESS;
}
