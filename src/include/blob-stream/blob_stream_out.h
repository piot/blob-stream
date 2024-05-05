/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef BLOB_STREAM_OUT_H
#define BLOB_STREAM_OUT_H

#include <bit-array/bit_array.h>
#include <blob-stream/types.h>
#include <clog/clog.h>
#include <monotonic-time/monotonic_time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct ImprintAllocatorWithFree;
struct ImprintAllocator;

typedef struct BlobStreamOutEntry {
    const uint8_t* octets;
    size_t octetCount;
    BlobStreamChunkId chunkId;
    MonotonicTimeMs lastSentAtTime;
    size_t sendCount;
    bool isReceived;
} BlobStreamOutEntry;

typedef struct BlobStreamOut {
    size_t fixedChunkSize;
    size_t octetCount;
    size_t chunkCount;
    size_t sentChunkEntryCount;
    const uint8_t* blob;
    bool isComplete;
    BlobStreamOutEntry* entries;
    struct ImprintAllocatorWithFree* blobAllocator;
    MonotonicTimeMs thresholdForRedundancy;
    Clog log;
} BlobStreamOut;

void blobStreamOutInit(BlobStreamOut* self, struct ImprintAllocator* allocator,
                       struct ImprintAllocatorWithFree* blobAllocator, const uint8_t* octets, size_t totalOctetCount,
                       size_t fixedChunkSize, Clog log);
void blobStreamOutDestroy(BlobStreamOut* self);
void blobStreamOutReset(BlobStreamOut* self);
bool blobStreamOutIsComplete(const BlobStreamOut* self);
bool blobStreamOutIsAllSent(const BlobStreamOut* self);
void blobStreamOutMarkReceived(BlobStreamOut* self, BlobStreamChunkId everythingBeforeThis, BitArrayAtom maskReceived);
int blobStreamOutGetChunksToSend(BlobStreamOut* self, MonotonicTimeMs now, const BlobStreamOutEntry** resultEntries,
                                 size_t maxEntriesCount);
const char* blobStreamOutToString(const BlobStreamOut* self, char* buf, size_t maxBuf);

#endif
