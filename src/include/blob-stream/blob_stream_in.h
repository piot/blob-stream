#ifndef BLOB_STREAM_IN_H
#define BLOB_STREAM_IN_H

#include <stdint.h>
#include <stdlib.h>

#include <bit-array/bit_array.h>
#include <blob-stream/types.h>

struct ImprintAllocator;
struct ImprintAllocatorWithFree;

typedef struct BlobStreamIn {
    BitArray bitArray;
    size_t fixedChunkSize;
    size_t octetCount;
    uint8_t* blob;
    int isComplete;
    struct ImprintAllocatorWithFree* blobAllocator;
} BlobStreamIn;


void blobStreamInInit(BlobStreamIn* self, struct ImprintAllocator* memory, struct ImprintAllocatorWithFree* blobAllocator, size_t totalOctetCount, size_t fixedChunkSize);
void blobStreamInDestroy(BlobStreamIn* self);
void blobStreamInReset(BlobStreamIn* self);
int blobStreamInIsComplete(const BlobStreamIn* self);
void blobStreamInSetChunk(BlobStreamIn* self, BlobStreamChunkId chunkId, const uint8_t* octets, size_t octetCount);
const char* blobStreamInToString(const BlobStreamIn* self, char* buf, size_t maxBuf);

#endif