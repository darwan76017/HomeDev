#include <SFML/Graphics.hpp>
#include <time.h>
#include <sstream>

using namespace sf;

const int N = 60, M = 40;
const int size=16;  // size of the sprite in pixels
const int windowWidth = size*N;
const int windowHeight = size*M;

const float TIME_ALOTTED = 35;  // seconds
const float TURBO_SPEED = 0.05;
const float TURBO_SPEED_DELAY = 0.02;  // takes awhile to kick in after a turn
const float BASE_SPEED = 0.09;

const int START_SNAKE_LEN = 40;  // should be 199 or less
const int GROW_LENGTH = 5;
const int MAX_SNAKE_LEN = 500;
const int START_X = 10;
const int START_Y = 14;
const int START_HOW_LONG_LEN = 15;
const int NUM_FRUIT = 5;
const int MAX_ROCK = 100;
const int NUM_ROCK = 100;
const int GROUP_ROCKS_NUM = 12;

struct MapObject
{ 
	int x,y;
};
MapObject snake[MAX_SNAKE_LEN] = { 0 };
MapObject fruit[NUM_FRUIT] = { 0 };
MapObject rock[MAX_ROCK] = { 0 };

enum class Direction
{
	None,
	Left,
	Right,
	Up, 
	Down
};

enum class GameMode
{
	Countdown,	// EdgeWrap ok, hit self truncate, limited time
	HowLong		// EdgeWrap ok, hit self truncate, unlimited time
};

struct TheSprites
{
	TheSprites(Texture& t1, Texture& t2, Texture& t3, Texture& t4)
		: BackgroundSprite(t1)
		, SnakeSprite(t2)
		, FruitSprite(t3)
		, RockSprite(t4)
	{
	}
	Sprite BackgroundSprite;
	Sprite SnakeSprite;
	Sprite FruitSprite;
	Sprite RockSprite;
};

class GameStats
{
public:
	GameStats(int currentLen) : m_Len(currentLen), m_MaxLen(currentLen), m_Mode(GameMode::Countdown), m_HasHitSelf(false),
								m_IsTurbo(false), m_IsReset(false), m_ShouldTruncate(false), m_ShouldCreateRocks(true), m_NumRocks(NUM_ROCK),
								m_HitRock(false)
	{ }

	void SetGameMode(GameMode mode) { m_Mode = mode; }

	int GetCurrentLength() { return m_Len;  }
	void SetCurrentLength(int len) { m_Len = len; }

	int GetMaxLength() { return m_MaxLen; }
	void SetMaxLength(int len) { m_MaxLen = len; }

	int GetNumRocks() { return m_NumRocks; }
	void SetNumRocks(int num) { m_NumRocks = num; }

	int IsTurbo() { return m_IsTurbo; }
	void SetTurbo(bool val) { m_IsTurbo = val; }

	int IsReset() { return m_IsReset; }
	void SetReset(bool val) { m_IsReset = val; }

	Direction GetSnakeDirection() { return m_SnakeDir; }
	void SetSnakeDirection(Direction dir) { m_SnakeDir = dir; }	

	void SetHasHitSelf() { m_HasHitSelf = true; }
	void SetHitRock() { m_HitRock = true; }
	bool ShouldTruncate() { return m_ShouldTruncate; }
	bool ShouldCreateRocks() { return m_ShouldCreateRocks;  }

	void ResetCountdownClock() { m_CountDownClock.restart(); }

	int GetTimeLeft()
	{
		float elapsedSeconds = m_CountDownClock.getElapsedTime().asSeconds();
		int timeLeft = static_cast<int>(TIME_ALOTTED - elapsedSeconds);
		return timeLeft;
	}

	void GrowByLength(int len)
	{
		m_Len += len;
		if (m_Len > m_MaxLen)
		{
			m_MaxLen = m_Len;
		}
	}

