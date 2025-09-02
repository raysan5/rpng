/**********************************************************************************************
*
*   rpng benchmark program
*
*   This benchmark implements the following actions:
*
*     1. Generate a set of random images with a random amount of random color pixels (randColFactor)
*     2. Compress and export PNG images using stb_image_write (STBIW)
*     3. Compress and export PNG images using rpng (RPNG)
*     4. Load generated PNGs (STBIW and RPNG) with stb_image and compare pixel data
*
*   ADDITIONAL NOTES: 
*     - All compression/decompression times are measured from memory operations, 
*       this way file access costs are ignored
*     - Compression time measures include png image data filtering process, 
*       depending on data prefilter, compression could be better but also slower
*
*   CONFIGURATION:
*     The 4 processes to be performed can be configured depending on user needs, please note
*     that some of those processes could take long time (> 1 hour) depending on the system
*
*       #define PROCESS_GENERATE_RANDOM_IMAGES
*       #define PROCESS_COMPRESS_EXPORT_STBIW
*       #define PROCESS_COMPRESS_EXPORT_RPNG
*       #define PROCESS_LOAD_COMPARE_STBIW_RPNG
*
*   DEPENDENCIES:
*     - raylib 4.6-dev  - STBIW interface to load and save images from memory, file management
*     - rpng 1.1-dev    - RPNG with latest changes to generated png file streams
*     - rprand 1.0      - Pseudo-random number generator, used to get random image positions
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2023 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#define RPNG_IMPLEMENTATION
#include "../src/rpng.h"            // PNG loading and saving

#define RPRAND_IMPLEMENTATION
#include "external/rprand.h"        // Pseudo-random numbers generator

// Use this defines to select processes to be performed
// NOTE: Some of those processes could take >1h depending on the system
#define PROCESS_GENERATE_RANDOM_IMAGES
#define PROCESS_COMPRESS_EXPORT_STBIW
#define PROCESS_COMPRESS_EXPORT_RPNG
#define PROCESS_LOAD_COMPARE_STBIW_RPNG

//----------------------------------------------------------------------------------
// Defines and macros
//----------------------------------------------------------------------------------
#define MAX_COMBINE_SIZES           50
#define MAX_GENERATED_IMAGES        MAX_COMBINE_SIZES*MAX_COMBINE_SIZES

const unsigned int imageWH[MAX_COMBINE_SIZES] = { 
    1, 2, 4, 6, 8, 12, 16, 18, 24, 32, 36, 40, 48, 60, 64, 80, 100, 128, 
    132, 151, 160, 180, 192, 221, 240, 250, 256, 280, 300, 320, 350, 400, 
    512, 513, 600, 800, 901, 1024, 1280, 1349, 2048, 2765, 3287, 4096, 
    5345, 5999, 7000, 7689, 8000, 8192 };

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static Color GetRandomColor(void);
static Image GenImageRandom(int width, int height, float randColFactor);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(800, 450, "rpng vs STBIW vs zlib benchmark");    // Required for GetTime() measures
    
    rprand_set_seed(68921929);          // Initialize random seed -> Sequences will be always the same!
    
    double baseTime = GetTime();        // Reference time measure (updated before every measure)
    
    double compressTimeRPNG = 0;        // Data compression time with rpng library
    double compressTimeSTBIW = 0;       // Data compression time with stb_image_write library
    
    double decompressTimeRPNG = 0;      // rpng generated image data decompression time, using stb_image
    double decompressTimeSTBI = 0;      // stb_image_write generated image data decompression time, using stb_image

    // Generate Image sequence at multiple sizes and random contents
    Image *images = (Image *)RL_CALLOC(MAX_GENERATED_IMAGES, sizeof(Image));

#if defined(PROCESS_GENERATE_RANDOM_IMAGES)
    // Generate multiple images at multiple sizes with random data
    for (int i = 0; i < MAX_GENERATED_IMAGES; i++)
    {
        float randPixelsFactor = (float)rprand_get_value(10, 60)/100.0f;
        images[i] = GenImageRandom(imageWH[i%50], imageWH[(i/50)], randPixelsFactor);
        TraceLog(LOG_INFO, "%03i Random image generation: [%i, %i] - Random Pixels Factor: %.2f", i + 1, images[i].width, images[i].height, randPixelsFactor);
    }
#endif
    
#if defined(PROCESS_COMPRESS_EXPORT_STBIW)
    // Compress and export PNG images with STBIW
    for (int i = 0; i < MAX_GENERATED_IMAGES; i++)
    {
        baseTime = GetTime();
        
        int outputSize = 0;
        unsigned char *imageData = ExportImageToMemory(images[i], ".png", &outputSize);
        compressTimeSTBIW = (GetTime() - baseTime);
        
        TraceLog(LOG_INFO, "%03i STBIW: [%i, %i] | Out stream size: %i bytes  [%.2f %%] | Comp. time: %.2f ms", i + 1,
            images[i].width, images[i].height, outputSize, ((float)outputSize/(float)GetPixelDataSize(images[i].width, images[i].height, images[i].format))*100.0f, compressTimeSTBIW*1000.0f);
        
        SaveFileData(TextFormat("stbiw/image_%ix%i.png", images[i].width, images[i].height), imageData, outputSize);
        RL_FREE(imageData);
    }
#endif

#if defined(PROCESS_COMPRESS_EXPORT_RPNG)
    // Compress and export PNG images with RPNG
    for (int i = 0; i < MAX_GENERATED_IMAGES; i++)
    {
        baseTime = GetTime();
        
        int outputSize = 0;
        char *imageData = rpng_save_image_to_memory(images[i].data, images[i].width, images[i].height, 4, 8, &outputSize);
        compressTimeRPNG = (GetTime() - baseTime);

        TraceLog(LOG_INFO, "%03i RPNG: [%i, %i] | Out stream size: %i bytes  [%.2f %%] | Comp. time: %.2f ms", i + 1,
            images[i].width, images[i].height, outputSize, ((float)outputSize/(float)GetPixelDataSize(images[i].width, images[i].height, images[i].format))*100.0f, compressTimeRPNG*1000.0f);
        
        SaveFileData(TextFormat("rpng/image_%ix%i.png", images[i].width, images[i].height), imageData, outputSize);
        RL_FREE(imageData);
    }
#endif
 
#if defined(PROCESS_LOAD_COMPARE_STBIW_RPNG)
    // Validate RPNG generated images with STBI
    // NOTE: We load data with STBI and we compare results with STWI
    for (int i = 0; i < MAX_GENERATED_IMAGES; i++)
    {
        // Load data from RPNG file -> Using STBI
        unsigned int imageDataRPNGSize = 0;
        unsigned char *imageDataRPNG = LoadFileData(TextFormat("rpng/image_%ix%i.png", imageWH[i%50], imageWH[(i/50)]), &imageDataRPNGSize);
        
        baseTime = GetTime();
        Image imageRPNG = LoadImageFromMemory(".png", imageDataRPNG, imageDataRPNGSize);
        decompressTimeRPNG = (GetTime() - baseTime);
        baseTime = GetTime();
        
        // Load data from STBIW file -> Using STBI
        unsigned int imageDataSTBIWSize = 0;
        unsigned char *imageDataSTBIW = LoadFileData(TextFormat("stbiw/image_%ix%i.png", imageWH[i%50], imageWH[(i/50)]), &imageDataSTBIWSize);
        
        baseTime = GetTime();
        Image imageSTBIW = LoadImageFromMemory(".png", imageDataSTBIW, imageDataSTBIWSize);
        decompressTimeSTBI = (GetTime() - baseTime);
        baseTime = GetTime();
        
        TraceLog(LOG_INFO, "%03i PNG decompression times [%i, %i] | RPNG: %.2f ms | STBIW: %.2f ms", i + 1,
            imageRPNG.width, imageRPNG.height, decompressTimeRPNG*1000.0f, decompressTimeSTBI*1000.0f);
        
        if (memcmp(imageRPNG.data, imageSTBIW.data, GetPixelDataSize(imageRPNG.width, imageRPNG.height, imageRPNG.format)) != 0)
        {
            // NOTE: If this message is never reached, then everything is ok with image data pixels
            TraceLog(LOG_WARNING, "[%i, %i] Image data possibly corrupted!", imageRPNG.width, imageRPNG.height);
        }
        
        RL_FREE(imageDataRPNG);
        RL_FREE(imageDataSTBIW);
        
        UnloadImage(imageRPNG);
        UnloadImage(imageSTBIW);
    }
#endif
    
    for (int i = 0; i < MAX_GENERATED_IMAGES; i++) UnloadImage(images[i]);
    
    RL_FREE(images);
    
    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
static Color GetRandomColor(void)
{
    Color color = WHITE;
    
    color.r = (unsigned char)rprand_get_value(0, 255);
    color.g = (unsigned char)rprand_get_value(0, 255);
    color.b = (unsigned char)rprand_get_value(0, 255);
    
    return color;
}

// Generate a random color image with a random number of random color pixels
static Image GenImageRandom(int width, int height, float randColFactor)
{
    Image image = GenImageColor(width, height, GetRandomColor());
    int pixelsToUpdate = (int)((float)width*height*randColFactor);
    
    for (int i = 0, pos = 0; i < pixelsToUpdate; i++)
    {
        // Get one random pixel from image
        pos = rprand_get_value(0, width*height - 1);
        ((Color *)image.data)[pos] = GetRandomColor();
    }
    
    return image;
}
