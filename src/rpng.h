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
    int length;             // Data length, must be converted to big endian when saving!
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

// Create a PNG file from image data (IHDR, IDAT, IEND)
//  - Color channels defines pixel color channels, supported values: 1 (GRAY), 2 (GRAY+ALPHA), 3 (RGB), 4 (RGBA)
//  - Bit depth defines every color channel size, supported values: 8 bit, 16 bit
// NOTE: It's up to the user to provide the right data format as specified by color_channels and bit_depth
RPNGAPI void rpng_create_image(const char *filename, const char *data, int width, int height, int color_channels, int bit_depth);

// Read and write chunks from file
RPNGAPI int rpng_chunk_count(const char *filename);                                  // Count the chunks in a PNG image
RPNGAPI rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);    // Read one chunk type
RPNGAPI rpng_chunk *rpng_chunk_read_all(const char *filename, int *count);           // Read all chunks
RPNGAPI void rpng_chunk_remove(const char *filename, const char *chunk_type);        // Remove one chunk type
RPNGAPI void rpng_chunk_remove_ancillary(const char *filename);                      // Remove all chunks except: IHDR-PLTE-IDAT-IEND
RPNGAPI void rpng_chunk_write(const char *filename, rpng_chunk data);                // Write one new chunk after IHDR (any kind)

// Write specific chunks to file
RPNGAPI void rpng_chunk_write_text(const char *filename, char *keyword, char *text);        // Write tEXt chunk
RPNGAPI void rpng_chunk_write_comp_text(const char *filename, char *keyword, char *text);   // Write zTXt chunk, DEFLATE compressed text
RPNGAPI void rpng_chunk_write_gamma(const char *filename, float gamma);                     // Write gAMA chunk (stored as int, gamma*100000)
RPNGAPI void rpng_chunk_write_srgb(const char *filename, char srgb_type);                   // Write sRGB chunk, requires gAMA chunk
RPNGAPI void rpng_chunk_write_time(const char *filename, short year, char month, char day, char hour, char min, char sec);  // Write tIME chunk
RPNGAPI void rpng_chunk_write_physical_size(const char *filename, int pixels_unit_x, int pixels_unit_y, bool meters);       // Write pHYs chunk
RPNGAPI void rpng_chunk_write_chroma(const char *filename, float white_x, float white_y, float red_x, float red_y, float green_x, float green_y, float blue_x, float blue_y); // Write cHRM chunk

// Chunk utilities
RPNGAPI void rpng_chunk_print_info(const char *filename);                            // Output info about the chunks
RPNGAPI bool rpng_chunk_check_all_valid(const char *filename);                       // Check chunks CRC is valid
RPNGAPI void rpng_chunk_combine_image_data(const char *filename);                    // Combine multiple IDAT chunks into a single one
RPNGAPI void rpng_chunk_split_image_data(const char *filename, int split_size);      // Split one IDAT chunk into multiple ones

// Read and write chunks from memory buffer
RPNGAPI int rpng_chunk_count_from_memory(const char *buffer);                                             // Count the chunks in a PNG image from memory
RPNGAPI rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type);               // Read one chunk type from memory
RPNGAPI rpng_chunk *rpng_chunk_read_all_from_memory(const char *buffer, int *count);                      // Read all chunks from memory
RPNGAPI char *rpng_chunk_remove_from_memory(const char *buffer, const char *chunk_type, int *output_size);          // Remove one chunk type from memory
RPNGAPI char *rpng_chunk_remove_ancillary_from_memory(const char *buffer, int *output_size);                        // Remove all chunks except: IHDR-IDAT-IEND
RPNGAPI char *rpng_chunk_write_from_memory(const char *buffer, rpng_chunk chunk, int *output_size);                 // Write one new chunk after IHDR (any kind)
RPNGAPI char *rpng_chunk_combine_image_data_from_memory(char *buffer, int *output_size);                            // Combine multiple IDAT chunks into a single one
RPNGAPI char *rpng_chunk_split_image_data_from_memory(char *buffer, int split_size, int *output_size);              // Split one IDAT chunk into multiple ones

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

// NOTE: Some chunks strutures are defined for convenience,
// but only the ones that can be directly serialized to chunk.dataa

// Critical chunks (IHDR, PLTE, IDAT, IEND)
//------------------------------------------------------------------------
// IHDR: Image header 
// Mandatory chunk: image info (13 bytes)
typedef struct {
    unsigned int width;         // Image width
    unsigned int height;        // Image width
    unsigned char bit_depth;    // Bit depth
    unsigned char color_type;   // Pixel format: 0 - Grayscale, 2 - RGB, 3 - Indexed, 4 - GrayAlpha, 6 - RGBA
    unsigned char compression;  // Compression method: 0 (deflate)
    unsigned char filter;       // Filter method: 0 (default)
    unsigned char interlace;    // Interlace scheme (optional): 0 (none)
    // WARNING: 3 bytes of padding required for proper alignment!!!
} rpng_chunk_IHDR;

// PLTE: Palette
// Contains from 1 to 256 palette entries, each a three-byte series (RGB)
// Chunk must appear for color type 3, and can appear for color types 2 and 6; it must not appear for color types 0 and 4. 
// If this chunk does appear, it must precede the first IDAT chunk. There must not be more than one PLTE chunk

// IDAT: Image data
// There can be multiple IDAT chunks; if so, they must appear consecutively with no other intervening chunks

// IEND: Image ending trailer
// rpng_chunk_IEND > rpng_chunk (empty), it should be LAST

// Transparency information
//------------------------------------------------------------------------
// tRNS: Transparency
// For color type 3 (indexed color), the tRNS chunk contains a series of one-byte alpha values, corresponding to entries in the PLTE chunk.
// For color type 0 (grayscale), the tRNS chunk contains a single gray level value, stored in the format: Gray (2 bytes), range 0 .. (2^bitdepth)-1
// For color type 2 (truecolor), the tRNS chunk contains a single RGB color value, stored in the format: Red-Green-Blue (2 bytes each), range 0 .. (2^bitdepth)-1
// tRNS is prohibited for color types 4 and 6, since a full alpha channel is already present in those cases.
// When present, the tRNS chunk must precede the first IDAT chunk, and must follow the PLTE chunk, if any.

// Color space information
//------------------------------------------------------------------------
// gAMA: Image gamma
// Specifies the relationship between the image samples and the desired display output intensity as a power function: sample = light_out^gamma
// Sample and light_out are normalized to the range 0.0 (minimum intensity) to 1.0 (maximum intensity). Therefore: sample = integer_sample/(2^bitdepth - 1)