	std::string GetTitle()
	{
		std::string number = std::to_string(m_Len);
		std::string maxNumber = std::to_string(m_MaxLen);

		int timeLeft = GetTimeLeft();
		std::string timeValue = std::to_string(timeLeft);

		std::string prefix, title;

		switch (m_Mode)
		{
		case GameMode::Countdown:
			prefix = "Snake Race!! ";
			if (IsGameOver())
			{
				title = prefix + "GAME OVER  Current: " + number + "  Max: " + maxNumber;
			}
			else
			{
				title = prefix + " Time: " + timeValue + "  Current: " + number + "  Max: " + maxNumber;
			}
			break;
		case GameMode::HowLong:			
			prefix = "Snake Grow!! ";
			if (IsGameOver())
			{
				title = prefix + "GAME OVER  Current: " + number + "  Max: " + maxNumber;
			}
			else
			{
				title = prefix + " Current: " + number + "  Max: " + maxNumber;
			}			
			break;						
		}
		
		return title;
	}

	bool IsGameOver()
	{
		switch (m_Mode)
		{
		case GameMode::Countdown:
			if ((GetTimeLeft() <= 0) || m_HitRock)
				return true;
			break;
		case GameMode::HowLong:
			return (m_HasHitSelf | m_HitRock);
		}

		return false;
	}

	void Reset()
	{		
		m_SnakeDir = Direction::Right;
		m_IsTurbo = false;
		m_IsReset = false;
		m_HitRock = false;

		switch (m_Mode)
		{
		case GameMode::Countdown:
			// keep current length
			ResetCountdownClock();
			m_ShouldTruncate = true;
			break;

		case GameMode::HowLong:
			m_Len = START_HOW_LONG_LEN;
			m_HasHitSelf = false;
			m_ShouldTruncate = false;
			break;
		}		
	}

private:
	int m_Len;
	int m_MaxLen;
	int m_NumRocks;

	Clock m_CountDownClock;

	GameMode m_Mode;

	bool m_HasHitSelf;
	bool m_HitRock;

	Direction m_SnakeDir;
	bool m_IsTurbo; 
	bool m_IsReset;
	bool m_ShouldTruncate;
	bool m_ShouldCreateRocks;
};

bool IsFruitLocation(int x, int y, int& fruitIndex)
{
	for (int i = 0; i < NUM_FRUIT; ++i)
	{
		if (x == fruit[i].x &&
			y == fruit[i].y)
		{
			fruitIndex = i;
			return true;
		}
	}
	return false;
}

void CreateAFruit(int fruitIndex, int snakeLen)
{
	bool isConflict = true;
	while (isConflict)
	{
		isConflict = false;
		fruit[fruitIndex].x = rand() % N;
		fruit[fruitIndex].y = rand() % M;
		for (int i = snakeLen; i > 0; --i)
		{
			if ((snake[i].x == fruit[fruitIndex].x) &&
				(snake[i].y == fruit[fruitIndex].y))
			{
				isConflict = true;
				continue;  // try to place a different fruit location
			}
		}

		for (int i = 0; i < NUM_FRUIT; i++)
		{
			if (i == fruitIndex)
			{
				continue;
			}
			if ((fruit[i].x == fruit[fruitIndex].x) &&
				(fruit[i].y == fruit[fruitIndex].y))
			{
				isConflict = true;
				continue;  // try to place a different fruit location
			}
		}
	}
}

bool FindARock(int x, int y, int numRocks)
{
	for (int i = 0; i < numRocks; i++)
	{
		if ((rock[i].x == x) &&
			(rock[i].y == y))
		{
			return true;
		}
	}
	return false;
}

void CreateARock(int rockIndex, int snakeLen, int numRocks)
{
	bool isConflict = true;
	while (isConflict)
	{
		isConflict = false;
		rock[rockIndex].x = rand() % N;
		rock[rockIndex].y = rand() % M;
		for (int i = snakeLen; i > 0; --i)
		{
			if ((snake[i].x == rock[rockIndex].x) &&
				(snake[i].y == rock[rockIndex].y))
			{
				isConflict = true;
				continue;  // try to place a different rock location
			}
		}

		if (FindARock(rock[rockIndex].x, rock[rockIndex].y, rockIndex))
		{		
			isConflict = true;
			continue;  // try to place a different fruit location		
		}

		if (rockIndex > GROUP_ROCKS_NUM)
		{
			// find neighbors
			for (int j = -1; j < 2; j++)
				for (int k = -1; k < 2; k++)
				{
					if (j == 0 && k == 0)
					{
						continue;
					}

					int findX = rock[rockIndex].x + j;
					int findY = rock[rockIndex].y + k;

					if (findX < 0 || findX >= M) continue;
					if (findY < 0 || findY >= M) continue;

					if (FindARock(findX, findY, rockIndex))
					{
						return; // good; don't want the new rock to be lonely
					}
				}

			isConflict = true; // don't want new rocks by themselves, find a buddy!
		}
	}
}



