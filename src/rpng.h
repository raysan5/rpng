/**********************************************************************************************
*
*   rpng v1.0 - A simple and easy-to-use library to manage png chunks
*
*   FEATURES:
*       - Count/read/write/remove png chunks
*       - Operate on file or file-buffer
*       - Chunks data abstraction
*       - Add custom chunks
*
*   SUMMARY of STANDARD CHUNKS:
*
*   This table summarizes some properties of the standard chunk types.
*
*   Based on official docs:
*       http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
*       http://www.libpng.org/pub/png/book/chapter11.html
*
*   Critical chunks (must appear in this order, except PLTE is optional):
*
*       Name   Multi?  Ordering constraints
*   -------------------------------------------
*       IHDR    No      Must be first
*       PLTE    No      Before IDAT
*       IDAT    Yes     Multiple IDATs must be consecutive
*       IEND    No      Must be last
*
*   Ancillary chunks (need not appear in this order):
*
*       Name   Multi?  Ordering constraints
*   --------------------------------------------
*       cHRM    No      Before PLTE and IDAT
*       gAMA    No      Before PLTE and IDAT
*       iCCP    No      Before PLTE and IDAT
*       sBIT    No      Before PLTE and IDAT
*       sRGB    No      Before PLTE and IDAT
*       bKGD    No      After PLTE; before IDAT
*       hIST    No      After PLTE; before IDAT
*       tRNS    No      After PLTE; before IDAT
*       pHYs    No      Before IDAT
*       sPLT    Yes     Before IDAT
*       tIME    No      None
*       iTXt    Yes     None
*       tEXt    Yes     None
*       zTXt    Yes     None
*
*   Standard keywords for text chunks (iTXt, tEXt, zTXt):
*
*       Title            Short (one line) title or caption for image
*       Author           Name of image's creator
*       Description      Description of image (possibly long)
*       Copyright        Copyright notice
*       Creation Time    Time of original image creation
*       Software         Software used to create the image
*       Disclaimer       Legal disclaimer
*       Warning          Warning of nature of content
*       Source           Device used to create the image
*       Comment          Miscellaneous comment; conversion from GIF comment
*
*   DEPENDENCIES: libc
*       stdlib.h        Required for: malloc(), calloc(), free()
*       string.h        Required for: memcmp(), memcpy()
*       stdio.h         Required for: FILE, fopen(), fread(), fwrite(), fclose() (only if !RPNG_NO_STDIO)
*
*   CONFIGURATION:
*
*   #define RPNG_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RPNG_NO_STDIO
*       Do not include FILE I/O API, only read/write from memory buffers
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2020 Ramon Santamaria (@raysan5)
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

#ifndef RPNG_H
#define RPNG_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef RPNGAPI
#ifdef RPNGAPI_STATIC
#define RPNGAPI static
#else
#define RPNGAPI extern
#endif
#endif

// Allow custom memory allocators
#ifndef RPNG_MALLOC
    #define RPNG_MALLOC(sz)         malloc(sz)
#endif
#ifndef RPNG_CALLOC
    #define RPNG_CALLOC(ptr,sz)     calloc(ptr,sz)
#endif
#ifndef RPNG_REALLOC
    #define RPNG_REALLOC(ptr,sz)    realloc(ptr,sz)
#endif
#ifndef RPNG_FREE
    #define RPNG_FREE(ptr)          free(ptr)
#endif

#ifndef RPNG_MAX_CHUNKS_COUNT
    // Maximum number of chunks to read
    #define RPNG_MAX_CHUNKS_COUNT   64
#endif
#ifndef RPNG_MAX_OUTPUT_SIZE 
    // Maximum size for temporal buffer on write/remove chunks,
    // buffer is scaled to required output file size before being returned
    #define RPNG_MAX_OUTPUT_SIZE    (32*1024*1024)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#ifndef __cplusplus
#include <stdbool.h>        // Boolean type
#endif

// After signature we have a series of chunks, every chunck has the same structure:
typedef struct {
    unsigned int length;    // Data length, must be converted to big endian when saving!
    unsigned char type[4];  // Chunk type FOURCC: IDHR, PLTE, IDAT, IEND / gAMA, sRGB, tEXt, tIME...
    unsigned char *data;    // Chunk data pointer
    unsigned int crc;       // 32bit CRC (computed over type and data)
} rpng_chunk;

// A minimal PNG only requires: png_signature | rpng_chunk(IHDR) | rpng_chunk(IDAT) | rpng_chunk(IEND)

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Functions operating on file (use memory versions internally)
RPNGAPI int rpng_chunk_count(const char *filename);                                  // Count the chunks in a PNG image
RPNGAPI rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);    // Read one chunk type
RPNGAPI rpng_chunk *rpng_chunk_read_all(const char *filename, int *count);           // Read all chunks
RPNGAPI void rpng_chunk_remove(const char *filename, const char *chunk_type);        // Remove one chunk type
RPNGAPI void rpng_chunk_remove_ancillary(const char *filename);                      // Remove all chunks except: IHDR-IDAT-IEND
RPNGAPI void rpng_chunk_write(const char *filename, rpng_chunk data);                // Write one new chunk after IHDR (any kind)
RPNGAPI void rpng_chunk_write_text(const char *filename, char *keyword, char *text); // Write tEXt chunk
RPNGAPI void rpng_chunk_print_info(const char *filename);                            // Output info about the chunks

// Functions operating on memory buffer
RPNGAPI int rpng_chunk_count_from_memory(const char *buffer);                                             // Count the chunks in a PNG image on memory
RPNGAPI rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type);               // Read one chunk type on memory
RPNGAPI rpng_chunk *rpng_chunk_read_all_from_memory(const char *buffer, int *count);                      // Read all chunks on memory
RPNGAPI char *rpng_chunk_remove_from_memory(const char *buffer, const char *chunk_type, int *output_size);      // Remove one chunk type on memory
RPNGAPI char *rpng_chunk_remove_ancillary_from_memory(const char *buffer, int *output_size);                    // Remove all chunks except: IHDR-IDAT-IEND
RPNGAPI char *rpng_chunk_write_to_memory(const char *buffer, rpng_chunk chunk, int *output_size);               // Write one new chunk after IHDR (any kind)
RPNGAPI char *rpng_chunk_write_text_to_memory(const char *buffer, char *keyword, char *text, int *output_size); // Write one new tEXt chunk

#ifdef __cplusplus
}
#endif

#endif // RPNG_H

/***********************************************************************************
*
*   RPNG IMPLEMENTATION
*
************************************************************************************/