// cHRM: Primary chromaticities
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

// sRGB: Standard RGB color space
// When using sRGB chunk, gAMA should also be present (and perhaps a cHRM chunk)
typedef struct {
    unsigned char flag;             // 0: Perceptual, 1: Relative colorimetric, 2: Saturation, 3: Absolute colorimetric
} rpng_chunk_sRGB;

// iCCP: Embedded ICC profile
//   unsigned char *profile;        // Profile name: 1-80 bytes (must end with NULL separator: /0)
//   unsigned char comp;            // Compression method (0 for DEFLATE)
//   unsigned char *comp_profile;   // Compressed profile: n bytes

// Textual information
//------------------------------------------------------------------------
// tEXt: Textual data
//   unsigned char *keyword;        // Keyword: 1-80 bytes (must end with NULL separator: /0)
//   unsigned char *text;           // Text: n bytes (character string, no NULL terminated required)

// zTXt: Compressed text data
//   unsigned char *keyword;        // Keyword: 1-80 bytes (must end with NULL separator: /0)
//   unsigned char comp;            // Compression method (0 for DEFLATE)
//   unsigned char *text;           // UTF-8 text (0 or more bytes)

// iTXt: International textual data
//   unsigned char *keyword;        // Keyword: 1-80 bytes (must end with NULL separator: /0)
//   unsigned char comp_flag;       // Compression flag (0 for uncompressed text, 1 for compressed text)
//   unsigned char comp;            // Compression method (0 for DEFLATE)
//   unsigned char *lang_tag;       // Language tag (0 or more bytes, must end with NULL separator: /0)
//   unsigned char *tr_keyword;     // Translated keyword (0 or more bytes, must end with NULL separator: /0)
//   unsigned char *text;           // UTF-8 text (0 or more bytes)


// Miscellaneous information
//------------------------------------------------------------------------

// bKGD: Background color
// Color type  3 (indexed color) -> Palette index:  1 byte
// Color types 0 and 4 (gray or gray + alpha) -> Gray:  2 bytes, range 0 .. (2^bitdepth)-1
// Color types 2 and 6 (truecolor, with or without alpha)
//      Red:   2 bytes, range 0 .. (2^bitdepth)-1
//      Green: 2 bytes, range 0 .. (2^bitdepth)-1
//      Blue:  2 bytes, range 0 .. (2^bitdepth)-1


// pHYs: Physical pixel dimensions
typedef struct {
    unsigned int pixels_per_unit_x;
    unsigned int pixels_per_unit_y;
    unsigned char unit_specifier;   // 0 - Unit unknown, 1 - Unit is meter
} rpng_chunk_pHYs;

// tIME: Image last-modification time
typedef struct {
    unsigned short year;            // Year complete, i.e. 1995
    unsigned char month;            // 1 to 12
    unsigned char day;              // 1 to 31
    unsigned char hour;             // 0 to 23
    unsigned char minute;           // 0 to 59
    unsigned char second;           // 0 to 60 (yes, 60, for leap seconds; not 61, a common error)
} rpng_chunk_tIME;

// Other chunks (view documentation)
//sBIT Significant bits
//sPLT Suggested palette
//hIST Palette histogram

//===================================================================
//                              SDEFL
// DEFLATE algorythm implementation: https://github.com/vurtun/sdefl
//===================================================================
#define SDEFL_MAX_OFF       (1 << 15)
#define SDEFL_WIN_SIZ       SDEFL_MAX_OFF
#define SDEFL_WIN_MSK       (SDEFL_WIN_SIZ-1)

#define SDEFL_MIN_MATCH     4
#define SDEFL_MAX_MATCH     258

#define SDEFL_HASH_BITS     19
#define SDEFL_HASH_SIZ      (1 << SDEFL_HASH_BITS)
#define SDEFL_HASH_MSK      (SDEFL_HASH_SIZ-1)
#define SDEFL_NIL           (-1)

#define SDEFL_LVL_MIN       0
#define SDEFL_LVL_DEF       5
#define SDEFL_LVL_MAX       8

struct sdefl {
    int bits, cnt;
    int tbl[SDEFL_HASH_SIZ];
    int prv[SDEFL_WIN_SIZ];
};

// Compression and decompression
extern int sdefl_bound(int in_len);
extern int sdeflate(struct sdefl *s, unsigned char *out, const unsigned char *in, int in_len, int lvl);
extern int zsdeflate(struct sdefl *s, unsigned char *out, const unsigned char *in, int in_len, int lvl);

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

// The Paeth filter function computes a simple linear function of the three neighbouring pixels (left, above, upper left), 
// then chooses as predictor the neighbouring pixel closest to the computed value. Ref: https://www.w3.org/TR/PNG/#9Filters
// The algorithm used in this International Standard is an adaptation of the technique due to Alan W. Paeth.
// NOTE: Paeth predictor calculations shall be performed exactly, without overflow.
static unsigned char rpng_paeth_predictor(int a, int b, int c)
{
    unsigned char pr = 0;
    
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
   
    if ((pa <= pb) && (pa <= pc)) pr = (unsigned char)a;
    else if (pb <= pc) pr = (unsigned char)b;
    else pr = (unsigned char)c;
    
    return pr;
}

