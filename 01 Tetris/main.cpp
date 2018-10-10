//
//  TETRIS
//
// TODO: Detect end game, keep score
//
// Added controls: 
//		P = Pause
//		N = New Game
//		W = Faster
//		S = Slower
//
//		^ = Rotate
//		< = Left
//		> = Right
//		V = Faster drop, only while held down
//

#include <SFML/Graphics.hpp>
#include <time.h>
using namespace sf;

const int M = 20; // height
const int N = 10;  // width

float defaultDelay = 0.5;  // Was 0.3
bool isPaused = false;
bool isNewGame = true;

int field[M][N] = {0};

struct Point
{
	int x,y;
} a[4], b[4];

int figures[7][4] =
{
	1,3,5,7, // I
	2,4,5,7, // Z
	3,5,4,6, // S
	3,5,4,7, // T
	2,3,5,7, // L
	3,5,7,6, // J
	2,3,4,5, // O
};

bool IsCollision()
{
	for (int i=0;i<4;i++)
		if (a[i].x <  0 ||   // Can't move left of the edge
			a[i].x >= N ||   // Can't move right past the edge
			a[i].y >= M)     // Can't fall below the bottom line of the field
		{
			return false;
		}
		else if (field[a[i].y][a[i].x])
		{
			// Ran into a block on the field
			return false;
		}

   return true;
}

int GenerateNewPiece()
{
	int colorNum = 1 + rand() % 7;
	int n = rand() % 7;
	int offsetX = rand() % (N - 1);
	for (int i = 0; i < 4; i++)
	{
		a[i].x = figures[n][i] % 2 + offsetX;
		a[i].y = figures[n][i] / 2;
	}

	return colorNum;
}


int main()
{
    srand(time(0));	 

	RenderWindow window(VideoMode(320, 480), "Dad's Tetris Game!!!");

    Texture t1,t2,t3;
	t1.loadFromFile("images/tiles.png");
	t2.loadFromFile("images/background.png");
	t3.loadFromFile("images/frame.png");

	Sprite s(t1), background(t2), frame(t3);

    int dx=0; bool rotate=false; int colorNum=1;
	float timer = 0, delay = defaultDelay;
	
	Clock clock;

    while (window.isOpen())
    {
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();
		timer+=time;

        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

			if (e.type == Event::KeyPressed)
			{
				switch (e.key.code)
				{
				case Keyboard::Up:
					rotate = true;
					break;
				case Keyboard::Left:
					dx = -1;
					break;
				case Keyboard::Right:
					dx = 1;
					break;
				case Keyboard::W:
					defaultDelay -= 0.1;
					break;
				case Keyboard::S:
					defaultDelay += 0.1;
					break;
				case Keyboard::P:
					isPaused = !isPaused;
					break;
				case Keyboard::N:
					isNewGame= true;
					break;

				}				
			}			  
		}

		if (Keyboard::isKeyPressed(Keyboard::Down))
		{
			delay = 0.05;
		}

		if (defaultDelay < 0.05)
		{
			defaultDelay = 0.05;
		}
		if (defaultDelay > 2.0)
		{
			defaultDelay = 2.0;
		}

		if (isNewGame)
		{
			for (int i = 0; i < M; i++)
				for (int j = 0; j < N; j++)
				{
					field[i][j] = 0;
				}
			for (int i = 0; i < 4; i++)
			{
				a[i].x = b[i].x = 0;
				a[i].y = b[i].y = 0;
			}
			colorNum = GenerateNewPiece();
			isNewGame = false;
		}

	//// <- Move -> ///
	for (int i=0;i<4;i++)  
	{ 
		b[i]=a[i]; a[i].x += dx; 
	}

    if (!IsCollision()) 
		for (int i = 0; i < 4; i++)
		{
			a[i] = b[i];
		}

	//////Rotate//////
	if (rotate)
	{
		Point p = a[1]; //center of rotation
		for (int i=0;i<4;i++)
		{
			int x = a[i].y-p.y;
			int y = a[i].x-p.x;
			a[i].x = p.x - x;
			a[i].y = p.y + y;
	 	}
   		if (!IsCollision()) 
			for (int i = 0; i < 4; i++)
			{
				a[i] = b[i];
			}
	}

	///////Tick//////
	if (!isPaused && (timer > delay))
	{
		// Move the piece down
	    for (int i=0;i<4;i++) 
		{ 
			b[i]=a[i]; 
			a[i].y += 1; 
		}

		if (!IsCollision())
		{
			//update the field with the old location of the piece
			for (int i = 0; i < 4; i++)
			{				
				field[b[i].y][b[i].x] = colorNum;
			}

			// new piece
			colorNum = GenerateNewPiece();
		}

	  	timer = 0;
	}

	// Check for lines to remove
    int k=M-1;
	for (int i=M-1; i>0; i--)
	{
		int count=0;
		for (int j=0;j<N;j++)
		{
			if (field[i][j])
			{
				count++;
			}
		    field[k][j] = field[i][j];
		}
		//if (count < N)
		if (count < (N-1)) // almost all squares is close enough!
		{
			k--;
		}
	}

	// Reset state for deltaX, rotate, and delay
    dx=0; rotate=0; delay = defaultDelay;

    /////////draw//////////
    window.clear(Color::White);	
    window.draw(background);
		  
	for (int i=0;i<M;i++)
	 for (int j=0;j<N;j++)
	   {
		 if (field[i][j] == 0)
		 {
			 continue;  // no tile to draw
		 }

		 // draw field
		 s.setTextureRect(IntRect(field[i][j]*18,0,18,18));
		 s.setPosition(j*18,i*18);
		 s.move(28,31); //offset
		 window.draw(s);
	   }

	// Draw current piece
	for (int i=0;i<4;i++)
	{
		s.setTextureRect(IntRect(colorNum*18,0,18,18));
		s.setPosition(a[i].x*18,a[i].y*18);
		s.move(28,31); //offset
		window.draw(s);
	}

	window.draw(frame);
 	window.display();
	}

    return 0;
}