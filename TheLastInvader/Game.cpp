#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <list>

enum player_direction {
	LEFT, RIGHT, DOWN
};

struct player_movement {
	player_direction direction;
	olc::vf2d position;
};

struct bullet {
	olc::vf2d position;
	bool collided;
	bool beneath_screen;
};

class Game :
	public olc::PixelGameEngine
{
public:

	Game()
	{
		sAppName = "The Last Invader";
	}

	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		const float acceleration = 2.0f;
		int modified_velocity;

		if (GetKey(olc::Q).bHeld)
			modified_velocity = velocity * acceleration;
		else if (GetKey(olc::A).bHeld)
			modified_velocity = velocity / acceleration;
		else
			modified_velocity = velocity;

		if (GetKey(olc::SPACE).bPressed)
			bullets.push_back({ { player_movement.position.x + 5, player_movement.position.y + 10 }, false, false });

		player_movement = MovePlayer(player_movement, modified_velocity * fElapsedTime);

		Clear(olc::BLACK);
		
		// Draw the player
		DrawRect(player_movement.position.x, player_movement.position.y, 10, 10);

		auto building_width = ScreenWidth() / building_heights.size();
		std::vector<bool> building_collisions{ false, false, false, false, false, false, false, false, false, false };

		// Collision of bullets and buildings
		for (auto& bullet : bullets) {
			auto index = std::floor(bullet.position.x / building_width);
			auto building_height = building_heights[index] * 15;

			if (building_height == 0 && bullet.position.y > 480) {
				bullet.beneath_screen = true;
			}

			if (bullet.position.y + 4 >= 480 - building_height) {
				bullet.collided = true;
				building_collisions[index] = true;
			}
			else {
				bullet.collided = false;
			}
		}

		// Break the buildings that have been shot
		for (int i = 0; i < building_collisions.size(); i++)
		{
			if (building_collisions[i])
				building_heights[i]--;
		}

		// Filter out old bullets
		bullets.remove_if([](const bullet& bullet) { return bullet.collided || bullet.beneath_screen; });

		// Draw the bullets
		for (auto& bullet : bullets) {
			DrawLine(bullet.position.x, bullet.position.y, bullet.position.x, bullet.position.y + 4, olc::RED);
			bullet.position.y += 350.0f * fElapsedTime;
		}

		// Draw the buildings to destroy
		auto left = 0;

		for (auto height : building_heights) {
			FillRect(left, 480 - height * 15, building_width, height * 15, olc::BLUE);
			left += building_width;
		}

		return true;
	}

	player_movement MovePlayer(player_movement movement, float displacement)
	{
		if (movement.direction == RIGHT)
		{
			float newPos{ movement.position.x + displacement };
			if (newPos <= 610.0f) {
				return { RIGHT, {newPos, movement.position.y} };
			}
			else {
				return { DOWN, {610.0f, movement.position.y + (newPos - 610.0f) } };
			}
		}

		if (movement.direction == DOWN)
		{
			auto current_layer = std::floor(movement.position.y / 20.0f);
			auto new_layer = std::floor((movement.position.y + displacement) / 20.0f);
			
			if (current_layer == new_layer) {
				return { DOWN, {movement.position.x, movement.position.y + displacement} };
			}
			else {
				auto y = new_layer * 20;
				auto x_movement = movement.position.y + displacement - y;
				if (movement.position.x < 30) {
					return { RIGHT, { movement.position.x + x_movement, y } };
				}
				else {
					return { LEFT, { movement.position.x - x_movement, y } };
				}
			}
		}

		if (movement.direction == LEFT)
		{
			auto newPos = movement.position.x - displacement;
			if (newPos >= 20.0f) {
				return { LEFT, {newPos, movement.position.y} };
			}
			else {
				return { DOWN, {20.0f, movement.position.y + (20.0f - newPos) } };
			}
		}

		return movement;
	}

private:
	float velocity = 10.0f;

	std::vector<int> building_durability{ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
	std::vector<int> building_heights{ 4, 5, 3, 2, 6, 4, 6, 3, 5, 2 };
	
	std::list<bullet> bullets;
	player_movement player_movement{ RIGHT, { 20.0f, 20.0f } };
};

int main()
{
	Game game;
	if (game.Construct(640, 480, 2, 2))
		game.Start();
	return 0;
}