// Create a PNG file from image data (IHDR, IDAT, IEND)
//  - Color channels defines pixel color channels, supported values: 1 (GRAY), 2 (GRAY+ALPHA), 3 (RGB), 4 (RGBA)
//  - Bit depth defines every color channel size, supported values: 8 bit, 16 bit
// NOTE: It's up to the user to provide the right data format as specified by color_channels and bit_depth
void rpng_create_image(const char *filename, const char *data, int width, int height, int color_channels, int bit_depth)
{
    if ((bit_depth != 8) && (bit_depth != 16)) return;  // Bit depth not supported

    int color_type = -1;
    if (color_channels == 1) color_type = 0;        // Grayscale
    else if (color_channels == 2) color_type = 4;   // Gray + Alpha
    else if (color_channels == 3) color_type = 2;   // RGB
    else if (color_channels == 4) color_type = 6;   // RGBA
    
    if (color_type == -1) return;   // Number of channels not supported

    rpng_chunk_IHDR image_info = { 0 };
    image_info.width = swap_endian(width);
    image_info.height = swap_endian(height);
    image_info.bit_depth = (unsigned char)bit_depth;
    image_info.color_type = (unsigned char)color_type;
    
    // Image data pre-processing to append filter type byte to every scanline
    int pixel_size = color_channels*(bit_depth/8);
    int scanline_size = width*pixel_size;
    unsigned int data_size_filtered = (scanline_size + 1)*height;   // Adding 1 byte per scanline filter
    unsigned char *data_filtered = (unsigned char *)RPNG_CALLOC(data_size_filtered, 1);
    
    int out = 0, x = 0, a = 0, b = 0, c = 0;
    int sum_value[5] = { 0 };
    int best_filter = 0;
    
    for (int y = 0; y < height; y++)
    {
        // Choose the best filter type for every scanline
        // REF: https://www.w3.org/TR/PNG-Encoders.html#E.Filter-selection
        for (int p = 0; p < scanline_size; p++)
        {
            x = (int)((unsigned char *)data)[scanline_size*y + p];
            a = (int)((unsigned char *)data)[scanline_size*y + p - ((p > pixel_size)? pixel_size : 0)];
            b = (y > 0)? (int)((unsigned char *)data)[scanline_size*(y - 1) + p] : 0;
            c = (y > 0)? (int)((unsigned char *)data)[scanline_size*(y - 1) + p - ((p > pixel_size)? pixel_size : 0)] : 0;
            
            // Heuristic: Compute the output scanline using all five filters
            // REF: https://www.w3.org/TR/PNG/#9Filters
            for (int filter = 0; filter < 5; filter++)
            {
                switch(filter)
                {
                    case 0: out = x; break;
                    case 1: out = x - a; break;
                    case 2: out = x - b; break;
                    case 3: out = x - ((a + b)>>1); break;
                    case 4: out = x - rpng_paeth_predictor(a, b, c); break;
                    default: break;
                }
            
                sum_value[filter] += abs((signed char)out);
            }
        }
        
        // Select the filter that gives the smallest sum of absolute values of outputs. 
        // NOTE: Considering the output bytes as signed differences for the test.
        best_filter = 0;
        int best_value = sum_value[0];

        for (int filter = 1; filter < 5; filter++)
        {
            if (sum_value[filter] < best_value)
            {
                best_value = sum_value[filter];
                best_filter = filter;
            }
        }

        // Register scanline filter byte
        data_filtered[(scanline_size + 1)*y] = best_filter;
        
        // Apply the best_filter to scanline
        for (int p = 0; p < scanline_size; p++)
        {
            x = (int)((unsigned char *)data)[scanline_size*y + p];
            a = (int)((unsigned char *)data)[scanline_size*y + p - ((p > pixel_size)? pixel_size : 0)];
            b = (y > 0)? (int)((unsigned char *)data)[scanline_size*(y - 1) + p] : 0;
            c = (y > 0)? (int)((unsigned char *)data)[scanline_size*(y - 1) + p - ((p > pixel_size)? pixel_size : 0)] : 0;
            
            switch(best_filter)
            {
                case 0: out = x; break;
                case 1: out = x - a; break;
                case 2: out = x - b; break;
                case 3: out = x - ((a + b)>>1); break;
                case 4: out = x - rpng_paeth_predictor(a, b, c); break;
                default: break;
            }
            
            // Register scanline filtered values, byte by byte
            data_filtered[(scanline_size + 1)*y + 1 + p] = (unsigned char)out;
        }
    }

    // Compress filtered image data and generate a valid zlib stream
    struct sdefl *sde = RPNG_CALLOC(sizeof(struct sdefl), 1);
    int bounds = sdefl_bound(data_size_filtered);
    unsigned char *comp_data = (unsigned char *)RPNG_CALLOC(bounds, 1);
    int comp_data_size = zsdeflate(sde, comp_data, data_filtered, data_size_filtered, 8);   // Compression level 8, same as stbwi
    RPNG_FREE(sde);
    
    printf("Data size: %i -> Comp data size: %i\n", data_size_filtered, comp_data_size);

    // Security check to verify compression worked
    if (comp_data_size > 0)
    {
        // Create the PNG in memory
        unsigned char *file_output = (unsigned char *)RPNG_CALLOC(8 + (13 + 12) + (comp_data_size + 12) + (12), 1); // Signature + IHDR + IDAT + IEND
        int file_output_size = 0;
        
        // Write PNG signature
        memcpy(file_output, png_signature, 8);
        
        // Write PNG chunk IHDR
        unsigned int length_IHDR = 13;
        length_IHDR = swap_endian(length_IHDR);
        memcpy(file_output + 8, &length_IHDR, 4);
        memcpy(file_output + 8 + 4, "IHDR", 4);
        memcpy(file_output + 8 + 4 + 4, &image_info, 13);
        unsigned int crc = compute_crc32(file_output + 8 + 4, 4 + 13);
        crc = swap_endian(crc);
        memcpy(file_output + 8 + 8 + 13, &crc, 4);
        file_output_size += (8 + 12 + 13);
        
        // Write PNG chunk IDAT
        unsigned int length_IDAT = comp_data_size;
        length_IDAT = swap_endian(length_IDAT);
        memcpy(file_output + file_output_size, &length_IDAT, 4);
        memcpy(file_output + file_output_size + 4, "IDAT", 4);
        memcpy(file_output + file_output_size + 8, comp_data, comp_data_size);
        crc = compute_crc32(file_output + file_output_size + 4, 4 + comp_data_size);
        crc = swap_endian(crc);
        memcpy(file_output + file_output_size + 8 + comp_data_size, &crc, 4);
        file_output_size += (comp_data_size + 12);
        
        // Write PNG chunk IEND
        unsigned char chunk_IEND[12] = { 0, 0, 0, 0, 'I', 'E', 'N', 'D', 0xAE, 0x42, 0x60, 0x82 };
        memcpy(file_output + file_output_size, chunk_IEND, 12);
        file_output_size += 12;
        
        save_file_from_buffer(filename, file_output, file_output_size);
        
        RPNG_FREE(file_output);
    }
    
    RPNG_FREE(comp_data);
}

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

    // TODO: Implement proper security check before writing to file
    save_file_from_buffer(filename, file_output, file_output_size);

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

    // TODO: Implement proper security check before writing to file
    save_file_from_buffer(filename, file_output, file_output_size);

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
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write text chunk data into PNG
// NOTE: It will be added just after IHDR chunk
// tEXt chunk data:
//   unsigned char *keyword;     // Keyword: 1-80 bytes (must end with NULL separator: /0)
//   unsigned char *text;        // Text: n bytes (character string, no NULL terminated required)
// Keyword/Text usual values:
//   Title            Short (one line) title or caption for image
//   Author           Name of image's creator
//   Description      Description of image (possibly long)
//   Copyright        Copyright notice
//   Creation Time    Time of original image creation
//   Software         Software used to create the image
//   Disclaimer       Legal disclaimer
//   Warning          Warning of nature of content
//   Source           Device used to create the image
//   Comment          Miscellaneous comment
void rpng_chunk_write_text(const char *filename, char *keyword, char *text)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };
    
    int keyword_len = strlen(keyword);
    int text_len = strlen(text);

    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "tEXt", 4);
    chunk.length = keyword_len + 1 + text_len;
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    memcpy(((unsigned char*)chunk.data), keyword, keyword_len);
    memcpy(((unsigned char*)chunk.data) + keyword_len + 1, text, text_len);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()

    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write zTXt chunk, DEFLATE compressed text
