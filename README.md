<img align="left" src="https://github.com/raysan5/rpng/blob/master/logo/rpng_256x256.png" width=256>

**rpng is a simple and easy-to-use library to manage png chunks.**

`rpng` is provided as a self-contained portable single-file header-only library with no external dependencies. Its only dependency, the standard C library, can also be replaced with a custom implementation if required.

`rpng` implements internally the `DEFLATE` algorythm to allow reading and writing PNG images when required.

<br>
<br>

## features

 - Load and save png files
 - Count/read/write/remove png chunks
 - Operates on file or memory-buffer
 - Chunks data abstraction (`png_chunk` type)
 - Minimal `libc` usage and `RPNG_NO_STDIO` supported
 
## basic functions
```c
// Create a PNG file from image data (IHDR, IDAT, IEND)
void rpng_create_image(const char *filename, const char *data, int width, int height, int color_channels, int bit_depth);
char *rpng_load_image(const char *filename, int *width, int *height, int *color_channels, int force_channels);

// Read and write chunks from file
int rpng_chunk_count(const char *filename);                                  // Count the chunks in a PNG image
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);    // Read one chunk type
rpng_chunk *rpng_chunk_read_all(const char *filename, int *count);           // Read all chunks
void rpng_chunk_remove(const char *filename, const char *chunk_type);        // Remove one chunk type
void rpng_chunk_remove_ancillary(const char *filename);                      // Remove all chunks except: IHDR-PLTE-IDAT-IEND
void rpng_chunk_write(const char *filename, rpng_chunk data);                // Write one new chunk after IHDR (any kind)

// Chunk utilities
void rpng_chunk_print_info(const char *filename);                            // Output info about the chunks
bool rpng_chunk_check_all_valid(const char *filename);                       // Check chunks CRC is valid
void rpng_chunk_combine_image_data(const char *filename);                    // Combine multiple IDAT chunks into a single one
void rpng_chunk_split_image_data(const char *filename, int split_size);      // Split one IDAT chunk into multiple ones
```

## design notes
`rpng` includes an advanced API to work directly on memory buffers, to avoid direct file access or allow virtual file systems.
Those functions share the same signature than file functions but add a `_from_memory()` suffix and receive the memory buffer instead of the filename, some of them also return the file output size. Here an example:
```c
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);            // Read one chunk type
```
```c
rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type);  // Read one chunk type from memory
```
*Note an important detail:* memory functions do not receive the size of the buffer. It was a design decision.
Data is validated following PNG specs (png magic number, chunks data, IEND closing chunk) but it's expected that user provides valid data.

Memory functions that require writing data, return the output buffer size as a parameter: `int *output_size` and are limited in size by `RPNG_MAX_OUTPUT_SIZE` definition, by default **32MB**, redefine that number if dealing with bigger PNG images.

## usage example

Write a custom data chunk into a png file:
```c
#define RPNG_IMPLEMENTATION
#include "rpng.h"

int main()
{
    CustomDataType customData = { 0 };

    // TODO: Fill your custom data

    rpng_chunk chunk = { 0 };
    memcpy(chunk.type, "cSTm", 4);
    chunk.length = sizeof(CustomDataType);
    chunk.data = &customData;
    chunk.crc = 0;   // Automatically computed on writing

    rpng_chunk_write("my_image.png", chunk);  // Write custom chunk
}
```

Several chunk operations on a command line input file:
```c
#define RPNG_IMPLEMENTATION
#include "rpng.h"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // TEST: Print chunks info
        rpng_chunk_print_info(argv[1]);

        // TEST: Write tEXt chunk
        rpng_chunk_write_text(argv[1], "Description", "rpng, library to manage png chunks");

        // TEST: Remove tEXt chunk
        rpng_chunk_remove(argv[1], "tEXt");
        
        // TEST: Remove all ancillary chunks
        rpng_chunk_remove_ancillary(argv[1]);
    }
    else printf("WARNING: No input file provided.\n");

    return 0;
}
```

There is a complete example [here](https://github.com/raysan5/rpng/blob/master/example/rpng_test_suite.c).

## license

rpng is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