#if defined(RPNG_IMPLEMENTATION)

#if !defined(RPNG_NO_STDIO)
#include <stdio.h>      // Required for: FILE, fopen(), fread(), fwrite(), fclose()
#endif

#include <stdlib.h>         // Required for: malloc(), calloc(), free()
#include <string.h>         // Required for: memcmp(), memcpy()

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Critical chunks (IHDR, PLTE, IDAT, IEND)
//------------------------------------------------------------------------

// Image header 
// Mandatory chunck: image info (13 bytes)
typedef struct {
    unsigned int width;         // Image width
    unsigned int height;        // Image width
    unsigned char bit_depth;    // Bit depth
    unsigned char color_type;   // Pixel format: 0 - Grayscale, 2 - RGB, 3 - Indexed, 4 - GrayAlpha, 6 - RGBA
    unsigned char compression;  // Compression method: 0
    unsigned char filter;       // Filter method: 0 (default)
    unsigned char interlace;    // Interlace scheme (optional): 0 (none)
} rpng_chunk_IHDR;

// Palette
// Contains from 1 to 256 palette entries, each a three-byte series (RGB)
// Chunk must appear for color type 3, and can appear for color types 2 and 6; it must not appear for color types 0 and 4. 
// If this chunk does appear, it must precede the first IDAT chunk. There must not be more than one PLTE chunk
typedef struct {
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } *palette;                 // Palette color values
} rpng_chunk_PLTE;

// Image data
// There can be multiple IDAT chunks; if so, they must appear consecutively with no other intervening chunks
typedef struct {
    unsigned char *data;        // Pixel data, output datastream of the compression algorithm
} rpng_chunk_IDAT;

// Image trailer
// rpng_chunk_IEND > rpng_chunk (empty), it should be LAST