// zTXt chunk information and size:
//    unsigned char *keyword;           // Keyword: 1-80 bytes (must end with NULL separator: /0)
//    unsigned char comp;               // Compression method (0 for DEFLATE)
//    unsigned char *comp_text;         // Compressed text: n bytes
void rpng_chunk_write_comp_text(const char *filename, char *keyword, char *text)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    // Create chunk and fill with data
    rpng_chunk chunk = { 0 };

    int keyword_len = strlen(keyword);
    int text_len = strlen(text);

    // Compress filtered image data and generate a valid zlib stream
    struct sdefl *sde = RPNG_CALLOC(sizeof(struct sdefl), 1);
    int bounds = sdefl_bound(text_len);
    unsigned char *comp_text = (unsigned char *)RPNG_CALLOC(bounds, 1);
    int comp_text_size = zsdeflate(sde, comp_text, (unsigned char *)text, text_len, 8);   // Compression level 8, same as stbwi
    RPNG_FREE(sde);

    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "zTXt", 4);
    chunk.length = keyword_len + 1 + 1 + comp_text_size;
    chunk.data = (unsigned char *)RPNG_CALLOC(chunk.length, 1);
    memcpy(chunk.data, keyword, keyword_len);
    memcpy(chunk.data + keyword_len + 2, comp_text, comp_text_size);
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(comp_text);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write gAMA chunk
// NOTE: Gamma is stored as one int: gamma*100000
void rpng_chunk_write_gamma(const char *filename, float gamma)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };
    
    int gamma_value = (int)(gamma*100000);

    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "gAMA", 4);
    chunk.length = 4;
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    gamma_value = swap_endian(gamma_value);
    memcpy(((unsigned char*)chunk.data), &gamma_value, 4);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write sRGB chunk, requires gAMA chunk
// NOTE: This chunk only contains 1 byte of data defining rendering intent:
//   0: Perceptual
//   1: Relative colorimetric
//   2: Saturation
//   3: Absolute colorimetric
void rpng_chunk_write_srgb(const char *filename, char srgb_type)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };
    
    if ((srgb_type < 0) || (srgb_type > 3)) srgb_type = 0;
    
    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "sRGB", 4);
    chunk.length = 1;
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    memcpy(((unsigned char*)chunk.data), &srgb_type, 1);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write tIME chunk
// tIME chunk information and size:
//   unsigned short year;         // Year complete, i.e. 1995
//   unsigned char month;         // 1 to 12
//   unsigned char day;           // 1 to 31
//   unsigned char hour;          // 0 to 23
//   unsigned char minute;        // 0 to 59
//   unsigned char second;        // 0 to 60 (yes, 60, for leap seconds; not 61, a common error)
void rpng_chunk_write_time(const char *filename, short year, char month, char day, char hour, char min, char sec)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };

    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "tIME", 4);
    chunk.length = 7;
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    memcpy(((unsigned char*)chunk.data), &year, 2);
    memcpy(((unsigned char*)chunk.data) + 2, &month, 1);
    memcpy(((unsigned char*)chunk.data) + 3, &day, 1);
    memcpy(((unsigned char*)chunk.data) + 4, &hour, 1);
    memcpy(((unsigned char*)chunk.data) + 5, &min, 1);
    memcpy(((unsigned char*)chunk.data) + 6, &sec, 1);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write pHYs chunk
// pHYs chunk information and size:
//   unsigned int pixels_per_unit_x;
//   unsigned int pixels_per_unit_y;
//   unsigned char unit_specifier;       // 0 - Unit unknown, 1 - Unit is meter
void rpng_chunk_write_physical_size(const char *filename, int pixels_unit_x, int pixels_unit_y, bool meters)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };
    
    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "pHYs", 4);
    chunk.length = 9;
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    pixels_unit_x = swap_endian(pixels_unit_x);
    memcpy(((unsigned char*)chunk.data), &pixels_unit_x, 4);
    pixels_unit_y = swap_endian(pixels_unit_y);
    memcpy(((unsigned char*)chunk.data) + 4, &pixels_unit_y, 4);
    char meters_value = (meters)? 1 : 0;
    memcpy(((unsigned char*)chunk.data) + 8, &meters_value, 1);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Write cHRM chunk
// cHRM chunk information and size:
//   unsigned int white_point_x;
//   unsigned int white_point_y;
//   unsigned int redx;
//   unsigned int redy;
//   unsigned int greenx;
//   unsigned int greeny;
//   unsigned int bluex;
//   unsigned int bluey;
// NOTE: Each value is stored as one int: value*100000
void rpng_chunk_write_chroma(const char *filename, float white_x, float white_y, float red_x, float red_y, float green_x, float green_y, float blue_x, float blue_y)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    rpng_chunk chunk = { 0 };
    
    // Fill chunk with required data
    // NOTE: CRC can be left to 0, it's calculated internally on writing
    memcpy(chunk.type, "pHYs", 4);
    chunk.length = 8*4;     // 8 integer values
    chunk.data = RPNG_CALLOC(chunk.length, 1);
    int white_x_value = swap_endian((int)(white_x*100000));
    memcpy(((unsigned char*)chunk.data), &white_x_value, 4);
    int white_y_value = swap_endian((int)(white_y*100000));
    memcpy(((unsigned char*)chunk.data) + 4, &white_y_value, 4);
    int red_x_value = swap_endian((int)(red_x*100000));
    memcpy(((unsigned char*)chunk.data) + 8, &red_x_value, 4);
    int red_y_value = swap_endian((int)(red_y*100000));
    memcpy(((unsigned char*)chunk.data) + 12, &red_y_value, 4);
    int green_x_value = swap_endian((int)(green_x*100000));
    memcpy(((unsigned char*)chunk.data) + 16, &green_x_value, 4);
    int green_y_value = swap_endian((int)(green_y*100000));
    memcpy(((unsigned char*)chunk.data) + 20, &green_y_value, 4);
    int blue_x_value = swap_endian((int)(blue_x*100000));
    memcpy(((unsigned char*)chunk.data) + 24, &blue_x_value, 4);
    int blue_y_value = swap_endian((int)(blue_y*100000));
    memcpy(((unsigned char*)chunk.data) + 28, &blue_y_value, 4);
    chunk.crc = 0;  // Computed by rpng_chunk_write_from_memory()
    
    int file_output_size = 0;
    char *file_output = rpng_chunk_write_from_memory(file_data, chunk, &file_output_size);

    // Verify expected output size before writing to file
    if (file_output_size == (file_size + chunk.length + 12)) save_file_from_buffer(filename, file_output, file_output_size);
    else printf("WARNING: Failed to save file, output size not matching expected size");

    RPNG_FREE(chunk.data);
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

