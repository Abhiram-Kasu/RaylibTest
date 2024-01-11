// RaylibTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include <format>

#include "raylib.h"
#include "raymath.h"

constexpr int _width = 1600;
constexpr int _height = 800;
constexpr Vector2 gravity {0,.5};

struct Drawable
{

	virtual ~Drawable() = default;
	virtual void draw() = 0;
};

struct Rect : public Drawable
{

    Vector2 pos;
    int height;
    int width;
    Color col;
    Vector2 vel;

    Rect(Vector2 pos, int height, int width, Color col, Vector2 vel) 
    {
        this->pos = pos;
        this->height = height;
        this->width = width;
        this->col = col;
        this->vel = vel;
    }

    Rect();


    void draw() override
	{
        pos = Vector2Clamp(Vector2Add(pos, vel), { 0 }, { _width - static_cast<float>(width), _height - static_cast<float>(height) });
        

     //   if(pos.x >= _width || pos.x <= 0)
     //   {
     //       vel.x = -vel.x;
     //   }
    	//if (pos.y >= _height || pos.y <= 0 )
    	//{
     //       vel.y = -vel.y;
    	//}
        

        DrawRectangle(pos.x, pos.y, width, height, col);

        
	}
    inline Vector2 Center() const
    {
        return Vector2Add(pos, { static_cast<float>(width) / 2, static_cast<float>(height) / 2 });
    }
};

struct Player : public Rect
{

    Player(Vector2 pos, int height, int width, Color col, Vector2 vel) : Rect(pos, height, width, col, vel)
    {
	    
    }
    void draw() override
    {
        pos = Vector2Clamp(Vector2Add(pos, vel), { 0 }, { _width - static_cast<float>(width), _height - static_cast<float>(height) });


        //   if(pos.x >= _width || pos.x <= 0)
        //   {
        //       vel.x = -vel.x;
        //   }
           //if (pos.y >= _height || pos.y <= 0 )
           //{
        //       vel.y = -vel.y;
           //}

        
        Rect::draw();
        // Gravity
        // vel = Vector2Clamp(Vector2Add(vel, gravity), { 0,-20 }, { 0,20 });
    }

    
};

struct Enemy : public Rect
{
    const float rotation_error_threshold = 1;

    float maxDistAwayFromCenter;

    Player* target = nullptr;
    float speed;
    float threshold;
    bool isRotating = false;
    bool counterClockWise = false;

    Enemy(Vector2 pos, int height, int width, Color col, Vector2 vel, Player* target, float speed, float threshold) : Rect(pos, height, width, col, vel)
    {
        this->target = target;
        this->speed = speed;
        this->threshold = threshold;
    	this->maxDistAwayFromCenter = sqrtf((width * width)/4.f + (height * height)/4.f);
    }

    void draw() override
    {
        // Calculate velocity based on the target position
        const auto dirVel = Vector2Normalize(Vector2Subtract(target->Center(), pos));
        vel = Vector2Scale(dirVel,
            speed * (Vector2Distance(pos, target->Center()) / threshold));

        // Calculate distance to the target center

        if (const auto dist = Vector2Distance(pos, target->Center()); dist <= threshold)
        {
            if(dist < threshold - rotation_error_threshold)
            {
                vel = Vector2Scale(Vector2Negate(dirVel), threshold / dist);
                isRotating = false;
                Rect::draw();
                return;
            }
            // Calculate orthogonal vector
            Vector2 ortho{ vel.y, -vel.x };

            if (isRotating)
            {
                // Rotate clockwise or counterclockwise based on the flag
                vel = counterClockWise ? Vector2Negate(ortho) : ortho;
            }
            else
            {
                // Start rotating and determine direction based on dot product
                isRotating = true;
                const Vector2 horizon{ vel.x > 0 ? 1.f : -1.f, 0 };

                if (Vector2DotProduct(ortho, horizon) < 0)
                {
                    vel = Vector2Negate(ortho);
                    counterClockWise = true;
                }
                else
                {
                    vel = ortho;
                    counterClockWise = false;
                }
            }
            const auto predictedPathVector = Vector2Add(Vector2Scale(Vector2Normalize(vel), maxDistAwayFromCenter), pos);
            if( predictedPathVector.x <=0 || predictedPathVector.y <= 0 || predictedPathVector.x >= _width || predictedPathVector.y >= _height)
            {
                counterClockWise = !counterClockWise;
            }
        }
        else
        {
            // Check if the distance is greater than the threshold with rotation error tolerance
            if (dist >= threshold + rotation_error_threshold)
                isRotating = false;
        }

        // Call the draw method from the base class
        Rect::draw();
    }
    void drawVelVectorWithScale(float scale) const
    {
        const auto enemyCenter = this->Center();
        DrawLine(enemyCenter.x, enemyCenter.y, enemyCenter.x + (this->vel.x * scale), enemyCenter.y + (this->vel.y * scale), BLUE);
    }

};

inline Color GetRandomColor()
{
    return { static_cast<unsigned char>(GetRandomValue(0,255)),static_cast<unsigned char>(GetRandomValue(0,255)), static_cast<unsigned char>(GetRandomValue(0,255)), 255 };
}

int main()
{
    
    InitWindow(_width, _height, "Raylib Test");
    SetTargetFPS(144);

    std::vector<Drawable*> objects;
    std::vector<Enemy*> enemies;
    Player mainCharacter ( {50,50} , 50, 50 , BLACK, {0});
    objects.push_back(&mainCharacter);

    const int enemyCount = 50;
    for(int i = 0; i < enemyCount; i++)
    {
        auto enemy = new Enemy({
            static_cast<float>(GetRandomValue(0, _width - 20)), static_cast<float>(GetRandomValue(0, _height - 20))

            }, 20, 20, GetRandomColor(), { 0 }, &mainCharacter, GetRandomValue(1,10) / 10.f, 100);
		enemies.push_back(enemy);
		objects.push_back(enemy);
    }
    



    while( ! WindowShouldClose() )
    {
        BeginDrawing();
        ClearBackground(Color{ 255,255,255,255 });
        
        
        
        if (IsKeyDown(KEY_UP))
        {
            mainCharacter.vel.y = -2;  // Apply a continuous upward force
        }else if(IsKeyDown(KEY_DOWN))
        {
            mainCharacter.vel.y = 2;
        }else
        {
            mainCharacter.vel.y = 0;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            mainCharacter.vel.x = 2;
        }
        else if (IsKeyDown(KEY_LEFT))
        {
            mainCharacter.vel.x = -2;
        }
        else
        {
            mainCharacter.vel.x = 0;
        }


        

        
        for(const auto d : objects)
        {
            d->draw();
        }
        auto debug = std::format("Vel: ( {} : {} )", mainCharacter.vel.x, mainCharacter.vel.y);
        DrawText(debug.c_str(), 0, 0, 40, BLACK);

		for(const auto e : enemies)
		{
            e->drawVelVectorWithScale(20);
		}


       

        

        EndDrawing();
    }
}


