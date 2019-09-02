#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <list>

const int starting_durability = 10;

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
	bool beneath_screen;
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

		float building_width = ScreenWidth() / buildings.size();
		std::vector<bool> building_collisions{ false, false, false, false, false, false, false, false, false, false };

		// Collision of bullets and buildings
		for (auto& bullet : bullets) {
			auto index = std::floor(bullet.position.x / building_width);
			auto building_height = buildings[index].height * block_height;

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
				buildings[i].durability--;
				if (buildings[i].durability == 0) {
					buildings[i].height--;
					buildings[i].durability = starting_durability;
					buildings[i].desctruction_lines = {};
				}
				else {
					float left = i * building_width;
					float right = (i + 1) * building_width;
					float top = ScreenHeight() - (buildings[i].height * block_height);
					float bottom = top + block_height;

					float x1 = left + (right - left) * normalised_random();
					float x2 = left + (right - left) * normalised_random();

					float y1 = top + (bottom - top) * normalised_random();
					float y2 = top + (bottom - top) * normalised_random();
					buildings[i].desctruction_lines.push_back({ x1, y1, x2, y2 });
				}
			}
		}

		// Draw the buildings to destroy
		auto left = 0;

		for (auto building : buildings) {
			FillRect(left, ScreenHeight() - building.height * block_height, building_width, building.height * block_height, olc::GREEN);

			for (auto line : building.desctruction_lines) {
				DrawLine(line.x1, line.y1, line.x2, line.y2, olc::BLACK);
			}

			left += building_width;
		}

		// Filter out old bullets
		bullets.remove_if([](const Bullet& bullet) { return bullet.collided || bullet.beneath_screen; });

		// Draw the bullets
		for (auto& bullet : bullets) {
			DrawLine(bullet.position.x, bullet.position.y, bullet.position.x, bullet.position.y + 4, olc::GREEN);
			bullet.position.y += 350.0f * fElapsedTime;
		}

		return true;
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

private:
	float velocity = 100.0f;

	const int scale = 2;
	const int base_width = 10;
	const int base_height = 10;

	const float margin = 20;
	const int layer_height = 20;

	const int player_width = base_width * scale;
	const int player_height = base_height * scale;

	const int block_height = 15;

	std::vector<Building> buildings{ 4, 5, 3, 2, 6, 4, 6, 3, 5, 2 };

	std::list<Bullet> bullets;
	PlayerMovement player_movement{ PlayerMovement::Direction::RIGHT, { 20.0f, 20.0f } };

	olc::Sprite* player_sprite;
};

int main()
{
	Game game;
	if (game.Construct(640, 480, 2, 2))
		game.Start();
	return 0;
}