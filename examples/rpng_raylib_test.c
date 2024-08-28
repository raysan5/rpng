/*******************************************************************************************
*
*   raylib rpng example - PNG image loading and drawing
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RPNG_IMPLEMENTATION
#include "../src/rpng.h"

#include <stdio.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib rpng example - png image loading and drawing");

#if 0
    int channels = 0;
    int bits = 0;
    Image image = { 0 };
    image.data = rpng_load_image("resources/fudesumi_paint.png", &image.width, &image.height, &channels, &bits);
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1;
    Texture2D texture = LoadTextureFromImage(image);      // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(image);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM
#endif

    int width = 0;
    int height = 0;
    rpng_palette palette = { 0 };
    char *indexed_data = rpng_load_image_indexed("resources/scarfy_indexed.png", &width, &height, &palette);

    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_ENTER))
        {
            rpng_save_image_indexed("resources/scarfy_indexed_output.png", indexed_data, width, height, palette);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            // Draw palette to verify it has been loaded successfully
            for (int i = 0; i < palette.color_count; i++)
            {
                DrawRectangle(10 + 25*(i%24), screenHeight - 34 + 25*(i/24), 24, 24, (Color){ palette.colors[i].r, palette.colors[i].g, palette.colors[i].b, palette.colors[i].a });
                DrawRectangleLines(10 + 25*(i%24), screenHeight - 34 + 25*(i/24), 24, 24, BLACK);
            }
            
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int index = (int)indexed_data[y*width + x];
                    if (index > palette.color_count) printf("WARNING: Index [%i]: %i\n", y*width + x, index);
                    DrawPixel(x, 200 + y, (Color){ palette.colors[index].r, palette.colors[index].g, palette.colors[index].b, palette.colors[index].a });
                }
            }

            //DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2 - 40, WHITE);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}