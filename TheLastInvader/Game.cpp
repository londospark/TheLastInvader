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
			bullets.push_back({ player_movement.position.x + 5, player_movement.position.y + 10 });

		player_movement = MovePlayer(player_movement, modified_velocity * fElapsedTime);

		Clear(olc::BLACK);
		
		// Draw the player
		DrawRect(player_movement.position.x, player_movement.position.y, 10, 10);

		// Filter out old bullets
		bullets.remove_if([](const olc::vf2d& bullet) { return bullet.y > 480.0f; });

		// Draw the bullets
		for (auto& bullet : bullets) {
			DrawLine(bullet.x, bullet.y, bullet.x, bullet.y + 4, olc::RED);
			bullet.y += 350.0f * fElapsedTime;
		}

		// Draw the buildings to destroy

		auto width = ScreenWidth() / building_heights.size();
		auto left = 0;

		for (auto height : building_heights) {
			FillRect(left, 480 - height * 15, width, height * 15, olc::BLUE);
			left += width;
		}

		return true;
	}

	player_movement MovePlayer(player_movement movement, float displacement)
	{
		if (movement.direction == RIGHT)
		{
			float newPos{ movement.position.x + displacement };
			if (newPos <= 620.0f) {
				return { RIGHT, {newPos, movement.position.y} };
			}
			else {
				return { DOWN, {620.0f, movement.position.y + (newPos - 620.0f) } };
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

	std::vector<int> building_heights{ 4, 5, 3, 2, 6, 4, 6, 3, 5, 2 };
	std::list<olc::vf2d> bullets;
	player_movement player_movement{ RIGHT, { 20.0f, 20.0f } };
};

int main()
{
	Game game;
	if (game.Construct(640, 480, 2, 2))
		game.Start();
	return 0;
}