// Check chunks CRC and order
bool rpng_chunk_check_all_valid(const char *filename)
{
    bool result = true;
    
    int count = 0;
    rpng_chunk *chunks = rpng_chunk_read_all(filename, &count);
    
    unsigned int crc = 0;
    char *chunk_type_data = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);
    
    for (int i = 0; i < count; i++)
    {
        memcpy(chunk_type_data, chunks[i].type, 4);
        memcpy(chunk_type_data + 4, chunks[i].data, chunks[i].length);
        crc = compute_crc32((unsigned char *)chunk_type_data, 4 + chunks[i].length);
        crc = swap_endian(crc);
        
        // Check computed CRC matches provided CRC
        if (chunks[i].crc != crc) 
        {
            result = false;
            break;
        }
    }

    // Free chunks memory
    for (int i = 0; i < count; i++) RPNG_FREE(chunks[i].data);
    RPNG_FREE(chunks);
    
    return result;
}

// Combine multiple IDAT chunks into a single one
void rpng_chunk_combine_image_data(const char *filename)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_combine_image_data_from_memory(file_data, &file_output_size);

    // TODO: Implement proper security check before writing to file
    save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Split one IDAT chunk into multiple ones
void rpng_chunk_split_image_data(const char *filename, int split_size)
{
    int file_size = 0;
    char *file_data = load_file_to_buffer(filename, &file_size);

    int file_output_size = 0;
    char *file_output = rpng_chunk_split_image_data_from_memory(file_data, split_size, &file_output_size);

    // TODO: Implement proper security check before writing to file
    if (file_output_size > file_size) save_file_from_buffer(filename, file_output, file_output_size);

    RPNG_FREE(file_output);
    RPNG_FREE(file_data);
}

// Functions operating on memory buffers data
//----------------------------------------------------------------------------------------------------------

// Count the chunks in a PNG image from memory buffer
int rpng_chunk_count_from_memory(const char *buffer)
{
    char *buffer_ptr = (char *)buffer;
    int count = 0;

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            buffer_ptr += (4 + 4 + chunk_size + 4);    // Skip chunk Length + FOURCC + chunk data + CRC32

            chunk_size = swap_endian(((int *)buffer_ptr)[0]);
            count++;
        }

        count++; // IEND chunk!
    }

    return count;
}

// Read one chunk type from memory buffer
rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type)
{
    char *buffer_ptr = (char *)buffer;
    rpng_chunk chunk = { 0 };

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        buffer_ptr += 8;   // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            if (memcmp(buffer_ptr + 4, chunk_type, 4) == 0)
            {
                chunk.length = chunk_size;
                memcpy(chunk.type, (char *)(buffer_ptr + 4), 4);
                chunk.data = RPNG_MALLOC(chunk_size);
                memcpy(chunk.data, buffer_ptr + 8, chunk_size);
                chunk.crc = swap_endian(((unsigned int *)(buffer_ptr + 8 + chunk_size))[0]);

                break;
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);           // Move pointer to next chunk of input data
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);  // Compute next chunk file_size
        }
    }

    return chunk;
}

// Read all chunks from memory buffer
rpng_chunk *rpng_chunk_read_all_from_memory(const char *buffer, int *count)
{
    char *buffer_ptr = (char *)buffer;
    rpng_chunk *chunks = NULL;
    int counter = 0;

    // NOTE: We check minimum file_size for a PNG (Signature + chunk IHDR + chunk IDAT + chunk IEND)
    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        // We allocate enough space for 64 chunks
        chunks = (rpng_chunk *)RPNG_CALLOC(RPNG_MAX_CHUNKS_COUNT, sizeof(rpng_chunk));

        buffer_ptr += 8;                       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            chunks[counter].length = chunk_size;
            memcpy(chunks[counter].type, (char *)(buffer_ptr + 4), 4);
            chunks[counter].data = RPNG_MALLOC(chunk_size);
            memcpy(chunks[counter].data, buffer_ptr + 8, chunk_size);
            chunks[counter].crc = swap_endian(((unsigned int *)(buffer_ptr + 8 + chunk_size))[0]);

            buffer_ptr += (4 + 4 + chunk_size + 4);   // Move pointer to next chunk
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);

            counter++;
            if (counter >= (RPNG_MAX_CHUNKS_COUNT - 2)) break;   // WARNING: Too many chunks!
        }

        // Read final IEND chunk
        chunks[counter].length = chunk_size;
        memcpy(chunks[counter].type, (char *)(buffer_ptr + 4), 4);
        chunks[counter].data = RPNG_MALLOC(chunk_size);
        memcpy(chunks[counter].data, buffer_ptr + 8, chunk_size);
        chunks[counter].crc = swap_endian(((unsigned int *)(buffer_ptr + 8 + chunk_size))[0]);

        counter++;
    }

    // Reallocate chunks file_size
    rpng_chunk *chunks_resized = RPNG_REALLOC(chunks, counter*sizeof(rpng_chunk));
    if (chunks_resized != NULL) chunks = chunks_resized;

    *count = counter;
    return chunks;
}

// Remove one chunk type from memory buffer
// NOTE: returns output_data and output_size through parameter 
char *rpng_chunk_remove_from_memory(const char *buffer, const char *chunk_type, int *output_size)
{
    char *buffer_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int output_buffer_size = 0;

    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        output_buffer_size += 8;
        
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            // If chunk type is not the requested, just copy input data into output buffer
            if (memcmp(buffer_ptr + 4, chunk_type, 4) != 0)
            {
                memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + chunk_size + 4);  // Length + FOURCC + chunk_size + CRC32
                output_buffer_size += (4 + 4 + chunk_size + 4);
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);   // Move pointer to next chunk
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);
        }

        // Write IEND chunk
        memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + 4);
        output_buffer_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, output_buffer_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = output_buffer_size;
    return output_buffer;
}

