#include <iostream>
#include <list>
#include <thread>

using namespace std;

#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

enum MyEnum
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

struct Body
{
	int x;
	int y;
};

class Snake
{
public:
	Snake()
	{
		snake_body = { {60, 15}, {61, 15}, {62, 15}, {63, 15}, {64, 15} };
		head_icon = '<';
		size = snake_body.size();
		direction = LEFT;
		bDead = false;
	}

	list<Body> snake_body;
	char head_icon;
	int size;
	int direction;
	bool bDead;

	void addPiece();
	void removeBack();
	void updateSize();
	void Move();
	void updateHeadChar();

	~Snake()
	{

	}
};

void Snake::addPiece()
{
	snake_body.push_back({ snake_body.back().x, snake_body.back().y });
}

void Snake::removeBack()
{
	snake_body.pop_back();
}

void Snake::updateSize()
{
	size = snake_body.size();
}

void Snake::Move()
{
	switch (direction)
	{
	case UP: // UP
		snake_body.push_front({ snake_body.front().x, snake_body.front().y - 1 });
		break;
	case RIGHT: // RIGHT
		snake_body.push_front({ snake_body.front().x + 1, snake_body.front().y });
		break;
	case DOWN: // DOWN
		snake_body.push_front({ snake_body.front().x, snake_body.front().y + 1 });
		break;
	case LEFT: // LEFT
		snake_body.push_front({ snake_body.front().x - 1, snake_body.front().y });
		break;
	}
}

void Snake::updateHeadChar()
{
	// Draw snake head
	switch (direction)
	{
	case UP:
		head_icon = bDead ? L'%' : L'^';
		break;
	case RIGHT:
		head_icon = bDead ? L'%' : L'>';
		break;
	case DOWN:
		head_icon = bDead ? L'%' : L'v';
		break;
	case LEFT:
		head_icon = bDead ? L'%' : L'<';
		break;
	}
}

class Food
{
public:
	Food()
	{
		x = 30;
		y = 15;
		icon = '*';
		points = 1;
	}

	int x;
	int y;
	char icon;
	int points;

	void set_position(int x_pos, int y_pos);
	void set_icon(char given_icon);
	void set_points(int given_points);
};

void Food::set_position(int x_pos, int y_pos)
{
	x = x_pos;
	y = y_pos;
}

void Food::set_icon(char given_icon)
{
	icon = given_icon;
}

void Food::set_points(int given_points)
{
	points = given_points;
}

class Vegetable: public Food
{
public:
	Vegetable()
	{
		set_icon('$');
		set_points(5);
	}
};

class Fruit : public Food
{
public:
	Fruit()
	{
		set_icon('*');
		set_points(1);
	}
};

int main()
{
	// Sets Command Prompt window to 120 x 40 so it matches game size
	system("mode 120,40");

	// Screen buffer for graphics
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool playing = true;

	while (playing)
	{
		int nScore = 0;

		Snake* player = new Snake();
		Food* food = new Fruit();

		bool bKeyLeft = false, bKeyRight = false, bKeyUp = false, bKeyDown = false;
		bool bKeyLeftOld = false, bKeyRightOld = false, bKeyUpOld = false, bKeyDownOld = false;;

		while (!player->bDead)
		{
			// Game Tick
			auto t1 = chrono::system_clock::now();
			while ((chrono::system_clock::now() - t1) < ((player->direction == UP || player->direction == DOWN) ? 120ms : 60ms))
			{
				// Player Input
				bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
				bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
				bKeyUp = (0x8000 & GetAsyncKeyState((unsigned char)('\x26'))) != 0;
				bKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x28'))) != 0;

				if (bKeyRight && !bKeyRightOld && (player->direction == UP || player->direction == DOWN))
				{
					player->direction = RIGHT;
				}
				if (bKeyLeft && !bKeyLeftOld && (player->direction == UP || player->direction == DOWN))
				{
					player->direction = LEFT;
				}
				if (bKeyUp && !bKeyUpOld && (player->direction == RIGHT || player->direction == LEFT))
				{
					player->direction = UP;
				}
				if (bKeyDown && !bKeyDownOld && (player->direction == RIGHT || player->direction == LEFT))
				{
					player->direction = DOWN;
				}

				bKeyLeftOld = bKeyLeft;
				bKeyRightOld = bKeyRight;
				bKeyUpOld = bKeyUp;
				bKeyDownOld = bKeyDown;
			}

			// Updating player position
			player->Move();

			// Boundary collision
			if (player->snake_body.front().x < 0 || player->snake_body.front().x >= nScreenWidth)
				player->bDead = true;
			if (player->snake_body.front().y < 3 || player->snake_body.front().y >= nScreenHeight)
				player->bDead = true;

			// Food collision
			if (player->snake_body.front().x == food->x && player->snake_body.front().y == food->y)
			{
				nScore+=food->points;

				for (int i = 0; i < food->points; i++)
				{
					player->addPiece();
				}

				delete food;
				
				int random_food = rand() % 2 + 1;

				if (random_food % 2 == 0)
				{
					food = new Vegetable();
				}
				else
				{
					food = new Fruit();
				}

				// Spawn next food
				while (screen[food->y * nScreenWidth + food->x] != L' ')
				{
					food->set_position(rand() % nScreenWidth, (rand() % (nScreenHeight - 3)) + 3);
				}
			}

			// Collision with self
			for (auto i = player->snake_body.begin(); i != player->snake_body.end(); i++)
			{
				if (i != player->snake_body.begin() && i->x == player->snake_body.front().x && i->y == player->snake_body.front().y)
					player->bDead = true;
			}

			// pops tail and increments player size
			player->removeBack();
			player->updateSize();

			// Clears Screen
			for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
			{
				screen[i] = L' ';
			}

			for (int i = 0; i < nScreenWidth; i++)
			{
				screen[i] = L'=';
				screen[2 * nScreenWidth + i] = L'=';
			}

			wsprintf(&screen[nScreenWidth + 5], L"SCORE: %d     Size: %d     X: %d  Y: %d     Fruits(*): 1 point/length     Vegetables($): 5 points/length", 
				nScore, player->size, player->snake_body.front().x, player->snake_body.front().y - 3);

			// Draw snake
			for (auto s : player->snake_body)
			{
				screen[s.y * nScreenWidth + s.x] = player->bDead ? L'+' : L'O';
			}

			// Draw snake head
			player->updateHeadChar();
			screen[player->snake_body.front().y * nScreenWidth + player->snake_body.front().x] = player->head_icon;

			// Draw food
			screen[food->y * nScreenWidth + food->x] = food->icon;

			if (player->bDead)
				wsprintf(&screen[15 * nScreenWidth + 40], L" Press 'Space' To Retry, 'Escape' to Exit.");

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
		}

		// Wait for space bar press
		while ((0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) == 0)
		{
			bool escapeKey = (0x8000 & GetAsyncKeyState((unsigned char)('\x1B'))) != 0;
			if (escapeKey)
			{
				playing = false;
				break;
			}
		};
		delete player;
		delete food;
	}
}