
#include <iostream>
#include <deque>
#include <raylib.h>
#include <raymath.h>
using namespace std;

Color green = {121, 145, 77, 255};
Color darkgreen = {43, 51, 24, 255};

int cellSize = 25;
int cellCount = 20;
int offset = 75;
double lastUpdatetime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();

    if (currentTime - lastUpdatetime >= interval)
    {
        lastUpdatetime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0}; // Moving the snake

    bool addSegment = false; // To add a segment to the snake when it eats the food.

    void draw()
    {
        for (unsigned int i = 0; i < body.size(); i++) // deque.size method returns an unsigned int so we add unsigned before
        {
            int x = body[i].x;
            int y = body[i].y;
            // DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, darkgreen);
            Rectangle segment = {
                static_cast<float>(offset + x * cellSize), // static_cast<float> was used to type cast the int data type to float during run time.
                static_cast<float>(offset + y * cellSize),
                static_cast<float>(cellSize),
                static_cast<float>(cellSize)};
            DrawRectangleRounded(segment, 0.5f, 6, darkgreen);
        }
    }

    void update() // Remove one cell from the end and then add another to the beginning.
    {
        body.push_front(Vector2Add(body[0], direction));
        if(addSegment)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture; // Created a Texture2D attribute in our class to hold the texture data.

    Food(deque<Vector2> snakeBody) // Constructor
    {
        Image image = LoadImage("Graphics/food.png"); // We made a constructor to load the image when an object is created.
        texture = LoadTextureFromImage(image);
        UnloadImage(image);             // Unloads the image for some free memory.
        position = GenerateRandomPos(snakeBody); // Call the GenerateRandomPos method.
    }

    ~Food() // Destructor
    {
        UnloadTexture(texture);
    }

    void draw()
    {
        // Draw food texture slightly smaller than the cell size
        DrawTexture(texture, 
            offset + position.x * cellSize + (cellSize - 15) / 2,  // Center horizontally
            offset + position.y * cellSize + (cellSize - 15) / 2,  // Center vertically
            WHITE); // Since we loaded a texture we can use it to display on the screen.
    }

    Vector2 GenerateRandomCell()
    {
        // Generate random position within game grid boundaries
        int maxX = cellCount - 2;  // Subtract 2 to leave 1 cell gap on each side
        int maxY = cellCount - 2;
        float x = GetRandomValue(1, maxX);
        float y = GetRandomValue(1, maxY);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) // This method will generate a random position in the grid.
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
           position = GenerateRandomCell();
        }
        return position;
    }
};

class Game // A game class ia made so that code can be more organized. 
{
    public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;

    Sound eatSound;
    Sound wallSound;

    Game()
    {
        InitAudioDevice();  
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }


    void Draw()
    {
        snake.draw();
        food.draw();
    }

    void Update()
    {
        if(running) 
        {
            snake.update();
            checkCollisionWithFood();
            checkCollisionWithWall();
            checkCollisionWithSelf();
        }
    }

    void checkCollisionWithFood()
    {
        if(Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void checkCollisionWithWall()
    {
        // Check if snake is at the actual edge of the grid
        if(snake.body[0].x < 0 || snake.body[0].x >= cellCount)
        {
            GameOver();
            PlaySound(wallSound);
        }
        if(snake.body[0].y < 0 || snake.body[0].y >= cellCount)
        {
            GameOver();
            PlaySound(wallSound);
        }
    }

    void checkCollisionWithSelf()
    {
        deque<Vector2> snakeBody = snake.body;
        snakeBody.pop_front();
        if(ElementInDeque(snake.body[0], snakeBody))
        {
            GameOver();
        }
    }
    
    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
    }
};

int main()
{
    cout << "Starting The Game" << endl;
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        if(eventTriggered(0.2))
        {
            game.Update();
        }

        if(IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) // snake.direction.y != 1 is used 
                                                           // to prevent the snake from moving in the opposite direction.
        {
            game.snake.direction = {0, -1};
            game.running=true;
        }

        if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) 
        {
            game.snake.direction = {0, 1};
            game.running=true;
        }

        if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) 
        {
            game.snake.direction = {-1, 0};
            game.running=true;
        }

        if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) 
        {
            game.snake.direction = {1, 0};
            game.running=true;
        }

        // Drawing
        ClearBackground({121, 164, 132, 255});
        DrawRectangleLinesEx(Rectangle{(float)(offset-5), (float)(offset-5), (float)(cellSize * cellCount), (float)(cellSize * cellCount)}, 5, darkgreen);

        DrawText("Retro Snake ", offset-5, 20, 40, darkgreen);
        DrawText(TextFormat("%i", game.score), offset-5, offset+cellSize*cellCount+10, 40, darkgreen);

        game.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}