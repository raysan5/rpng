![](logo/rpng_256x256.png)

**rpng** is a simple and easy-to-use library to manage png chunks.

## features

 - Count/read/write/remove png chunks
 - Chunks data abstraction (`png_chunk` type)
 - Create png file from raw pixel data
 - Operates on file or memory-buffer
 - Minimal `libc` usage and `RPNG_NO_STDIO` supported
 
## rpng basic file API
```c
// Create a PNG file from image data (IHDR, IDAT, IEND)
void rpng_create_image(const char *filename, const char *data, int width, int height, int color_channels, int bit_depth);

// Read and write chunks from file
int rpng_chunk_count(const char *filename);                                  // Count the chunks in a PNG image
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);    // Read one chunk type
rpng_chunk *rpng_chunk_read_all(const char *filename, int *count);           // Read all chunks
void rpng_chunk_remove(const char *filename, const char *chunk_type);        // Remove one chunk type
void rpng_chunk_remove_ancillary(const char *filename);                      // Remove all chunks except: IHDR-IDAT-IEND
void rpng_chunk_write(const char *filename, rpng_chunk data);                // Write one new chunk after IHDR (any kind)

// Chunk utilities
void rpng_chunk_print_info(const char *filename);                            // Output info about the chunks
bool rpng_chunk_check_all_valid(const char *filename);                       // Check chunks CRC is valid
void rpng_chunk_combine_image_data(const char *filename);                    // Combine multiple IDAT chunks into a single one
void rpng_chunk_split_image_data(const char *filename, int split_size);      // Split one IDAT chunk into multiple ones
```

## some notes on API design
`rpng` includes and advanced API to work directly on memory buffers, to avoid direct file access or allow virtual file systems.
Those functions share the same signature than file functions but add a `_from_memory()` suffix and receive the memory buffer instead of the filename. Here an example:
```c
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);            // Read one chunk type
```
```c
rpng_chunk rpng_chunk_read_from_memory(const char *buffer, const char *chunk_type);  // Read one chunk type from memory
```
Here you can note an important detail, memory function does not receive the size of the buffer. It was a design decision.
It is expected that user provides valid data and data is validated following PNG specs (png magic number, chunks data, IEND closing chunk).

## usage example
There is a complete example [here](https://github.com/raysan5/rpng/blob/master/example/rpng_test_suite.c).
```c
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

## license

rpng is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