void CreateAllFruit(int snakeLen)
{
	for (int i = 0; i < NUM_FRUIT; i++)
	{
		CreateAFruit(i, snakeLen);
	}
}

void CreateAllRocks(int snakeLen, int numRocks)
{
	for (int i = 0; i < numRocks; i++)
	{
		CreateARock(i, snakeLen, numRocks);
	}
}

bool IsRockLocation(int x, int y, int numRocks)
{
	for (int i = 0; i < numRocks; i++)
	{
		if (x == rock[i].x &&
			y == rock[i].y)
		{			
			return true;
		}
	}
	return false;
}


void Tick(float& delay, GameStats& gameStats)
 {	
	int snakeLen = gameStats.GetCurrentLength();

	// Update delay(snake speed) based on snake length
	if (snakeLen < 10)
		delay = BASE_SPEED;
	else if (snakeLen < 35)
		delay = BASE_SPEED + 0.01;
	else if (snakeLen < 55)
		delay = BASE_SPEED + 0.02;
	else if (snakeLen < 75)
		delay = BASE_SPEED + 0.03;
	else
		delay = BASE_SPEED + 0.04;


	// Update the snake from tail to head
    for (int i= snakeLen; i > 0; --i)
	{
		snake[i].x = snake[i-1].x;
		snake[i].y = snake[i-1].y;
	}

	// Update the snakes heead
	Direction dir = gameStats.GetSnakeDirection();
	if (dir == Direction::Down)
	{
		snake[0].y += 1;
	}
	if (dir == Direction::Left)
	{
		snake[0].x -= 1;
	}
	if (dir == Direction::Right)
	{
		snake[0].x += 1;
	}
	if (dir == Direction::Up)
	{
		snake[0].y -= 1;
	}

	// Eat the fruit !!
	int fruitIndex = 0;
	if (IsFruitLocation(snake[0].x, snake[0].y, fruitIndex))
    { 
		// grow snake		
		for (int growth = 0; growth < GROW_LENGTH; ++growth)
		{
			snake[snakeLen + 1 + growth].x = snake[snakeLen].x;
			snake[snakeLen + 1 + growth].y = snake[snakeLen].y;
		}

		gameStats.GrowByLength(GROW_LENGTH);		
		CreateAFruit(fruitIndex, gameStats.GetCurrentLength());
	}

	if (IsRockLocation(snake[0].x, snake[0].y, gameStats.GetNumRocks()))
	{
		gameStats.SetHitRock();
	}

	// wrap around
    if (snake[0].x >= N) snake[0].x=0;  
	if (snake[0].x <  0) snake[0].x=N;
    if (snake[0].y >= M) snake[0].y=0;  
	if (snake[0].y <  0) snake[0].y=M;
 
	// Did the snake run into itself?
	for (int i = 1; i < snakeLen; i++)
	{
		if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
		{			
			gameStats.SetHasHitSelf();

			if (gameStats.ShouldTruncate())
			{
				gameStats.SetCurrentLength(i);  // ran into myself; truncate
			}			
		}
	}
}

void InitSnake(int snakeLen)
{
	for (int i = snakeLen; i >= 0; --i)
	{
		snake[i].x = START_X;
		snake[i].y = START_Y;		
	}	
}

void Draw(RenderWindow& window, TheSprites& sprites, GameStats gameStats)
{	
	window.clear();

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			sprites.BackgroundSprite.setPosition(i*size, j*size);
			window.draw(sprites.BackgroundSprite);
		}
	}

	for (int i = 0; i < gameStats.GetCurrentLength(); i++)
	{
		sprites.SnakeSprite.setPosition(snake[i].x*size, snake[i].y*size);
		window.draw(sprites.SnakeSprite);
	}

	for (int i = 0; i < NUM_FRUIT; i++)
	{
		sprites.FruitSprite.setPosition(fruit[i].x*size, fruit[i].y*size);
		window.draw(sprites.FruitSprite);
	}

	for (int i = 0; i < gameStats.GetNumRocks(); i++)
	{
		sprites.RockSprite.setPosition(rock[i].x*size, rock[i].y*size);
		window.draw(sprites.RockSprite);
	}
		
	window.setTitle(gameStats.GetTitle().c_str());
	window.display();
}

