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
		const float acceleration = 8.0f;

		if (GetKey(olc::Q).bHeld)
			velocity += acceleration * fElapsedTime;
		if (GetKey(olc::A).bHeld)
			velocity -= acceleration * fElapsedTime;

		if (GetKey(olc::SPACE).bPressed)
			bullets.push_back({ player_movement.position.x + 5, player_movement.position.y + 10 });

		if (velocity <= 7.0f) velocity = 7.0f;
		if (velocity <= 13.0f) velocity = 13.0f;

		player_movement = MovePlayer(player_movement, velocity * fElapsedTime);

		Clear(olc::BLACK);
		DrawRect(player_movement.position.x, player_movement.position.y, 10, 10);

		bullets.remove_if([](const olc::vf2d& bullet) { return bullet.y > 480.0f; });

		for (auto& bullet : bullets) {
			DrawLine(bullet.x, bullet.y, bullet.x, bullet.y + 4, olc::RED);
			bullet.y += 350.0f * fElapsedTime;
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