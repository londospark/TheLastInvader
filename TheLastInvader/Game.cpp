#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <list>

//TODO(gareth): Is building really an object? Should blocks be objects, that way they can contain the destruction lines in them.

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

struct line {
	float x1, y1, x2, y2;
};

float normalised_random() {
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

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
		srand(time(0));
		player_sprite = new olc::Sprite("Invader.png");
		return true;
	}

	bool OnUserDestroy() override {
		delete player_sprite;
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
			bullets.push_back({ { player_movement.position.x + 0.5f * player_width, player_movement.position.y + player_height }, false, false });

		player_movement = MovePlayer(player_movement, modified_velocity * fElapsedTime);

		Clear(olc::BLACK);
		
		// Draw the player
		//DrawRect(player_movement.position.x, player_movement.position.y, 10, 10);
		DrawSprite(player_movement.position.x, player_movement.position.y, player_sprite, scale);

		float building_width = ScreenWidth() / building_heights.size();
		std::vector<bool> building_collisions{ false, false, false, false, false, false, false, false, false, false };

		// Collision of bullets and buildings
		for (auto& bullet : bullets) {
			auto index = std::floor(bullet.position.x / building_width);
			auto building_height = building_heights[index] * block_height;

			if (building_height == 0 && bullet.position.y > ScreenHeight()) {
				bullet.beneath_screen = true;
			}

			if (bullet.position.y + 4 >= ScreenHeight() - building_height) {
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
			{
				building_durability[i]--;
				if (building_durability[i] == 0) {
					building_heights[i]--;
					building_durability[i] = starting_durability;
				}
				else {
					float left = i * building_width;
					float right = (i + 1) * building_width;
					float top = ScreenHeight() - (building_heights[i] * block_height);
					float bottom = top + block_height;

					float x1 = left + (right - left) * normalised_random();
					float x2 = left + (right - left) * normalised_random();

					float y1 = top + (bottom - top) * normalised_random();
					float y2 = top + (bottom - top) * normalised_random();
					desctruction_lines.push_back({ x1, y1, x2, y2 });
				}
			}
		}

		// Draw the buildings to destroy
		auto left = 0;

		for (auto height : building_heights) {
			FillRect(left, ScreenHeight() - height * block_height, building_width, height * block_height, olc::GREEN);
			left += building_width;
		}

		for (auto line : desctruction_lines) {
			DrawLine(line.x1, line.y1, line.x2, line.y2, olc::BLACK);
		}

		// Filter out old bullets
		bullets.remove_if([](const bullet& bullet) { return bullet.collided || bullet.beneath_screen; });

		// Draw the bullets
		for (auto& bullet : bullets) {
			DrawLine(bullet.position.x, bullet.position.y, bullet.position.x, bullet.position.y + 4, olc::GREEN);
			bullet.position.y += 350.0f * fElapsedTime;
		}

		return true;
	}

	player_movement MovePlayer(player_movement movement, float displacement)
	{
		if (movement.direction == RIGHT)
		{
			float right_boundary = ScreenWidth() - margin - player_width;
			float newPos{ movement.position.x + displacement };
			if (newPos <= right_boundary) {
				return { RIGHT, {newPos, movement.position.y} };
			}
			else {
				return { DOWN, {right_boundary, movement.position.y + (newPos - right_boundary) } };
			}
		}

		if (movement.direction == DOWN)
		{
			auto current_layer = std::floor(movement.position.y / layer_height);
			auto new_layer = std::floor((movement.position.y + displacement) / layer_height);
			
			if (current_layer == new_layer) {
				return { DOWN, {movement.position.x, movement.position.y + displacement} };
			}
			else {
				auto y = new_layer * layer_height;
				auto x_movement = movement.position.y + displacement - y;
				if (movement.position.x <= margin) {
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
			if (newPos >= margin) {
				return { LEFT, {newPos, movement.position.y} };
			}
			else {
				return { DOWN, {margin, movement.position.y + (margin - newPos) } };
			}
		}

		return movement;
	}

private:
	float velocity = 100.0f;

	const int scale = 2;
	const int base_width = 10;
	const int base_height = 10;

	const float margin = 20;
	const int layer_height = 20;

	const int starting_durability = 10;

	const int player_width = base_width * scale;
	const int player_height = base_height * scale;

	const int block_height = 15;

	std::vector<int> building_durability{
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability,
		starting_durability };
	std::vector<int> building_heights{ 4, 5, 3, 2, 6, 4, 6, 3, 5, 2 };
	
	std::vector<line> desctruction_lines;

	std::list<bullet> bullets;
	player_movement player_movement{ RIGHT, { 20.0f, 20.0f } };

	olc::Sprite* player_sprite;
};

int main()
{
	Game game;
	if (game.Construct(640, 480, 2, 2))
		game.Start();
	return 0;
}