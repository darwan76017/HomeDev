#include <SFML/Graphics.hpp>
#include <time.h>
#include <sstream>
using namespace sf;

int N=30,M=20;
int size=16;  // size of the sprite in pixels
int w = size*N;
int h = size*M;

int num = 40;  // starting length
int maxNum = num;

float TIME_ALOTTED = 30;  // seconds
float TURBO_SPEED = 0.05;
float TURBO_SPEED_DELAY = 0.02;  // takes awhile to kick in after a turn
float BASE_SPEED = 0.09;
int START_X = 10;
int START_Y = 14;

struct Snake 
{ 
	int x,y;
};
Snake snake[200] = { 0 };

struct Fruit
{ 
	int x,y;
} fruit;

enum class Direction
{
	None,
	Left,
	Right,
	Up, 
	Down
} dir;

struct TheSprites
{
	TheSprites(Texture& t1, Texture& t2, Texture& t3)
		: BackgroundSprite(t1)
		, SnakeSprite(t2)
		, FruitSprite(t3)
	{
	}
	Sprite BackgroundSprite;
	Sprite SnakeSprite;
	Sprite FruitSprite;
};

void Tick(float& delay)
 {
	// Update delay(snake speed) based on snake length
	if (num < 10)
		delay = BASE_SPEED;
	else if (num < 35)
		delay = BASE_SPEED + 0.01;
	else if (num < 55)
		delay = BASE_SPEED + 0.02;
	else if (num < 75)
		delay = BASE_SPEED + 0.03;
	else
		delay = BASE_SPEED + 0.04;


	// Update the snake from tail to head
    for (int i=num; i > 0; --i)
	{
		snake[i].x = snake[i-1].x;
		snake[i].y = snake[i-1].y;
	}

	// Update the snakes heead
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
	if ((snake[0].x == fruit.x) && 
		(snake[0].y == fruit.y)) 
    { 
		// grow snake
		int newLength = 5;
		for (int growth = 0; growth < newLength; ++growth)
		{
			snake[num + 1 + growth].x = snake[num].x;
			snake[num + 1 + growth].y = snake[num].y;
		}

		num += newLength;		
		if (num > maxNum)
		{
			maxNum = num;
		}

		bool isSnake = true;
		while (isSnake)
		{
			isSnake = false;
			fruit.x = rand() % N;
			fruit.y = rand() % M;
			for (int i = num; i > 0; --i)
			{
				if ((snake[i].x == fruit.x) &&
					(snake[i].y == fruit.y))
				{
					isSnake = true;
					continue;  // try to place a different fruit location
				}
			}
		}    	
	}

	// wrap around
    if (snake[0].x >= N) snake[0].x=0;  
	if (snake[0].x <  0) snake[0].x=N;
    if (snake[0].y >= M) snake[0].y=0;  
	if (snake[0].y <  0) snake[0].y=M;
 
	for (int i = 1; i < num; i++)
	{
		if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
		{
			num = i; // ran into myself; truncate
		}
	}
}

int GetTimeLeft(Clock& countDownClock)
{
	float elapsedSeconds = countDownClock.getElapsedTime().asSeconds();
	int timeLeft = static_cast<int>(TIME_ALOTTED - elapsedSeconds);
	return timeLeft;
}


void init_snake()
{
	for (int i = num; i >= 0; --i)
	{
		snake[i].x = START_X;
		snake[i].y = START_Y;		
	}
	dir = Direction::Right;
}

void Draw(RenderWindow& window, TheSprites& sprites, Clock& countDownClock)
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

	for (int i = 0; i < num; i++)
	{
		sprites.SnakeSprite.setPosition(snake[i].x*size, snake[i].y*size);
		window.draw(sprites.SnakeSprite);
	}

	sprites.FruitSprite.setPosition(fruit.x*size, fruit.y*size);
	window.draw(sprites.FruitSprite);

	std::string number = std::to_string(num);
	std::string maxNumber = std::to_string(maxNum);

	int timeLeft = GetTimeLeft(countDownClock);
	std::string timeValue = std::to_string(timeLeft);	

	std::string title;
	if (timeLeft <= 0)
	{
		title = "Snake Game!  GAME OVER  Current: " + number + "  Max: " + maxNumber;
	}
	else
	{
		title = "Snake Game!  Time: " + timeValue + "  Current: " + number + "  Max: " + maxNumber;
	}
	
	window.setTitle(title.c_str());
	window.display();
}

void GetInput(RenderWindow& window, Direction& dir, bool& isTurbo, Clock& turboClock)
{
	Event e;
	while (window.pollEvent(e))
	{
		if (e.type == Event::Closed)
		{
			window.close();
		}
	}

	isTurbo = false;
	Direction desiredDir = Direction::None;
	Direction oppositeDir = Direction::None;
	if (Keyboard::isKeyPressed(Keyboard::Left))
	{
		desiredDir = Direction::Left;
		oppositeDir = Direction::Right;
	}
	if (Keyboard::isKeyPressed(Keyboard::Right))
	{
		desiredDir = Direction::Right;
		oppositeDir = Direction::Left;
	}
	if (Keyboard::isKeyPressed(Keyboard::Up))
	{
		desiredDir = Direction::Up;
		oppositeDir = Direction::Down;
	}
	if (Keyboard::isKeyPressed(Keyboard::Down))
	{
		desiredDir = Direction::Down;
		oppositeDir = Direction::Up;
	}

	if (desiredDir != Direction::None)
	{		
		if (desiredDir != dir)
		{
			if (dir != oppositeDir)
			{
				dir = desiredDir;
				turboClock.restart();  // change direction; back to normal speed
			}
			// else ignore it if they desire the opposite direction
		}
		else
		{
			// Desire the same direction, is it time for turbo?
			float time = turboClock.getElapsedTime().asSeconds();
			isTurbo = (time > TURBO_SPEED_DELAY);
		}
	}
}

int main()
{  
	srand(time(0));

    RenderWindow window(VideoMode(w, h), "Snake Game!");

	Texture t1,t2,t3;
	t1.loadFromFile("images/white.png");
	t2.loadFromFile("images/red.png");
	t3.loadFromFile("images/green.png");

	TheSprites sprites(t1, t2, t3);	

	Clock clock, countDownClock, turboClock;
    float timer=0, delay=0.10;

	fruit.x = 10;
    fruit.y = 10;

	init_snake();
	countDownClock.restart();
	bool isTurbo = false;
	
    while (window.isOpen())
    {
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

        timer += time;       

		GetInput(window, dir, isTurbo, turboClock);

		int timeLeft = GetTimeLeft(countDownClock);
		if (timeLeft > 0)
		{
			if ( (isTurbo && (timer > (delay - TURBO_SPEED))) ||
				 (timer > delay))
			{
				timer = 0;				
				Tick(delay);
			}
		}

		Draw(window, sprites, countDownClock);								
	}

    return 0;
}