// Remove all chunks from memory buffer except: IHDR-IDAT-IEND
// NOTE: returns output_data and output_size through parameter 
char *rpng_chunk_remove_ancillary_from_memory(const char *buffer, int *output_size)
{
    char *buffer_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int output_buffer_size = 0;

    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        bool preserve_palette_transparency = false;
        
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        output_buffer_size += 8;
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            if (memcmp(buffer_ptr + 4, "PLTE", 4) == 0) preserve_palette_transparency = true;
            
            // If chunk type is mandatory, just copy input data into output buffer
            if ((memcmp(buffer_ptr + 4, "IHDR", 4) == 0) ||
                (memcmp(buffer_ptr + 4, "PLTE", 4) == 0) ||
                (memcmp(buffer_ptr + 4, "IDAT", 4) == 0) ||
                (preserve_palette_transparency && (memcmp(buffer_ptr + 4, "tRNS", 4) == 0)))
            {
                memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + chunk_size + 4);  // Length + FOURCC + chunk_size + CRC32
                output_buffer_size += (4 + 4 + chunk_size + 4);
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);   // Move pointer to next chunk
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);
        }

        // Write IEND chunk
        memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + 4);
        output_buffer_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, output_buffer_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = output_buffer_size;
    return output_buffer;
}

// Write one new chunk after IHDR (any kind) to memory buffer
// NOTE: returns output data file_size
char *rpng_chunk_write_from_memory(const char *buffer, rpng_chunk chunk, int *output_size)
{
    char *buffer_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int output_buffer_size = 0;

    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        output_buffer = RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);
        
        memcpy(output_buffer, png_signature, 8);        // Copy PNG signature
        output_buffer_size += 8;
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0) // While IEND chunk not reached
        {
            memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + chunk_size + 4);  // Length + FOURCC + chunk_size + CRC32
            output_buffer_size += (4 + 4 + chunk_size + 4);

            // Check if we just copied the IHDR chunk to append our chunk after it
            if (memcmp(buffer_ptr + 4, "IHDR", 4) == 0)
            {
                int chunk_length_be = swap_endian(chunk.length);
                memcpy(output_buffer + output_buffer_size, &chunk_length_be, sizeof(int));           // Write chunk length
                memcpy(output_buffer + output_buffer_size + 4, chunk.type, 4);                // Write chunk type
                memcpy(output_buffer + output_buffer_size + 4 + 4, chunk.data, chunk.length); // Write chunk data

                unsigned char *type_data = RPNG_MALLOC(4 + chunk.length);
                memcpy(type_data, chunk.type, 4);
                memcpy(type_data + 4, chunk.data, chunk.length);
                unsigned int crc = compute_crc32(type_data, 4 + chunk.length);
                crc = swap_endian(crc);
                memcpy(output_buffer + output_buffer_size + 4 + 4 + chunk.length, &crc, 4);   // Write CRC32 (computed over type + data)
                
                RPNG_FREE(type_data);

                output_buffer_size += (4 + 4 + chunk.length + 4);  // Update output file file_size with new chunk
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);           // Move pointer to next chunk of input data
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);  // Compute next chunk file_size
        }

        // Write IEND chunk
        memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + 4);
        output_buffer_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = RPNG_REALLOC(output_buffer, output_buffer_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = output_buffer_size;
    return output_buffer;
}

// Combine multiple IDAT chunks into a single one
char *rpng_chunk_combine_image_data_from_memory(char *buffer, int *output_size)
{
    char *buffer_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int output_buffer_size = 0;

    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        char *idata_buffer = (char *)RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);    // Output buffer allocation
        memcpy(idata_buffer, "IDAT", 4);
        int idata_buffer_size = 0;
        output_buffer = (char *)RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);               // Copy PNG signature
        output_buffer_size += 8;
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0)         // While IEND chunk not reached
        {
            // If IDAT chunk just copy data into idata_buffer
            if ((memcmp(buffer_ptr + 4, "IDAT", 4) == 0))
            {
                memcpy(idata_buffer + 4 + idata_buffer_size, buffer_ptr + 8, chunk_size);
                idata_buffer_size += chunk_size;
            }
            else
            {
                memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + chunk_size + 4);  // Length + FOURCC + chunk_size + CRC32
                output_buffer_size += (4 + 4 + chunk_size + 4);
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);   // Move pointer to next chunk
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);
        }
        
        // Write IDAT combined chunk
        unsigned int idata_buffer_size_be = swap_endian(idata_buffer_size);
        memcpy(output_buffer + output_buffer_size, &idata_buffer_size_be, 4);
        memcpy(output_buffer + output_buffer_size + 4, idata_buffer, 4 + idata_buffer_size);
        unsigned int crc = compute_crc32((unsigned char *)idata_buffer, 4 + idata_buffer_size);
        crc = swap_endian(crc);
        memcpy(output_buffer + output_buffer_size + 4 + 4 + idata_buffer_size, &crc, 4);
        RPNG_FREE(idata_buffer);
        
        output_buffer_size += (idata_buffer_size + 12);

        // Write IEND chunk
        memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + 4);
        output_buffer_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = (char *)RPNG_REALLOC(output_buffer, output_buffer_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }

    *output_size = output_buffer_size;
    return output_buffer;
}

