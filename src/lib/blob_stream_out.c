/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/

#include <blob-stream/blob_stream_out.h>
#include <imprint/allocator.h>
#include <stdbool.h>
#include <inttypes.h>

/// Initializes a blobStream for sending
/// @param self outgoing blob stream
/// @param allocator allocator for internal book keeping entries
/// @param blobAllocator not really used
/// @param data the payload to send out
/// @param octetCount the number of octets in the data payload
/// @param fixedChunkSize the size of each chunk to send out (except the last one). Usually 1024.
void blobStreamOutInit(BlobStreamOut* self, ImprintAllocator* allocator, ImprintAllocatorWithFree* blobAllocator,
                       const uint8_t* data, size_t octetCount, size_t fixedChunkSize, Clog log)
{
    self->log = log;
    self->blob = data;
    self->octetCount = octetCount;
    self->fixedChunkSize = fixedChunkSize;
    CLOG_ASSERT(fixedChunkSize <= 1024, "only chunks up to 1024 is supported")
    self->isComplete = false;
    self->chunkCount = (octetCount + self->fixedChunkSize - 1) / self->fixedChunkSize;
    self->entries = IMPRINT_ALLOC_TYPE_COUNT(allocator, BlobStreamOutEntry, self->chunkCount);
    self->blobAllocator = blobAllocator;
    self->sentChunkEntryCount = 0;
    self->thresholdForRedundancy = 50;

    for (size_t i = 0; i < self->chunkCount; ++i) {
        BlobStreamOutEntry* entry = &self->entries[i];
        entry->octets = data + i * self->fixedChunkSize;
        if (i == self->chunkCount - 1) {
            size_t expectedLastChunkSize = (self->octetCount % self->fixedChunkSize);
            if (expectedLastChunkSize == 0) {
                expectedLastChunkSize = self->fixedChunkSize;
            }
            entry->octetCount = expectedLastChunkSize;
        } else {
            entry->octetCount = self->fixedChunkSize;
        }
        entry->chunkId = (BlobStreamChunkId) i;
        entry->lastSentAtTime = 0;
        entry->sendCount = 0;
        entry->isReceived = false;
    }
    CLOG_C_VERBOSE(&self->log, "blobStreamOutInit octetCount: %zu chunkCount: %zu fixedChunkSize %zu", octetCount,
                   self->chunkCount, self->fixedChunkSize)
}

/// Frees up the memory of the outgoing blob stream
/// @param self outgoing blob stream
void blobStreamOutDestroy(BlobStreamOut* self)
{
    // IMPRINT_FREE(self->blobAllocator, self->entries);
    self->blob = 0;
}

/// Checks if the blobStream is fully received by the receiver.
/// @param self outgoing blob stream
/// @return true if received
bool blobStreamOutIsComplete(const BlobStreamOut* self)
{
    return self->isComplete;
}

/// Checks if the blobStream is fully sent (but not neccessarily recevied by the receiver).
/// @param self outgoing blob stream
/// @return true if all sent
bool blobStreamOutIsAllSent(const BlobStreamOut* self)
{
    return self->sentChunkEntryCount == self->chunkCount;
}

/// Marks chunks as received.
/// @param self outgoing blob stream
/// @param everythingBeforeThis all chunks before this index should be marked
/// as received.
/// @param maskReceived the bits that are set should also be marked as received.
/// They indicate the chunks after everythingBeforeThis.
void blobStreamOutMarkReceived(BlobStreamOut* self, BlobStreamChunkId everythingBeforeThis, BitArrayAtom maskReceived)
{
    if (everythingBeforeThis > self->chunkCount) {
        CLOG_C_ERROR(&self->log, "strange everythingBeforeThis")
    }

    CLOG_C_VERBOSE(&self->log, "markReceived remote expecting %04X mask %" PRIx64,  everythingBeforeThis, maskReceived)

    if (self->isComplete) {
        return;
    }

    // CLOG_OUTPUT_STDERR("blobStreamOut: remote has received everything before
    // %04X", everythingBeforeThis)
    for (size_t i = 0; i < everythingBeforeThis; ++i) {
        BlobStreamOutEntry* entry = &self->entries[i];
        entry->isReceived = true;
    }
    if (everythingBeforeThis == self->chunkCount) {
        self->isComplete = true;
        CLOG_C_VERBOSE(&self->log, "remote has received everything")
        return;
    }

    BitArrayAtom accumulator = maskReceived;

    for (size_t i = 0; i < BIT_ARRAY_BITS_IN_ATOM; ++i) {
        size_t index = everythingBeforeThis + i + 1;
        if (index >= self->chunkCount) {
            return;
        }
        if (accumulator & 0x1) {
            BlobStreamOutEntry* entry = &self->entries[index];
            entry->isReceived = true;
            CLOG_C_VERBOSE(&self->log, "remote has received chunkId %04zX", index)
        }
        accumulator = accumulator >> 1;
    }
}

/// Calculates which chunks that needs to be sent
/// @param self outgoing blob stream
/// @param now current time
/// @param resultEntries the resulting entries that needs to be sent/resent.
/// @param maxEntriesCount the maximum size of the resultEntries
/// @return the number of resultEntries filled, or if negative: the error code.
int blobStreamOutGetChunksToSend(BlobStreamOut* self, MonotonicTimeMs now, const BlobStreamOutEntry** resultEntries,
                                 size_t maxEntriesCount)
{
    if (maxEntriesCount == 0) {
        return 0;
    }

    if (maxEntriesCount > 5) {
        maxEntriesCount = 5;
    }

    size_t resultCount = 0;

    for (size_t i = 0; i < self->chunkCount; ++i) {
        BlobStreamOutEntry* entry = &self->entries[i];
        if (!entry->isReceived &&
            (((now - entry->lastSentAtTime > self->thresholdForRedundancy) && entry->octetCount != 0) ||
             entry->sendCount == 0)) {
            resultEntries[resultCount] = entry;
            entry->lastSentAtTime = now;
            resultCount++;
            if (!entry->sendCount) {
                // First time we sent it
                self->sentChunkEntryCount++;
            }
            entry->sendCount++;

            CLOG_C_VERBOSE(&self->log, "send chunkIndex %04X", entry->chunkId)

            if (resultCount == maxEntriesCount) {
                return (int) resultCount;
            }
        } else {
            // CLOG_OUTPUT_STDERR("blobStreamOut: not sending: %zu (%zu) %zu %d",
            // entry->lastSentAtTime, now, entry->octetCount, entry->isReceived)
        }
    }

    return (int) resultCount;
}

/// Returns a string describing the internal state of the outgoing blob stream.
/// Not implemented!
/// @param self outgoing blob stream
/// @param buf target character buffer
/// @param maxBuf maximum number of characters to write to buf
/// @return buf
const char* blobStreamOutToString(const BlobStreamOut* self, char* buf, size_t maxBuf)
{
    (void) self;
    (void) maxBuf;

    buf[0] = 0;
    return buf;
}
