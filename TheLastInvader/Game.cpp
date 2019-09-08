/*
 Game.cpp
 
 +-------------------------------------------------------------+
 |                     The Last Invader                        |
 |                Entry to #olcCodeJam2019                     |
 +-------------------------------------------------------------+
 
 Special Thanks:
 ~~~~~~~~~~~~~~~	
 javidx9 - The olcPixelGameEngine, the lifeblood of the game.
 
 License (GAH-3)
 ~~~~~~~~~~~~~~~
 
 Copyright 2018 - 2019 Gareth Hubbal;
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions or derivations of source code must retain the above
 copyright notice, this list of conditions and the following disclaimer.
 
 2. Redistributions or derivative works in binary form must reproduce
 the above copyright notice. This list of conditions and the following
 disclaimer must be reproduced in the documentation and/or other
 materials provided with the distribution.
 
 3. Neither the name of the copyright holder nor the names of its
 contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Links
 ~~~~~
 YouTube:    https://www.youtube.com/channel/UC0B3TK74rxHImJcuSR9PkqQ
 Discord:    https://discord.gg/VuMTS3Z
 Twitter:    https://www.twitter.com/garethhubball
 Twitch:     https://www.twitch.tv/garethhubball
 GitHub:     https://www.github.com/garethhubball
 
 Author
 ~~~~~~
 Gareth Hubball, ©Gareth Hubball 2019
*/


#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

#include <list>

#define CHAR_HEIGHT 8
#define CHAR_WIDTH 8

const int starting_durability = 15;

enum GameState {
    CREDITS,
    START_SCREEN,
    START_ROUND,
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct Line {
	float x1, y1, x2, y2;
};

struct Building {
	int height;
	int durability = starting_durability;
	std::vector<Line> desctruction_lines;
    
	Building(int height) : height(height)
	{}
};

struct PlayerMovement {
    enum Direction {
        LEFT, RIGHT, DOWN
    };
    