// Transparency information
//------------------------------------------------------------------------
// Transparency
// For color type 3 (indexed color), the tRNS chunk contains a series of one-byte alpha values, corresponding to entries in the PLTE chunk.
// For color type 0 (grayscale), the tRNS chunk contains a single gray level value, stored in the format: Gray (2 bytes), range 0 .. (2^bitdepth)-1
// For color type 2 (truecolor), the tRNS chunk contains a single RGB color value, stored in the format: Red-Green-Blue (2 bytes each), range 0 .. (2^bitdepth)-1
// tRNS is prohibited for color types 4 and 6, since a full alpha channel is already present in those cases.
//When present, the tRNS chunk must precede the first IDAT chunk, and must follow the PLTE chunk, if any.
typedef struct {
    unsigned char *data;        // Gamma times 100000 (i.e. Gamma of 1/2.2 would be stored as 45455)
} rpng_chunk_tRNS;

// Color space information
//------------------------------------------------------------------------
// Image gamma
// Specifies the relationship between the image samples and the desired display output intensity as a power function: sample = light_out^gamma
// Sample and light_out are normalized to the range 0.0 (minimum intensity) to 1.0 (maximum intensity). Therefore: sample = integer_sample/(2^bitdepth - 1)
typedef struct {
    unsigned int gamma;         // Gamma times 100000 (i.e. Gamma of 1/2.2 would be stored as 45455)
} rpng_chunk_gAMA;

// Primary chromaticities
// NOTE: Values multiplied by 100000 (i.e. Value 0.3127 would be stored as the integer 31270)
typedef struct {
    unsigned int white_point_x;
    unsigned int white_point_y;
    unsigned int redx;
    unsigned int redy;
    unsigned int greenx;
    unsigned int greeny;
    unsigned int bluex;
    unsigned int bluey;
} rpng_chunk_cHRM;

// Standard RGB color space
// When using sRGB chunk, gAMA should also be present (and perhaps a cHRM chunk)
typedef struct {
    unsigned char flag;         // 0: Perceptual, 1: Relative colorimetric, 2: Saturation, 3: Absolute colorimetric
} rpng_chunk_sRGB;

// Embedded ICC profile
typedef struct {
    unsigned char *profile;     // Profile name: 1-80 bytes (must end with NULL separator: /0)
    unsigned char comp;         // Compression method (0 for DEFLATE)
    unsigned char *comp_profile; // Compressed profile: n bytes
} rpng_chunk_iCCP;

// Textual information
//------------------------------------------------------------------------
// Textual data
typedef struct {
    unsigned char *keyword;     // Keyword: 1-80 bytes (must end with NULL separator: /0)
    unsigned char *text;        // Text: n bytes (character string, no NULL terminated required)
} rpng_chunk_tEXt;

// Compressed textual data
typedef struct {
    unsigned char *keyword;     // Keyword: 1-80 bytes (must end with NULL separator: /0)
    unsigned char comp;         // Compression method (0 for DEFLATE)
    unsigned char *comp_text;   // Compressed text: n bytes
} rpng_chunk_zTXt;

// International textual data
typedef struct {
    unsigned char *keyword;     // Keyword: 1-80 bytes (must end with NULL separator: /0)
    unsigned char comp_flag;    // Compression flag (0 for uncompressed text, 1 for compressed text)
    unsigned char comp;         // Compression method (0 for DEFLATE)
    unsigned char *lang_tag;    // Language tag (0 or more bytes, must end with NULL separator: /0)
    unsigned char *tr_keyword;  // Translated keyword (0 or more bytes, must end with NULL separator: /0)
    unsigned char *text;        // UTF-8 text (0 or more bytes)
} rpng_chunk_iTXt;

//Miscellaneous information
//------------------------------------------------------------------------

// Background color
// Color type 3 (indexed color) -> Palette index:  1 byte
// Color types 0 and 4 (gray or gray + alpha) -> Gray:  2 bytes, range 0 .. (2^bitdepth)-1
// Color types 2 and 6 (truecolor, with or without alpha)
//      Red:   2 bytes, range 0 .. (2^bitdepth)-1
//      Green: 2 bytes, range 0 .. (2^bitdepth)-1
//      Blue:  2 bytes, range 0 .. (2^bitdepth)-1
typedef struct {
    unsigned char *color;
} rpng_chunk_bKGD;

