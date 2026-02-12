/**

* Author: Matthew Jiang 

* Assignment: Simple 2D Scene

* Date due: 02/14/2026 

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

//note i know eric personally :)

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH        = 1800,
              SCREEN_HEIGHT       = 1100,
              SIZE                = 1000, 
              FPS                 = 120,
              SIDES               = 3;

constexpr float RADIUS = 400.0f; 
constexpr float ORBIT_SPEED = 2.0f;  
constexpr float MAX_AMPLITUDE = 10.0f;

constexpr Vector2 ORIGIN = { 
    SCREEN_WIDTH  / 2, 
    SCREEN_HEIGHT / 2
};

constexpr Vector2 BASE_SIZE = { 
    500.0f, 500.0f
}; 

// Global Variables
AppStatus gAppStatus = RUNNING;
Texture2D gTexture1;
Texture2D gTexture2;
Texture2D gTexture3;
Vector2 gScale = BASE_SIZE;
Vector2 gPosition1 = ORIGIN;
Vector2 gPosition2 = ORIGIN;
Vector2 gPosition3 = ORIGIN;
float gPreviousTicks = 0.0f;
float gOrbitLocation = 0.0f;
float gRotation = 0.0f;
Vector2 gScale2 = {200.0f , 200.0f}; 
Vector2 gScale3 = {400.0f , 400.0f};
float gBackgroundTime = 0.0f;  
int gColorIndex = 0;

// Function Declarations
Color ColorFromHex(const char *hex);
void initialise();
void processInput();
void update();
void render();
void shutdown();

// You can ignore this function, it's just to get nice colours :
Color ColorFromHex(const char *hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    // Default alpha = 255 (opaque)
    unsigned int r = 0, 
                 g = 0, 
                 b = 0, 
                 a = 255;

    // 6‑digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // 8‑digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}


void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Funny Heartbeat");

    gTexture1 = LoadTexture("assets/maomao.jpg");
    gTexture2 = LoadTexture("assets/eric.jpg");
    gTexture3 = LoadTexture("assets/patrick.jpg");

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    //object 1 transform orbit and rotation 
    gOrbitLocation += ORBIT_SPEED * deltaTime;
    float theta = gOrbitLocation;
    gPosition1.x = ORIGIN.x + RADIUS * cosf(theta);
    gPosition1.y = ORIGIN.y + RADIUS * sinf(theta);

    gRotation += 50.0f * deltaTime; 


    //object 2 tranformation orbit relative to object 1 
    gPosition2.x = gPosition1.x + RADIUS * cosf(theta * 2);
    gPosition2.y = gPosition1.y + RADIUS * sinf(theta * 2);


    //object 3 tranformation diagonal sin wave 
    gPosition3.x += 150.0f * deltaTime;
    gPosition3.y += 100.0f * deltaTime;
    float pulse = 50.0f * sin(GetTime());
    gScale3.x = 300.0f + pulse;
    gScale3.y = 300.0f + pulse;

    if (gPosition3.x > SCREEN_WIDTH) gPosition3.x = 0;
    if (gPosition3.y > SCREEN_HEIGHT) gPosition3.y = 0;

    //background
    gBackgroundTime += deltaTime;
    if (gBackgroundTime > 2.0f) { 
        gColorIndex++;
        gBackgroundTime = 0.0f; 

        if (gColorIndex > 3) { 
            gColorIndex = 0; 
        } 
    }
}
    


void render()
{
    BeginDrawing();

    Color background; 

    if (gColorIndex == 0) { 
        background = ColorFromHex("#CF6757");
    }
    else if (gColorIndex == 1) { 
        background = ColorFromHex("#1C355C");
    }

    else if (gColorIndex == 2) { 
        background = ColorFromHex("#521C5C");
    }

    else { 
        background = ColorFromHex("#1C5C26");
    }

    ClearBackground(background);
    
    Rectangle textureArea = { 
        //top left corner 
        0.0f, 
        0.0f,
        static_cast<float> (gTexture1.width),
        static_cast<float> (gTexture1.height)
    
    };

    Rectangle destinationArea = { 
        gPosition1.x, 
        gPosition1.y,
        static_cast<float> (gScale.x),
        static_cast<float> (gScale.y)
    };

    Vector2 originOffset = { 
        gScale.x / 2.0f, gScale.y / 2.0f 
    };

    Rectangle textureArea2 = { 
        //top left corner 
        0.0f, 
        0.0f,
        static_cast<float> (gTexture2.width),
        static_cast<float> (gTexture2.height)
    
    };

    Rectangle destinationArea2 = { 
        gPosition2.x, 
        gPosition2.y,
        static_cast<float> (gScale2.x),
        static_cast<float> (gScale2.y)
    };

    Vector2 originOffset2 = { 
        gScale2.x / 2.0f, gScale2.y / 2.0f 
    };

    Rectangle textureArea3 = { 
        //top left corner 
        0.0f, 
        0.0f,
        static_cast<float> (gTexture3.width),
        static_cast<float> (gTexture3.height)
    
    };

    Rectangle destinationArea3 = { 
        gPosition3.x, 
        gPosition3.y,
        static_cast<float> (gScale3.x),
        static_cast<float> (gScale3.y)
    };

    Vector2 originOffset3 = { 
        gScale3.x / 2.0f, gScale3.y / 2.0f 
    };


    DrawTexturePro(
        gTexture1, 
        textureArea,
        destinationArea,
        originOffset, 
        gRotation,
        WHITE
    ); 

    DrawTexturePro(
        gTexture2, 
        textureArea2,
        destinationArea2,
        originOffset2, 
        0.0f,
        WHITE
    ); 

    DrawTexturePro(
        gTexture3, 
        textureArea3,
        destinationArea3,
        originOffset3, 
        0.0f,
        WHITE
    ); 

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}