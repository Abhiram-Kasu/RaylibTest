// RaylibTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include <format>

#include "raylib.h"
#include "raymath.h"

constexpr int _width = 1500;
constexpr int _height = 400;
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
    const float rotation_error_threshold = 2;
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
    }

    void draw() override
    {
        // Calculate velocity based on the target position
        vel = Vector2Scale(Vector2Normalize(Vector2Subtract(target->Center(), pos)),
            speed * (Vector2Distance(pos, target->Center()) / threshold));

        // Calculate distance to the target center

        if (const auto dist = Vector2Distance(pos, target->Center()); dist <= threshold)
        {
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

};

int main()
{
    
    InitWindow(_width, _height, "Raylib Test");
    SetTargetFPS(144);

    std::vector<Drawable*> objects;
    Player mainCharacter ( {50,50} , 50, 50 , BLACK, {0});
    objects.push_back(&mainCharacter);

    Enemy enemy{
	    {
		    static_cast<float>(GetRandomValue(0, _width - 20)), static_cast<float>(GetRandomValue(0, _height - 20))

	    }, 20, 20 , RED, {0}, &mainCharacter, 1, 100 };


    objects.push_back(&enemy);


    while( ! WindowShouldClose() )
    {
        BeginDrawing();
        ClearBackground(Color{ 255,255,255,255 });
        
        
        
        if (IsKeyDown(KEY_UP))
        {
            mainCharacter.vel.y = -5;  // Apply a continuous upward force
        }else if(IsKeyDown(KEY_DOWN))
        {
            mainCharacter.vel.y = 5;
        }else
        {
            mainCharacter.vel.y = 0;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            mainCharacter.vel.x = 5;
        }
        else if (IsKeyDown(KEY_LEFT))
        {
            mainCharacter.vel.x = -5;
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

		if(enemy.isRotating)
		{
            DrawText("Rotating : true", 0, 40, 40, BLACK);
		}else
		{
            DrawText("Rotating : false", 0, 40, 40, BLACK);
		}
        const auto enemyCenter = enemy.Center();
        DrawLine(enemyCenter.x, enemyCenter.y, enemyCenter.x + (enemy.vel.x * 20) , enemyCenter.y + (enemy.vel.y * 20), BLUE);
       

        

        EndDrawing();
    }
}


