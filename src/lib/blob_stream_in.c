/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/

#include <blob-stream/blob_stream_in.h>
#include <imprint/tagged_allocator.h>

/// Initialize a blob stream
/// Allocates memory for a blob stream which is later defined by calling
/// blobStreamInSetChunk().
/// @param self incoming blob stream
/// @param memory allocator for things that are not explicitly freed
/// @param blobAllocator allocator for the target blob, freed in
/// blobStreamInDestroy()
/// @param octetCount the total size of the blob to be received.
/// @param fixedChunkSize the size of each chunk. Only the last chunk is allowed
/// to have a different size.
void blobStreamInInit(BlobStreamIn* self, struct ImprintAllocator* memory,
                      struct ImprintAllocatorWithFree* blobAllocator, size_t octetCount, size_t fixedChunkSize,
                      Clog log)
{
    self->log = log;
    self->blob = IMPRINT_ALLOC((ImprintAllocator*) blobAllocator, octetCount, "blob stream in payload");
    self->octetCount = octetCount;
    self->fixedChunkSize = fixedChunkSize;
    self->isComplete = false;
    self->blobAllocator = blobAllocator;
    size_t chunkCount = (octetCount + self->fixedChunkSize - 1) / self->fixedChunkSize;
    bitArrayInit(&self->bitArray, memory, chunkCount);

    CLOG_C_VERBOSE(&self->log, "initialize. Expecting %zu octets", self->octetCount)
}

/// Frees the blob memory
void blobStreamInDestroy(BlobStreamIn* self)
{
    IMPRINT_FREE(self->blobAllocator, self->blob);
    self->blob = 0;
    bitArrayDestroy(&self->bitArray);
}

/// Checks if the blob stream is complete
/// @param self incoming blob stream
/// @return true if blob stream is completely received.
bool blobStreamInIsComplete(const BlobStreamIn* self)
{
    return self->isComplete;
}

/// Sets a received chunk (part) to the blob memory
/// @param self incoming blob stream
/// @param chunkId the zero based index of the chunk
/// @param octets the blob octets of the chunk
/// @param octetCount the number of octets in the chunk. Must be the
/// fixedChunkSize, apart from maybe the last chunk.
void blobStreamInSetChunk(BlobStreamIn* self, BlobStreamChunkId chunkId, const uint8_t* octets, size_t octetCount)
{
    size_t offset = chunkId * self->fixedChunkSize;
    if (offset + octetCount > self->octetCount) {
        CLOG_C_ERROR(&self->log, "blobStreamInSetChunk overwrite")
    }

    uint8_t* target = self->blob + offset;

    CLOG_C_VERBOSE(&self->log, "setChunk chunkId: %hu octetCount: %zu", chunkId, octetCount)

    if (chunkId == self->bitArray.bitCount - 1) {
        size_t expectedLastChunkSize = (self->octetCount % self->fixedChunkSize);
        if (expectedLastChunkSize == 0) {
            expectedLastChunkSize = self->fixedChunkSize;
        }
        if (octetCount != expectedLastChunkSize) {
            CLOG_C_ERROR(&self->log, "last chunk size must exactly. %zu vs %zu", octetCount, expectedLastChunkSize)
        }
    } else {
        if (octetCount != self->fixedChunkSize) {
            CLOG_C_ERROR(&self->log, "chunk size must be equal to fixed chunk size. %zu vs %zu", octetCount,
                         self->fixedChunkSize)
        }
    }

    bitArraySet(&self->bitArray, chunkId);

    tc_memcpy_octets(target, octets, octetCount);

    if (bitArrayAreAllSet(&self->bitArray)) {
        CLOG_C_VERBOSE(&self->log, "stream is complete")
        self->isComplete = true;
    }
}

/// returns a debug string of the state of the blob stream. Not implemented.
/// @param self incoming blob stream
/// @param buf target char buffer
/// @param maxBuf maximum number of characters in buf
/// @return buf pointer
const char* blobStreamInToString(const BlobStreamIn* self, char* buf, size_t maxBuf)
{
    (void) self;
    (void) maxBuf;

    buf[0] = 0;
    return buf;
}