// Physical pixel dimensions
typedef struct {
    unsigned int pixels_per_unit_x;
    unsigned int pixels_per_unit_y;
    unsigned char unit_specifier;       // 0 - Unit unknown, 1 - Unit is meter
} rpng_chunk_pHYs;

// Image last-modification time
typedef struct {
    unsigned short year;         // Year complete, i.e. 1995
    unsigned char month;         // 1 to 12
    unsigned char day;           // 1 to 31
    unsigned char hour;          // 0 to 23
    unsigned char minute;        // 0 to 59
    unsigned char second;        // 0 to 60 (yes, 60, for leap seconds; not 61, a common error)
} rpng_chunk_tIME;

// Other chunks (view documentation)
//sBIT Significant bits
//sPLT Suggested palette
//hIST Palette histogram

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const char png_signature[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a }; // PNG Signature

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static unsigned int swap_endian(unsigned int value);                // Swap integer from big<->little endian
static unsigned int compute_crc32(unsigned char *buffer, int size); // Compute CRC32

// Load data from file into a buffer
static char *load_file_to_buffer(const char *filename, int *bytes_read);
static void save_file_from_buffer(const char *filename, void *data, int bytesToWrite);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Count number of PNG chunks
int rpng_chunk_count(const char *filename)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int count = rpng_chunk_count_from_memory(file_data);

    RPNG_FREE(file_data);

    return count;
}

// Read one chunk from a PNG file
// NOTE: There could be multiple chunks of same type, only first found is returned
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = rpng_chunk_read_from_memory(file_data, chunk_type);

    RPNG_FREE(file_data);

    return chunk;
}

// Read all chunks from a PNG file
rpng_chunk *rpng_chunk_read_all(const char *filename, int *count)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int counter = 0;
    rpng_chunk *chunks = rpng_chunk_read_all_from_memory(file_data, &counter);

    RPNG_FREE(file_data);

    *count = counter;
    return chunks;
}

// Remove text chunk by type
void rpng_chunk_remove(const char *filename, const char *chunk_type)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_remove_from_memory(file_data, chunk_type, &file_output_size);

    if (file_output_size < (int)file_size) save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Remove all chunks except: IHDR-IDAT-IEND
void rpng_chunk_remove_ancillary(const char *filename)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_remove_ancillary_from_memory(file_data, &file_output_size);

    if (file_output_size < (int)file_size) save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Add one new chunk (any kind)
// NOTE: Chunk is added by default after IHDR
void rpng_chunk_write(const char *filename, rpng_chunk chunk)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_write_to_memory(file_data, chunk, &file_output_size);

    if (file_output_size > (int)file_size) save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Save text chunk data into PNG
// NOTE: It will be added just after IHDR chunk
void rpng_chunk_write_text(const char *filename, char *keyword, char *text)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_write_text_to_memory(file_data, keyword, text, &file_output_size);

    if (file_output_size > (int)file_size) save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Output info about the chunks
void rpng_chunk_print_info(const char *filename)
{
    int count = 0;
    rpng_chunk *chunks = rpng_chunk_read_all(filename, &count);

    printf("\n| Chunk |   Data Length  |   CRC32   |\n");
    printf("|-------|----------------|-----------|\n");
    for (int i = 0; i < count; i++)
    {
        printf("| %c%c%c%c  | %8i bytes |  %08X |\n", chunks[i].type[0], chunks[i].type[1], chunks[i].type[2], chunks[i].type[3], chunks[i].length, chunks[i].crc);
    }
    printf("\n");
    /*
        rpng_chunk_IHDR *IHDRData = (rpng_chunk_IHDR *)chunks[0].data;
        printf("\n| IHDR information    |\n");
        printf("|---------------------|\n");
        printf("| width:         %4i |\n", swap_endian(IHDRData->width));   // Image width
        printf("| weight:        %4i |\n", swap_endian(IHDRData->height));  // Image height
        printf("| bit depth:     %4i |\n", IHDRData->bit_depth);            // Bit depth
        printf("| color type:    %4i |\n", IHDRData->color_type);           // Pixel format: 0-Grayscale, 2-RGB, 3-Indexed, 4-GrayAlpha, 6-RGBA
        printf("| compression:      %i |\n", IHDRData->compression);         // Compression method: 0
        printf("| filter method:    %i |\n", IHDRData->filter);            // Filter method: 0 (default)
        printf("| interlace:        %i |\n", IHDRData->interlace);             // Interlace scheme (optional): 0 (none)
    */
    for (int i = 0; i < count; i++) RPNG_FREE(chunks[i].data);
    RPNG_FREE(chunks);
}

