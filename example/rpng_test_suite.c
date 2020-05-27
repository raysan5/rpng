/*******************************************************************************************
*
*   rpng chunks manager - test suite
*
*   This example has been created using rpng 1.0 (https://github.com/raysan5/rpng)
*   rpng is licensed under an unmodified zlib/libpng license (View rpng.h for details)
*
*   Copyright (c) 2020 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#define RPNG_IMPLEMENTATION
#include "rpng.h"

#include <stdio.h>      // Required for: printf()

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // TEST: Chunk count and print chunk info
        printf("Chunks count: %i\n\n", rpng_chunk_count(argv[1]));

        rpng_chunk_print_info(argv[1]);

        // TEST: Chunk reading (all)
        int count = 0;
        rpng_chunk *chunks = rpng_chunk_read_all(argv[1], &count);
        
        // First chunk is always IHDR, we can check data
        rpng_chunk_IHDR *IHDRData = (rpng_chunk_IHDR *)chunks[0].data;
        printf("\n| IHDR information    |\n");
        printf("|---------------------|\n");
        printf("| width:         %4i |\n", swap_endian(IHDRData->width));   // Image width
        printf("| weight:        %4i |\n", swap_endian(IHDRData->height));  // Image height
        printf("| bit depth:     %4i |\n", IHDRData->bit_depth);            // Bit depth
        printf("| color type:    %4i |\n", IHDRData->color_type);           // Pixel format: 0-Grayscale, 2-RGB, 3-Indexed, 4-GrayAlpha, 6-RGBA
        printf("| compression:      %i |\n", IHDRData->compression);        // Compression method: 0 (DEFLATE)
        printf("| filter method:    %i |\n", IHDRData->filter);             // Filter method: 0 (default)
        printf("| interlace:        %i |\n\n", IHDRData->interlace);        // Interlace scheme (optional): 0 (none)

        // Free chunks memory
        for (int i = 0; i < count; i++) RPNG_FREE(chunks[i].data);
        RPNG_FREE(chunks);

        // TEST: Write a chunk - rPNG
        rpng_chunk chunk = { 0 };
        chunk.length = 20;              // Data length
        strcpy(chunk.type, "rPNG");     // Chunck type FOURCC
        chunk.data = RPNG_MALLOC(20);   // Chunck data pointer
        memcpy(chunk.data, "This is a test data.", 20);
        chunk.crc = compute_crc32("rPNGThis is a test data.", 24);          // 32bit CRC (computed over type and data)

        rpng_chunk_write(argv[1], chunk);

        rpng_chunk_print_info(argv[1]);

        // TEST: Write a tEXt chunk
        rpng_chunk_write_text(argv[1], "Description", "rpng, library to manage png chunks");
        
        rpng_chunk_print_info(argv[1]);

        // TEST: Chunk reading
        rpng_chunk rchunk = rpng_chunk_read(argv[1], "rPNG");

        printf("\n");
        printf("  Chunk length:  %i\n", rchunk.length);
        printf("  Chunk type:    %c%c%c%c\n", rchunk.type[0], rchunk.type[1], rchunk.type[2], rchunk.type[3]);
        char data[32] = { 0 };
        memcpy(data, rchunk.data, rchunk.length);
        printf("  Chunk data:    %s\n", data);
        printf("  Chunk crc:     %08X\n\n", rchunk.crc);

        RPNG_FREE(chunk.data);

        // TEST: Chunk removing
        rpng_chunk_remove(argv[1], "rPNG");
        rpng_chunk_remove(argv[1], "tEXt");
        
        rpng_chunk_print_info(argv[1]);
        
        // TEST: Remove all ancillary chunks
        //rpng_chunk_remove_ancillary(argv[1]);
    }
    else printf("WARNING: No input file provided.\n");

    return 0;
}