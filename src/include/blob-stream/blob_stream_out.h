#ifndef BLOB_STREAM_OUT_H
#define BLOB_STREAM_OUT_H

#include <stdint.h>
#include <stdlib.h>

#include <bit-array/bit_array.h>
#include <blob-stream/types.h>
#include <monotonic-time/monotonic_time.h>

struct ImprintAllocatorWithFree;
struct ImprintAllocator;

typedef struct BlobStreamOutEntry {
    const uint8_t* octets;
    size_t octetCount;
    BlobStreamChunkId chunkId;
    MonotonicTimeMs lastSentAtTime;
    size_t sendCount;
    int isReceived;
} BlobStreamOutEntry;

typedef struct BlobStreamOut {
    size_t fixedChunkSize;
    size_t octetCount;
    size_t chunkCount;
    const uint8_t* blob;
    int isComplete;
    BlobStreamOutEntry* entries;
    struct ImprintAllocatorWithFree* blobAllocator;
} BlobStreamOut;


void blobStreamOutInit(BlobStreamOut* self, struct ImprintAllocator* allocator,  struct ImprintAllocatorWithFree* blobAllocator, const uint8_t* octets, size_t totalOctetCount, size_t fixedChunkSize);
void blobStreamOutDestroy(BlobStreamOut* self);
void blobStreamOutReset(BlobStreamOut* self);
int blobStreamOutIsComplete(const BlobStreamOut* self);
void blobStreamOutMarkReceived(BlobStreamOut *self, BlobStreamChunkId everythingBeforeThis, BitArrayAtom maskReceived);
int blobStreamOutGetChunksToSend(BlobStreamOut *self, MonotonicTimeMs now, const BlobStreamOutEntry **resultEntries, size_t maxEntriesCount);
const char* blobStreamOutToString(const BlobStreamOut* self, char* buf, size_t maxBuf);

#endif