// Functions operating on memory buffers data
//----------------------------------------------------------------------------------------------------------

// Count the chunks in a PNG image from memory buffer
int rpng_chunk_count_from_memory(const char *buffer)
{
    char *file_data_ptr = (char *)buffer;
    int count = 0;

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        file_data_ptr += 8;       // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);
        file_data_ptr += 4;

        while (memcmp(file_data_ptr, "IEND", 4) != 0) // While IEND chunk not reached
        {
            //printf("Chunk: %c%c%c%c [%i bytes]\n", ((char *)file_data_ptr)[0], ((char *)file_data_ptr)[1], ((char *)file_data_ptr)[2], ((char *)file_data_ptr)[3], chunk_len);
            file_data_ptr += (4 + chunk_len + 4);   // Skip chunk type FOURCC + chunk data + CRC32

            chunk_len = swap_endian(((int *)file_data_ptr)[0]);
            file_data_ptr += 4;  // Skip chunk file_size
            count++;
        }

        //printf("Chunk: %c%c%c%c [%i bytes]\n", ((char *)file_data_ptr)[0], ((char *)file_data_ptr)[1], ((char *)file_data_ptr)[2], ((char *)file_data_ptr)[3], chunk_len);
        count++; // IEND chunk!
    }

    return count;
}

// Read one chunk type from memory buffer
rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type)
{
    char *file_data_ptr = (char *)buffer;
    rpng_chunk chunk = { 0 };

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        file_data_ptr += 8;   // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);

        while (memcmp(file_data_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            if (memcmp(file_data_ptr + 4, chunk_type, 4) == 0)
            {
                chunk.length = chunk_len;
                memcpy(chunk.type, (char *)(file_data_ptr + 4), 4);
                chunk.data = RPNG_MALLOC(chunk_len);
                memcpy(chunk.data, file_data_ptr + 8, chunk_len);
                chunk.crc = swap_endian(((unsigned int *)(file_data_ptr + 8 + chunk_len))[0]);

                break;
            }

            file_data_ptr += (4 + 4 + chunk_len + 4);           // Move pointer to next chunk of input data
            chunk_len = swap_endian(((int *)file_data_ptr)[0]);  // Compute next chunk file_size
        }
    }

    return chunk;
}

// Read all chunks from memory buffer
rpng_chunk *rpng_chunk_read_all_from_memory(const char *buffer, int *count)
{
    char *file_data_ptr = (char *)buffer;
    rpng_chunk *chunks = NULL;
    int counter = 0;

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        // We allocate enough space for 64 chunks
        chunks = (rpng_chunk *)RPNG_CALLOC(RPNG_MAX_CHUNKS_COUNT, sizeof(rpng_chunk));

        file_data_ptr += 8;                       // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);

        while (memcmp(file_data_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            chunks[counter].length = chunk_len;
            memcpy(chunks[counter].type, (char *)(file_data_ptr + 4), 4);
            chunks[counter].data = RPNG_MALLOC(chunk_len);
            memcpy(chunks[counter].data, file_data_ptr + 8, chunk_len);
            chunks[counter].crc = swap_endian(((unsigned int *)(file_data_ptr + 8 + chunk_len))[0]);

            file_data_ptr += (4 + 4 + chunk_len + 4);   // Move pointer to next chunk
            chunk_len = swap_endian(((int *)file_data_ptr)[0]);

            counter++;

            if (counter >= (RPNG_MAX_CHUNKS_COUNT - 2)) break;   // WARNING: Too many chunks!
        }

        // Read final IEND chunk
        chunks[counter].length = chunk_len;
        memcpy(chunks[counter].type, (char *)(file_data_ptr + 4), 4);
        chunks[counter].data = RPNG_MALLOC(chunk_len);
        memcpy(chunks[counter].data, file_data_ptr + 8, chunk_len);
        chunks[counter].crc = swap_endian(((unsigned int *)(file_data_ptr + 8 + chunk_len))[0]);

        counter++;
    }

    // Reallocate chunks file_size
    rpng_chunk *temp = RPNG_REALLOC(chunks, counter*sizeof(rpng_chunk));
    if (temp != NULL) chunks = temp;

    *count = counter;
    return chunks;
}