// Split one IDAT chunk into multiple ones
char *rpng_chunk_split_image_data_from_memory(char *buffer, int split_size, int *output_size)
{
    char *buffer_ptr = (char *)buffer;
    char *output_buffer = NULL;
    int output_buffer_size = 0;
    
    if ((buffer_ptr != NULL) && (memcmp(buffer_ptr, png_signature, 8) == 0))  // Check valid PNG file
    {
        char *idata_split_buffer = (char *)RPNG_CALLOC(split_size + 12, 1);    // Output buffer allocation
        
        output_buffer = (char *)RPNG_CALLOC(RPNG_MAX_OUTPUT_SIZE, 1);  // Output buffer allocation
        
        memcpy(output_buffer, png_signature, 8);    // Copy PNG signature
        output_buffer_size += 8;
        buffer_ptr += 8;       // Move pointer after signature

        unsigned int chunk_size = swap_endian(((int *)buffer_ptr)[0]);

        while (memcmp(buffer_ptr + 4, "IEND", 4) != 0)         // While IEND chunk not reached
        {
            // If IDAT chunk, split into multiple sized chunks
            if ((memcmp(buffer_ptr + 4, "IDAT", 4) == 0) && (chunk_size > (unsigned int)split_size))
            {
                // Split chunk into pieces!
                unsigned int chunk_remain_size = chunk_size;
                char *buffer_ptr_offset = buffer_ptr + 4 + 4;
                
                while (chunk_remain_size > (unsigned int)split_size)
                {
                    unsigned int split_size_be = swap_endian(split_size);
                    memcpy(idata_split_buffer, &split_size_be, 4);
                    memcpy(idata_split_buffer + 4, "IDAT", 4);
                    memcpy(idata_split_buffer + 4 + 4, buffer_ptr_offset, split_size);
                    unsigned int crc = compute_crc32((unsigned char *)(idata_split_buffer + 4), 4 + split_size);
                    crc = swap_endian(crc);
                    memcpy(idata_split_buffer + 4 + 4 + split_size, &crc, 4);
                    
                    chunk_remain_size -= split_size;
                    buffer_ptr_offset += split_size;
                    
                    memcpy(output_buffer + output_buffer_size, idata_split_buffer, split_size + 12);
                    output_buffer_size += (split_size + 12);
                }
                
                // Save last IDAT chunk piece
                unsigned int chunk_remain_size_be = swap_endian(chunk_remain_size);
                memcpy(idata_split_buffer, &chunk_remain_size_be, 4);
                memcpy(idata_split_buffer + 4, "IDAT", 4);
                memcpy(idata_split_buffer + 4 + 4, buffer_ptr_offset, chunk_remain_size);
                unsigned int crc = compute_crc32((unsigned char *)(idata_split_buffer + 4), 4 + chunk_remain_size);
                crc = swap_endian(crc);
                memcpy(idata_split_buffer + 4 + 4 + chunk_remain_size, &crc, 4);
                
                memcpy(output_buffer + output_buffer_size, idata_split_buffer, chunk_remain_size + 12);
                output_buffer_size += (chunk_remain_size + 12);
            }
            else
            {
                memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + chunk_size + 4);  // Length + FOURCC + chunk_size + CRC32
                output_buffer_size += (4 + 4 + chunk_size + 4);
            }

            buffer_ptr += (4 + 4 + chunk_size + 4);   // Move pointer to next chunk
            chunk_size = swap_endian(((int *)buffer_ptr)[0]);
        }
        
        RPNG_FREE(idata_split_buffer);

        // Write IEND chunk
        memcpy(output_buffer + output_buffer_size, buffer_ptr, 4 + 4 + 4);
        output_buffer_size += 12;
        
        // Resize output buffer
        char *output_buffer_sized = (char *)RPNG_REALLOC(output_buffer, output_buffer_size);
        if (output_buffer_sized != NULL) output_buffer = output_buffer_sized;
    }
    
    *output_size = output_buffer_size;
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
    #warning No FILE I/O API, RPNG_NO_STDIO defined
#endif
    return data;
}

// Write data to file from buffer
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
    #warning No FILE I/O API, RPNG_NO_STDIO defined
#endif
}

//=========================================================================
//                              SDEFL
// DEFLATE algorythm implementation: https://github.com/vurtun/lib/sdefl.h
//=========================================================================

#define SDEFL_ZLIB_HDR      (0x01)

