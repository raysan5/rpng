![](logo/rpng_256x256.png)

**rpng** is a simple and easy-to-use library to manage png chunks

## features

 - Count/read/write/remove png chunks
 - Chunks data abstraction (`png_chunk` type)
 - Chunk types provided for convenience
 - Operates on file or file-buffer
 - Minimal `libc` usage and `RPNG_NO_STDIO` supported
 
## rpng basic file API
```c
int rpng_chunk_count(const char *filename);                                  // Count the chunks in a PNG image
rpng_chunk rpng_chunk_read(const char *filename, const char *chunk_type);    // Read one chunk type
rpng_chunk *rpng_chunk_read_all(const char *filename, int *count);           // Read all chunks
void rpng_chunk_remove(const char *filename, const char *chunk_type);        // Remove one chunk type
void rpng_chunk_remove_ancillary(const char *filename);                      // Remove all chunks except: IHDR-PLTE-IDAT-IEND
void rpng_chunk_write(const char *filename, rpng_chunk data);                // Write one new chunk after IHDR (any kind)
void rpng_chunk_write_text(const char *filename, char *keyword, char *text); // Write tEXt chunk
void rpng_chunk_print_info(const char *filename);                            // Output info about the chunks
```

## license

rpng is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