// Remove one chunk type from memory buffer
// NOTE: returns output_data and output_size through parameter 
char *rpng_chunk_remove_from_memory(const char *buffer, const char *chunk_type, int *output_size)
{
    char *file_data_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int out_size = 0;

    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        out_size += 8;
        
        file_data_ptr += 8;       // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);

        while (memcmp(file_data_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            // If chunk type is not the requested, just copy input data into output buffer
            if (memcmp(file_data_ptr + 4, chunk_type, 4) != 0)
            {
                memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + chunk_len + 4);  // Length + FOURCC + chunk_len + CRC32
                out_size += (4 + 4 + chunk_len + 4);
            }

            file_data_ptr += (4 + 4 + chunk_len + 4);   // Move pointer to next chunk
            chunk_len = swap_endian(((int *)file_data_ptr)[0]);
        }

        // Save IEND chunk
        memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + 4);
        out_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, out_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = out_size;
    return output_buffer;
}

// Remove all chunks from memory buffer except: IHDR-IDAT-IEND
// NOTE: returns output_data and output_size through parameter 
char *rpng_chunk_remove_ancillary_from_memory(const char *buffer, int *output_size)
{
    char *file_data_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int out_size = 0;

    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        out_size += 8;
        file_data_ptr += 8;       // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);

        while (memcmp(file_data_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            // If chunk type is mandatory, just copy input data into output buffer
            if ((memcmp(file_data_ptr + 4, "IHDR", 4) == 0) ||
                (memcmp(file_data_ptr + 4, "PLTE", 4) == 0) ||
                (memcmp(file_data_ptr + 4, "IDAT", 4) == 0))
            {
                memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + chunk_len + 4);  // Length + FOURCC + chunk_len + CRC32
                out_size += (4 + 4 + chunk_len + 4);
            }

            file_data_ptr += (4 + 4 + chunk_len + 4);   // Move pointer to next chunk
            chunk_len = swap_endian(((int *)file_data_ptr)[0]);
        }

        // Save IEND chunk
        memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + 4);
        out_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, out_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = out_size;
    return output_buffer;
}