static const unsigned char sdefl_mirror[256] = {
    #define R2(n) n, n + 128, n + 64, n + 192
    #define R4(n) R2(n), R2(n + 32), R2(n + 16), R2(n + 48)
    #define R6(n) R4(n), R4(n +  8), R4(n +  4), R4(n + 12)
    R6(0), R6(2), R6(1), R6(3),
};
static unsigned
sdefl_adler32(unsigned adler32, const unsigned char *in, int in_len)
{
    #define SDEFL_ADLER_INIT  (1)
    const unsigned ADLER_MOD = 65521;
    unsigned s1 = adler32 & 0xffff;
    unsigned s2 = adler32 >> 16;
    unsigned blk_len, i;

    blk_len = in_len % 5552;
    while (in_len) {
        for (i=0; i + 7 < blk_len; i += 8) {
            s1 += in[0]; s2 += s1;
            s1 += in[1]; s2 += s1;
            s1 += in[2]; s2 += s1;
            s1 += in[3]; s2 += s1;
            s1 += in[4]; s2 += s1;
            s1 += in[5]; s2 += s1;
            s1 += in[6]; s2 += s1;
            s1 += in[7]; s2 += s1;
            in += 8;
        }
        for (; i < blk_len; ++i)
            s1 += *in++, s2 += s1;
        s1 %= ADLER_MOD; s2 %= ADLER_MOD;
        in_len -= blk_len;
        blk_len = 5552;
    } return (unsigned)(s2 << 16) + (unsigned)s1;
}
static int
sdefl_npow2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return (int)++n;
}
static int
sdefl_ilog2(int n)
{
    #define lt(n) n,n,n,n, n,n,n,n, n,n,n,n ,n,n,n,n
    static const char tbl[256] = {-1,0,1,1,2,2,2,2,3,3,3,3,
        3,3,3,3,lt(4),lt(5),lt(5),lt(6),lt(6),lt(6),lt(6),
        lt(7),lt(7),lt(7),lt(7),lt(7),lt(7),lt(7),lt(7)
    }; int tt, t;
    if ((tt = (n >> 16)))
        return (t = (tt >> 8)) ? 24+tbl[t]: 16+tbl[tt];
    else return (t = (n >> 8)) ? 8+tbl[t]: tbl[n];
    #undef lt
}
static unsigned
sdefl_uload32(const void *p)
{
    /* hopefully will be optimized to an unaligned read */
    unsigned int n = 0;
    memcpy(&n, p, sizeof(n));
    return n;
}
static unsigned
sdefl_hash32(const void *p)
{
    unsigned n = sdefl_uload32(p);
    return (n*0x9E377989)>>(32-SDEFL_HASH_BITS);
}
static unsigned char*
sdefl_put(unsigned char *dst, struct sdefl *s, int code, int bitcnt)
{
    s->bits |= (code << s->cnt);
    s->cnt += bitcnt;
    while (s->cnt >= 8) {
        *dst++ = (unsigned char)(s->bits & 0xFF);
        s->bits >>= 8;
        s->cnt -= 8;
    } return dst;
}
static unsigned char*
sdefl_match(unsigned char *dst, struct sdefl *s, int dist, int len)
{
    static const short lxmin[] = {0,11,19,35,67,131};
    static const short dxmax[] = {0,6,12,24,48,96,192,384,768,1536,3072,6144,12288,24576};
    static const short lmin[] = {11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227};
    static const short dmin[] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,
        385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};

    /* length encoding */
    int lc = len;
    int lx = sdefl_ilog2(len - 3) - 2;
    if (!(lx = (lx < 0) ? 0: lx)) lc += 254;
    else if (len >= 258) lx = 0, lc = 285;
    else lc = ((lx-1) << 2) + 265 + ((len - lxmin[lx]) >> lx);

    if (lc <= 279)
        dst = sdefl_put(dst, s, sdefl_mirror[(lc - 256) << 1], 7);
    else dst = sdefl_put(dst, s, sdefl_mirror[0xc0 - 280 + lc], 8);
    if (lx) dst = sdefl_put(dst, s, len - lmin[lc - 265], lx);

    /* distance encoding */
    {int dc = dist - 1;
    int dx = sdefl_ilog2(sdefl_npow2(dist) >> 2);
    if ((dx = (dx < 0) ? 0: dx))
        dc = ((dx + 1) << 1) + (dist > dxmax[dx]);
    dst = sdefl_put(dst, s, sdefl_mirror[dc << 3], 5);
    if (dx) dst = sdefl_put(dst, s, dist - dmin[dc], dx);}
    return dst;
}
static unsigned char*
sdefl_lit(unsigned char *dst, struct sdefl *s, int c)
{
    if (c <= 143)
        return sdefl_put(dst, s, sdefl_mirror[0x30+c], 8);
    else return sdefl_put(dst, s, 1 + 2 * sdefl_mirror[0x90 - 144 + c], 9);
}
static int
sdefl_compr(struct sdefl *s, unsigned char *out,
    const unsigned char *in, int in_len, int lvl, unsigned flags)
{
    int p = 0;
    unsigned char *q = out;
    int max_chain = (lvl < 8) ? (1<<(lvl+1)): (1<<13);

    s->bits = s->cnt = 0;
    for (p = 0; p < SDEFL_HASH_SIZ; ++p)
        s->tbl[p] = SDEFL_NIL;

    if (flags & SDEFL_ZLIB_HDR) {
        q = sdefl_put(q, s, 0x78, 8); /* compression method: deflate, 32k window */
        q = sdefl_put(q, s, 0xda, 8); /* maximum compression */
    }
    q = sdefl_put(q, s, 0x01, 1); /* block */
    q = sdefl_put(q, s, 0x01, 2); /* static huffman */

    p = 0;
    while (p < in_len) {
        int run, best_len = 0, dist = 0;
        int max_match = ((in_len-p)>SDEFL_MAX_MATCH) ? SDEFL_MAX_MATCH:(in_len-p);
        if (max_match > SDEFL_MIN_MATCH) {
            int chain_len = max_chain;
            int limit = ((p-SDEFL_WIN_SIZ)<SDEFL_NIL)?SDEFL_NIL:(p-SDEFL_WIN_SIZ);
            int i = s->tbl[sdefl_hash32(&in[p])];
            while (i > limit) {
                if (in[i+best_len] == in[p+best_len] &&
                    (sdefl_uload32(&in[i]) == sdefl_uload32(&in[p]))){
                    int n = SDEFL_MIN_MATCH;
                    while (n < max_match && in[i+n] == in[p+n]) n++;
                    if (n > best_len) {
                        best_len = n;
                        dist = p - i;
                        if (n == max_match)
                            break;
                    }
                }
                if (!(--chain_len)) break;
                i = s->prv[i&SDEFL_WIN_MSK];
            }
        }
        if (lvl >= 5 && best_len >= SDEFL_MIN_MATCH && best_len < max_match){
            const int x = p + 1;
            int chain_len = max_chain;
            int tar_len = best_len + 1;
            int limit = ((x-SDEFL_WIN_SIZ)<SDEFL_NIL)?SDEFL_NIL:(x-SDEFL_WIN_SIZ);
            int i = s->tbl[sdefl_hash32(&in[p])];
            while (i > limit) {
                if (in[i+best_len] == in[x+best_len] &&
                    (sdefl_uload32(&in[i]) == sdefl_uload32(&in[x]))){
                    int n = SDEFL_MIN_MATCH;
                    while (n < tar_len && in[i+n] == in[x+n]) n++;
                    if (n == tar_len) {
                        best_len = 0;
                        break;
                    }
                }
                if (!(--chain_len)) break;
                i = s->prv[i&SDEFL_WIN_MSK];
            }
        }
        if (best_len >= SDEFL_MIN_MATCH) {
            q = sdefl_match(q, s, dist, best_len);
            run = best_len;
        } else {
            q = sdefl_lit(q, s, in[p]);
            run = 1;
        }
        while (run-- != 0) {
            unsigned h = sdefl_hash32(&in[p]);
            s->prv[p&SDEFL_WIN_MSK] = s->tbl[h];
            s->tbl[h] = p++;
        }
    }
    q = sdefl_put(q, s, 0, 7); /* end of block */
    if (s->cnt) /* flush out all remaining bits */
        q = sdefl_put(q, s, 0, 8 - s->cnt);

    if (flags & SDEFL_ZLIB_HDR) {
        /* optionally append adler checksum */
        unsigned a = sdefl_adler32(SDEFL_ADLER_INIT, in, in_len);
        for (p = 0; p < 4; ++p) {
            q = sdefl_put(q, s, (a>>24)&0xFF, 8);
            a <<= 8;
        }
    } return (int)(q - out);
}
extern int
sdeflate(struct sdefl *s, unsigned char *out,
    const unsigned char *in, int in_len, int lvl)
{
    return sdefl_compr(s, out, in, in_len, lvl, 0u);
}
extern int
zsdeflate(struct sdefl *s, unsigned char *out,
    const unsigned char *in, int in_len, int lvl)
{
    return sdefl_compr(s, out, in, in_len, lvl, SDEFL_ZLIB_HDR);
}
extern int
sdefl_bound(int len)
{
    int a = 128 + (len * 110) / 100;
    int b = 128 + len + ((len / (31 * 1024)) + 1) * 5;
    return (a > b) ? a : b;
}

/*
------------------------------------------------------------------------------
sdefl software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Micha Mettke
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

#endif  // RPNG_IMPLEMENTATION