void GetInput(RenderWindow& window, Clock& turboClock, GameStats& gameStats)
{
	Event e;
	while (window.pollEvent(e))
	{
		if (e.type == Event::Closed)
		{
			window.close();
		}
	}

	gameStats.SetTurbo(false);
	Direction desiredDir = Direction::None;
	Direction oppositeDir = Direction::None;
	if (Keyboard::isKeyPressed(Keyboard::Left) || 
		Keyboard::isKeyPressed(Keyboard::A) )
	{
		desiredDir = Direction::Left;
		oppositeDir = Direction::Right;
	}
	if (Keyboard::isKeyPressed(Keyboard::Right) ||
		Keyboard::isKeyPressed(Keyboard::D))
	{
		desiredDir = Direction::Right;
		oppositeDir = Direction::Left;
	}
	if (Keyboard::isKeyPressed(Keyboard::Up) ||
		Keyboard::isKeyPressed(Keyboard::W))
	{
		desiredDir = Direction::Up;
		oppositeDir = Direction::Down;
	}
	if (Keyboard::isKeyPressed(Keyboard::Down) ||
		Keyboard::isKeyPressed(Keyboard::S))
	{
		desiredDir = Direction::Down;
		oppositeDir = Direction::Up;
	}

	if (Keyboard::isKeyPressed(Keyboard::Space))
	{
		gameStats.SetReset(true);
	}

	if (Keyboard::isKeyPressed(Keyboard::Num1))
	{
		if (gameStats.IsGameOver())
		{
			gameStats.SetGameMode(GameMode::Countdown);
		}		
	}

	if (Keyboard::isKeyPressed(Keyboard::Num2))
	{
		if (gameStats.IsGameOver())
		{
			gameStats.SetGameMode(GameMode::HowLong);
		}
	}

	Direction dir = gameStats.GetSnakeDirection();
	if (desiredDir != Direction::None)
	{		
		if (desiredDir != dir)
		{
			if (dir != oppositeDir)
			{				
				gameStats.SetSnakeDirection(desiredDir);
				turboClock.restart();  // change direction; back to normal speed
			}
			// else ignore it if they desire the opposite direction
		}
		else
		{
			// Desire the same direction, is it time for turbo?
			float time = turboClock.getElapsedTime().asSeconds();
			gameStats.SetTurbo(time > TURBO_SPEED_DELAY);
		}
	}
}

void SetupGame(GameStats& gameStats)
{		
	gameStats.Reset();
	InitSnake(gameStats.GetCurrentLength());
	CreateAllFruit(gameStats.GetCurrentLength());
	if (gameStats.ShouldCreateRocks())
	{
		CreateAllRocks(gameStats.GetCurrentLength(), gameStats.GetNumRocks());
	}
}

int main()
{  
	srand(time(0));

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Snake Game!");

	Texture t1, t2, t3, t4;
	t1.loadFromFile("images/white.png");
	t2.loadFromFile("images/green.png");
	t3.loadFromFile("images/red.png");	
	t4.loadFromFile("images/rock.png");

	TheSprites sprites(t1, t2, t3, t4);	

	Clock clock, turboClock;    
	float timer=0, delay=0.10;	

	GameStats gameStats(START_SNAKE_LEN);		
	SetupGame(gameStats);
	
    while (window.isOpen())
    {
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

        timer += time;       

		GetInput(window, turboClock, gameStats);
		
		if (!gameStats.IsGameOver())
		{
			if ( (gameStats.IsTurbo() && (timer > (delay - TURBO_SPEED))) ||
				 (timer > delay))
			{
				timer = 0;				
				Tick(delay, gameStats);
			}
		}

		if (gameStats.IsReset())
		{			
			SetupGame(gameStats);
		}

		Draw(window, sprites, gameStats);
	}

    return 0;
}