// Write one new chunk after IHDR (any kind) to memory buffer
// NOTE: returns output data file_size
char *rpng_chunk_write_to_memory(const char *buffer, rpng_chunk chunk, int *output_size)
{
    char *file_data_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int out_size = 0;

    if ((file_data_ptr != NULL) && (memcmp(file_data_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        out_size += 8;
        file_data_ptr += 8;       // Move pointer after signature

        unsigned int chunk_len = swap_endian(((int *)file_data_ptr)[0]);

        while (memcmp(file_data_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + chunk_len + 4);  // Length + FOURCC + chunk_len + CRC32
            out_size += (4 + 4 + chunk_len + 4);

            // Check if we just copied the IHDR chunk to append our chunk after it
            if (memcmp(file_data_ptr + 4, "IHDR", 4) == 0)
            {
                int lengthBE = swap_endian(chunk.length);
                memcpy(output_buffer + out_size, &lengthBE, sizeof(int));           // Write chunk length
                memcpy(output_buffer + out_size + 4, chunk.type, 4);                // Write chunk type
                memcpy(output_buffer + out_size + 4 + 4, chunk.data, chunk.length); // Write chunk data

                unsigned char *type_data = RPNG_MALLOC(4 + chunk.length);
                memcpy(type_data, chunk.type, 4);
                memcpy(type_data + 4, chunk.data, chunk.length);
                unsigned int crc = compute_crc32(type_data, 4 + chunk.length);
                crc = swap_endian(crc);
                memcpy(output_buffer + out_size + 4 + 4 + chunk.length, &crc, 4);   // Write CRC32 (computed over type + data)
                
                RPNG_FREE(type_data);

                out_size += (4 + 4 + chunk.length + 4);  // Update output file file_size with new chunk
            }

            file_data_ptr += (4 + 4 + chunk_len + 4);           // Move pointer to next chunk of input data
            chunk_len = swap_endian(((int *)file_data_ptr)[0]);  // Compute next chunk file_size
        }

        // Save IEND chunk
        memcpy(output_buffer + out_size, file_data_ptr, 4 + 4 + 4);
        out_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, out_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = out_size;
    return output_buffer;
}

// Write tEXt chunk to memory buffer
// NOTE: returns output data file_size
char *rpng_chunk_write_text_to_memory(const char *buffer, char *keyword, char *text, int *output_size)
{
    rpng_chunk chunk = { 0 };
    int keyword_len = strlen(keyword);
    int text_len = strlen(text);

    chunk.length = keyword_len + 1 + text_len;
    memcpy(chunk.type, "tEXt", 4);
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    memcpy(((unsigned char*)chunk.data), keyword, keyword_len);
    memcpy(((unsigned char*)chunk.data) + keyword_len + 1, text, text_len);
    chunk.crc = 0;  // Computed by rpng_chunk_write_to_memory()

    int out_size = 0;
    char *output_buffer = rpng_chunk_write_to_memory(buffer, chunk, &out_size);

    *output_size = out_size;
    return output_buffer;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Swap integer from big<->little endian
static unsigned int swap_endian(unsigned int value)
{
    // Swap endian (big to little) or (little to big)
    unsigned int b0, b1, b2, b3;
    unsigned int res;

    b0 = (value & 0x000000ff) << 24u;
    b1 = (value & 0x0000ff00) << 8u;
    b2 = (value & 0x00ff0000) >> 8u;
    b3 = (value & 0xff000000) >> 24u;

    res = b0 | b1 | b2 | b3;

    return res;
}

// Compute CRC32
static unsigned int compute_crc32(unsigned char *buffer, int size)
{
    static unsigned int crc_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0eDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    unsigned int crc = ~0u;

    for (int i = 0; i < size; i++) crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];

    return ~crc;
}

// Load data from file into a buffer
static char *load_file_to_buffer(const char *filename, int *bytes_read)
{
    char *data = NULL;
    *bytes_read = 0;

#if !defined(RPNG_NO_STDIO)
    if (filename != NULL)
    {
        FILE *file = fopen(filename, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (file_size > 0)
            {
                data = (char *)RPNG_MALLOC(sizeof(unsigned char)*file_size);

                // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, file_size elements]
                int count = (int)fread(data, sizeof(char), file_size, file);
                *bytes_read = count;

                if (count != file_size) printf("FILEIO: [%s] File partially loaded\n", filename);
                else printf("FILEIO: [%s] File loaded successfully\n", filename);
            }
            else printf("FILEIO: [%s] Failed to read file\n", filename);

            fclose(file);
        }
        else printf("FILEIO: [%s] Failed to open file\n", filename);
    }
    else printf("FILEIO: File name provided is not valid\n");
#else
    (void)filename;
    #warning No FILE I / O API, RPNG_NO_STDIO defined
#endif
        return data;
}

// Save data to file from buffer
static void save_file_from_buffer(const char *filename, void *data, int bytesToWrite)
{
#if !defined(RPNG_NO_STDIO)
    if (filename != NULL)
    {
        FILE *file = fopen(filename, "wb");

        if (file != NULL)
        {
            int count = (int)fwrite(data, sizeof(char), bytesToWrite, file);

            if (count == 0) printf("FILEIO: [%s] Failed to write file\n", filename);
            else if (count != bytesToWrite) printf("FILEIO: [%s] File partially written\n", filename);
            else printf("FILEIO: [%s] File saved successfully\n", filename);

            fclose(file);
        }
        else printf("FILEIO: [%s] Failed to open file\n", filename);
    }
    else printf("FILEIO: File name provided is not valid\n");
#else
    (void)filename;
    (void)data;
    (void)bytesToWrite;
    #warning No FILE I / O API, RPNG_NO_STDIO defined
#endif
}

#endif  // RPNG_IMPLEMENTATION