    Direction direction;
    olc::vf2d position;
};

struct Bullet {
    olc::vf2d position;
    bool collided;
    bool off_screen;
};

struct Missile {
    olc::vf2d position;
    olc::vf2d direction;
    float speed;
    bool collided;
    bool off_screen;
};

float normalised_random() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float random_between(float low, float high) {
    return low + (high - low) * normalised_random();
}

class Game :
public olc::PixelGameEngine
{
    private:
    
    static bool low_pip;
    static bool high_pip;
    
    static float PipFunction(int channel, float global_time, float time_step) {
        if (low_pip) {
            return sin(349 * 2.0f * 3.14159f * global_time);
        }
        else if (high_pip) {
            return sin(698 * 2.0f * 3.14159f * global_time);
        }
        else {
            return 0.0f;
        }
    }
    
    
    public:
    
    Game()
    {
        sAppName = "The Last Invader";
        olc::SOUND::InitialiseAudio();
		sampleMusic = olc::SOUND::LoadAudioSample("GameMusic.wav");
        
    }
    
    bool OnUserCreate() override
    {
        srand(static_cast<uint32_t>(time(0)));
        
        
        Clear(olc::BLACK);
        DrawStringCentered("LOADING MUSIC");
        DrawStringCenteredX("THE LAST INVADER", ScreenHeight() / 10, olc::WHITE, 2);
        player_sprite = new olc::Sprite("SpriteSheet.png");
        
        olc::SOUND::SetUserSynthFunction(PipFunction);
        
        return true;
    }
    
    bool OnUserDestroy() override
    {
        olc::SOUND::DestroyAudio();
        delete player_sprite;
        return true;
    }
    
    void GameScreen(float fElapsedTime) {
        
        const float acceleration = 2.0f;
        float modified_velocity;
        float building_width = ScreenWidth() / static_cast<float>(buildings.size());
        
        time_to_next_shot -= fElapsedTime;
        time_to_next_player_shot -= fElapsedTime;
        time_to_spite_change -= fElapsedTime;
        
        if (time_to_spite_change <= 0) {
            left_sprite = !left_sprite;
            time_to_spite_change += sprite_interval;
        }
        
        if (time_to_next_shot <= 0) {
            time_to_next_shot += enemy_shot_interval;
            
            for (uint32_t i = 0; i < buildings.size(); i++) {
                float x = building_width / 2.0f + i * building_width;
                float y = ScreenHeight() - static_cast<float>(buildings[i].height * block_height);
                enemy_bullets.push_back({ {x, y},olc::vf2d(random_between(-0.4f, 0.4f), 1.0f).norm(), random_between(0.75f, 1.25f), false, false });
            }
            
            score += 1000;
        }
        
        if (GetKey(olc::Q).bHeld)
            modified_velocity = velocity * acceleration;
        else if (GetKey(olc::A).bHeld)
            modified_velocity = velocity / acceleration;
        else
            modified_velocity = velocity;
        
        // if (GetKey(olc::SPACE).bPressed)
        if (time_to_next_player_shot < 0) {
            bullets.push_back({ { player_movement.position.x + 0.5f * player_width, player_movement.position.y + player_height }, false, false });
            time_to_next_player_shot += autofire_interval;
        }
        
        player_movement = MovePlayer(player_movement, modified_velocity * fElapsedTime);
        
        Clear(olc::BLACK);
        
        // Draw the player
        //DrawRect(player_movement.position.x, player_movement.position.y, 10, 10);
        
        int ox = 0, oy = 0;
        if (left_sprite)
            ox = base_width;
        
        DrawPartialSprite(static_cast<int>(player_movement.position.x), static_cast<int>(player_movement.position.y), player_sprite, ox, oy, base_width, base_height, scale);
        
        std::vector<bool> building_collisions{ false, false, false, false, false, false, false, false, false, false };
        
        // Collision of bullets and buildings
        for (auto& bullet : bullets) {
            int index = static_cast<int>(std::floor(bullet.position.x / building_width));
            int building_height = buildings[index].height * block_height;
            
            if (building_height == 0 && bullet.position.y > ScreenHeight()) {
                bullet.off_screen = true;
            }
            
            if (bullet.position.y + 4 >= ScreenHeight() - building_height) {
                bullet.collided = true;
                building_collisions[index] = true;
                score += (10 - buildings[index].height) * 100;
            }
            else {
                bullet.collided = false;
            }
        }
        
        // Collision of missiles and the player, only checking the first pixel against the whole of the player sprite.
        // TODO(gareth): Maybe look into masks and more accurate collision.
        
        int player_x_min = static_cast<int>(player_movement.position.x);
        int player_y_min = static_cast<int>(player_movement.position.y);
        
        int player_x_max = player_x_min + player_width;
        int player_y_max = player_y_min + player_height;
        
        for (auto& missile: enemy_bullets) {
            if (missile.position.x >= player_x_min && missile.position.x <= player_x_max && missile.position.y >= player_y_min && missile.position.y <= player_y_max){
                
                missile.collided = true;
                lives--;
                game_state = START_ROUND;
                
                olc::SOUND::StopSample(sampleMusic);
                
            }
            else
            {
                missile.collided = false;
            }
        }
        
        // Break the buildings that have been shot
        for (uint32_t i = 0; i < building_collisions.size(); i++)
        {
            if (building_collisions[i])
            {
                buildings[i].durability--;
                if (buildings[i].durability == 0) {
                    buildings[i].height--;
                    buildings[i].durability = starting_durability;
                    buildings[i].desctruction_lines = {};
                }
                else {
                    int left = static_cast<int>(i * building_width);
                    int right = static_cast<int>((i + 1) * building_width);
                    int top = ScreenHeight() - static_cast<int>(buildings[i].height * block_height);
                    int bottom = top + block_height;
                    
                    float x1 = left + (right - left) * normalised_random();
                    float x2 = left + (right - left) * normalised_random();
                    
                    float y1 = top + (bottom - top) * normalised_random();
                    float y2 = top + (bottom - top) * normalised_random();
                    buildings[i].desctruction_lines.push_back({ x1, y1, x2, y2 });
                }
            }
        }
        
        // Filter out old bullets
        bullets.remove_if([](const Bullet& bullet) { return bullet.collided || bullet.off_screen; });
        enemy_bullets.remove_if([](const Missile& missile) { return missile.collided || missile.off_screen; });
        
        // Draw the bullets
        for (auto& bullet : bullets) {
            DrawLine(static_cast<int>(bullet.position.x), static_cast<int>(bullet.position.y), static_cast<int>(bullet.position.x), static_cast<int>(bullet.position.y + 4), olc::GREEN);
            bullet.position.y += 350.0f * fElapsedTime;
        }
        
        //Draw enemy bullets
        for (auto& bullet : enemy_bullets) {
            auto middle = 4 * bullet.direction + bullet.position;
            auto back = 10 * bullet.direction + bullet.position;
            DrawLine(static_cast<int>(bullet.position.x), static_cast<int>(bullet.position.y), static_cast<int>(middle.x), static_cast<int>(middle.y), olc::RED);
            DrawLine(static_cast<int>(bullet.position.x), static_cast<int>(bullet.position.y), static_cast<int>(back.x), static_cast<int>(back.y), olc::DARK_RED);
            Draw(static_cast<int>(bullet.position.x), static_cast<int>(bullet.position.y), olc::YELLOW);
            bullet.position = bullet.position - (250.0f * fElapsedTime * bullet.speed) * bullet.direction;
            if (bullet.position.x < 0 || bullet.position.x > ScreenWidth() || bullet.position.y < 0)
                bullet.off_screen = true;
        }
        
        // Draw the buildings to destroy
        auto left = 0;
        
        for (auto building : buildings) {
            FillRect(left, ScreenHeight() - building.height * block_height, static_cast<int>(building_width), building.height * block_height, olc::GREEN);
            
            for (auto line : building.desctruction_lines) {
                DrawLine(static_cast<int>(line.x1), static_cast<int>(line.y1), static_cast<int>(line.x2), static_cast<int>(line.y2), olc::BLACK);
            }
            
            left += static_cast<int>(building_width);
        }
        
        DrawString(5, 5, "LIVES: " + std::to_string(lives));
        
        
        int score_x = (ScreenWidth() - 20 * CHAR_WIDTH) - 5;
        
        char score_buffer[12];
        sprintf_s(score_buffer, "%011I64d", score);
        DrawString(score_x, 5, "DAMAGE: $" + std::string(score_buffer));
        
        if (lives <= 0)
            game_state = GAME_OVER;
    }
    
    PlayerMovement MovePlayer(PlayerMovement movement, float displacement)
    {
        using Direction = PlayerMovement::Direction;
        if (movement.direction == Direction::RIGHT)
        {
            float right_boundary = ScreenWidth() - margin - player_width;
            float newPos{ movement.position.x + displacement };
            if (newPos <= right_boundary) {
                return { Direction::RIGHT, {newPos, movement.position.y} };
            }
            else {
                return { Direction::DOWN, {right_boundary, movement.position.y + (newPos - right_boundary) } };
            }
        }
        
        if (movement.direction == Direction::DOWN)
        {
            auto current_layer = std::floor(movement.position.y / layer_height);
            auto new_layer = std::floor((movement.position.y + displacement) / layer_height);
            
            if (current_layer == new_layer) {
                return { Direction::DOWN, {movement.position.x, movement.position.y + displacement} };
            }
            else {
                auto y = new_layer * layer_height;
                auto x_movement = movement.position.y + displacement - y;
                if (movement.position.x <= margin) {
                    return { Direction::RIGHT, { movement.position.x + x_movement, y } };
                }
                else {
                    return { Direction::LEFT, { movement.position.x - x_movement, y } };
                }
            }
        }
        
        if (movement.direction == Direction::LEFT)
        {
            auto newPos = movement.position.x - displacement;
            if (newPos >= margin) {
                return { Direction::LEFT, {newPos, movement.position.y} };
            }
            else {
                return { Direction::DOWN, {margin, movement.position.y + (margin - newPos) } };
            }
        }
        
        return movement;
    }
    
    void DrawStringCenteredX(std::string text, uint32_t y, olc::Pixel pixel, uint32_t scale) {
        DrawString(CenterHorizontal(text, scale), y, text, pixel, scale);
    }
    
    
    bool OnUserUpdate(float fElapsedTime) override
    {
        switch (game_state) {
            case CREDITS:
            {
                Clear(olc::BLACK);
                DrawStringCenteredX("THE LAST INVADER", ScreenHeight() / 10, olc::WHITE, 2);
                DrawStringCenteredX("CREDITS", (ScreenHeight() / 10) + 20, olc::WHITE, 2);
                
                int y = static_cast<int>(ScreenHeight() * 0.2f);
                
                
                // NOTE(gareth): CC-BY-SA attribution
                DrawStringCenteredX("MUSIC BY OZZED: https://ozzed.net/", y,  olc::WHITE, 1);
                DrawStringCenteredX("POWERED BY olcPixelGameEngine WITH THE SOUND EXTENSION.", y + 20,  olc::WHITE, 1);
                DrawStringCenteredX("olcPixelGameEngine Copyright 2018 - 2019 OneLoneCoder.com", y + 35,  olc::WHITE, 1);
                DrawStringCenteredX("GAME DESIGN AND DEVELOPMENT: Copyright 2019 Gareth Hubball", y + 55, olc::WHITE, 1);
                DrawStringCenteredX("https://twitch.tv/garethhubball", y + 70, olc::WHITE, 1);
                
                DrawStringCenteredX("PRESS ESC TO RETURN TO THE GAME", static_cast<int>(ScreenHeight() * 0.9f),  olc::WHITE, 1);
                
                if (GetKey(olc::ESCAPE).bPressed) {
                    game_state = START_SCREEN;
                }
                
            } break;
            
            case START_SCREEN: {
                Clear(olc::BLACK);
                DrawStringCenteredX("THE LAST INVADER", ScreenHeight() / 10, olc::WHITE, 2);
                
                int y = static_cast<int>(ScreenHeight() * 0.4f);
                DrawStringCenteredX("PRESS Q TO SPEED UP", y - 30, olc::WHITE, 1);
                DrawStringCenteredX("PRESS A TO SLOW DOWN", y - 15, olc::WHITE, 1);
                DrawStringCenteredX("PRESS P TO PAUSE/UNPAUSE", y, olc::WHITE, 1);
                DrawStringCenteredX("PRESS SPACE TO START THE GAME", y + 15, olc::WHITE, 1);
                DrawStringCenteredX("PRESS C FOR CREDITS", y + 30, olc::WHITE, 1);
                
                DrawStringCenteredX("GOOD LUCK!", static_cast<int>(ScreenHeight() * 0.8f), olc::WHITE, 2);
                
                if (GetKey(olc::C).bPressed) {
                    game_state = CREDITS;
                }
                
                if (GetKey(olc::SPACE).bPressed) {
                    Clear(olc::BLACK);
                    game_state = START_ROUND;
                    olc::SOUND::StopSample(sampleMusic);
                }
                
            } break;
            
            case START_ROUND: { // When we're starting the game, or have lost a life
                time_to_start -= fElapsedTime;
                std::string countdown;
                
                if (time_to_start > 0.5f) {
                    countdown = std::to_string(static_cast<int>(std::floor(time_to_start + 0.5f)));
                    
                    if (time_to_start + 0.5f - (int)(time_to_start + 0.5f) > 0.8f) {
                        low_pip = true;
                    } else {
                        low_pip = false;
                    }
                } else {
                    low_pip = false;
                    if (time_to_start > 0.0f) {
                        high_pip = true;
                    } else {
                        high_pip = false;
                    }
                    countdown = "GO!";
                }
                
                uint32_t width = StringWidth("GO!", 3) + 3;
                uint32_t height = CHAR_HEIGHT * 3 + 3;
                
                FillRect((ScreenWidth() - width) / 2, (ScreenHeight() - height) / 2, width, height, olc::BLACK);
                
                // Clear(olc::BLACK);
                DrawStringCentered(countdown, olc::WHITE, 3);
                
                if (time_to_start <= 0) {
                    time_to_start = 3.499f;
                    
                    // Reset the play field.
                    player_movement = starting_movement;
                    
                    bullets = {};
                    enemy_bullets = {};
                    olc::SOUND::PlaySample(sampleMusic, true);
                    game_state = PLAYING;
                }
            } break;
            
            case PLAYING: {
                GameScreen(fElapsedTime);
                
                if (GetKey(olc::P).bPressed) {
                    game_state = PAUSED;
                }
            } break;
            
            case PAUSED: {
                DrawStringCentered("PAUSED!", olc::WHITE, 2);
                
                if (GetKey(olc::P).bPressed) {
                    game_state = PLAYING;
                }
            } break;
            
            case GAME_OVER: { 
                DrawStringCentered("GAME OVER!", olc::WHITE, 2);
                
                DrawStringCenteredX("YOU COST THE PLANET: $" + std::to_string(score), static_cast<int>(ScreenHeight() * 0.5f) + 10, olc::WHITE, 1);
                DrawStringCenteredX("TO START AGAIN PRESS SPACE", static_cast<int>(ScreenHeight() * 0.5f) + 20, olc::WHITE, 1);
                
                if (GetKey(olc::SPACE).bPressed) {
                    Clear(olc::BLACK);
                    
                    lives = 3;
                    score = 0;
                    game_state = START_ROUND;
                    
                    buildings = starting_buildings;
                }
                
            } break;
        };
        
        return true;
    }
    
    // TODO(gareth): Should we move string handling out at some point?
    // NOTE(gareth): This function does not account for new lines, DrawString does.
    inline uint32_t StringWidth(std::string text, uint32_t scale = 1)
    {
        return static_cast<uint32_t>(text.size() * CHAR_WIDTH * scale);
    }
    
    inline uint32_t CenterHorizontal(std::string text, uint32_t scale) {
        return (ScreenWidth() - StringWidth(text, scale)) / 2;
    }
    
    inline uint32_t CenterVertical(uint32_t scale) {
        int text_height = CHAR_HEIGHT * scale;
        return (ScreenHeight() - text_height) / 2;
    }
    
    void DrawStringCentered(std::string text, olc::Pixel pixel = olc::WHITE, uint32_t scale = 1)
    {
        int x = CenterHorizontal(text, scale);
        int y = CenterVertical(scale);
        
        DrawString(x, y, text, pixel, scale);
    }
    
    private:
    
    const float velocity = 70.0f;
    const float enemy_shot_interval = 1.0f; // seconds
    
    const int scale = 2;
    const int base_width = 10;
    const int base_height = 10;
    
    const float margin = 20;
    const int layer_height = 20;
    
    const int player_width = base_width * scale;
    const int player_height = base_height * scale;
    
    const int block_height = 15;
    
    const float autofire_interval = 0.1f;
    
    float time_to_start = 3.49f;
    
    std::vector<Building> starting_buildings{ 4, 5, 3, 2, 6, 4, 6, 3, 5, 2 };
    std::vector<Building> buildings = starting_buildings;
    
    std::list<Bullet> bullets;
    std::list<Missile> enemy_bullets;
    PlayerMovement starting_movement{ PlayerMovement::Direction::RIGHT, { 20.0f, 20.0f } };
    PlayerMovement player_movement = starting_movement;
    
    olc::Sprite* player_sprite;
    
    GameState game_state = START_SCREEN;
    
    int sampleMusic;
    uint64_t score = 0; // NOTE(gareth): Most people are better at games than I am.
    
    float time_to_next_shot = enemy_shot_interval;
    float time_to_next_player_shot = autofire_interval;
    
    const float bpm = 160.0f;
    const float sprite_interval = 60.0f / bpm;
    
    bool left_sprite = false;
    
    float time_to_spite_change = sprite_interval;
    
    int8_t lives = 3;
};

bool Game::low_pip = false;
bool Game::high_pip = false;

int main()
{
    Game game;
    if (game.Construct(640, 480, 2, 2))
        game.Start();
    return 0;
}