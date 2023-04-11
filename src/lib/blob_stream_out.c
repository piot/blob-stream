#include <blob-stream/blob_stream_out.h>
#include <clog/clog.h>
#include <imprint/allocator.h>
#include <stdbool.h>

/// Initializes a blobStream for sending
/// @param self
/// @param allocator
/// @param blobAllocator
/// @param data
/// @param octetCount
/// @param fixedChunkSize
void blobStreamOutInit(BlobStreamOut *self, ImprintAllocator* allocator, ImprintAllocatorWithFree* blobAllocator, const uint8_t *data, size_t octetCount, size_t fixedChunkSize)
{
    self->blob = data;
    self->octetCount = octetCount;
    self->fixedChunkSize = fixedChunkSize;
    self->isComplete = 0;
    self->chunkCount = (octetCount + self->fixedChunkSize - 1) / self->fixedChunkSize;
    self->entries = IMPRINT_ALLOC_TYPE_COUNT(allocator, BlobStreamOutEntry, self->chunkCount);
    self->blobAllocator = blobAllocator;

    for (size_t i = 0; i < self->chunkCount; ++i)
    {
        BlobStreamOutEntry *entry = &self->entries[i];
        entry->octets = data + i * self->fixedChunkSize;
        if (i == self->chunkCount - 1)
        {
            size_t expectedLastChunkSize = (self->octetCount % self->fixedChunkSize);
            if (expectedLastChunkSize == 0)
            {
                expectedLastChunkSize = self->fixedChunkSize;
            }
            entry->octetCount = expectedLastChunkSize;
        }
        else
        {
            entry->octetCount = self->fixedChunkSize;
        }
        entry->chunkId = i;
        entry->lastSentAtTime = 0;
        entry->sendCount = 0;
        entry->isReceived = 0;
        entry->isReceived = false;
    }
    CLOG_VERBOSE("blobStreamOutInit octetCount: %zu chunkCount: %zu fixedChunkSize %zu", octetCount, self->chunkCount, self->fixedChunkSize)
}

/// Frees up the memory of the outgoing blob stream
/// @param self
void blobStreamOutDestroy(BlobStreamOut *self)
{
 // IMPRINT_FREE(self->blobAllocator, self->entries);
    self->blob = 0;
}

/// Checks if the blobStream is fully received by the receiver.
/// @param self
/// @return true if received
int blobStreamOutIsComplete(const BlobStreamOut *self)
{
    return self->isComplete;
}

/// Marks chunks as received.
/// @param self
/// @param everythingBeforeThis all chunks before this index should be marksed as received.
/// @param maskReceived the bits that are set should also be marked as received. They indicate the chunks after everythingBeforeThis.
void blobStreamOutMarkReceived(BlobStreamOut *self, BlobStreamChunkId everythingBeforeThis, BitArrayAtom maskReceived)
{
    if (everythingBeforeThis > self->chunkCount) {
        CLOG_ERROR("strange everythingBeforeThis")
    }

    CLOG_VERBOSE("blobStreamOut: markReceived remote expecting %04X mask %08X", everythingBeforeThis, maskReceived)

    if (self->isComplete) {
      return;
    }

    //CLOG_OUTPUT_STDERR("blobStreamOut: remote has received everything before %04X", everythingBeforeThis)
    for (size_t i = 0; i < everythingBeforeThis; ++i)
    {
        BlobStreamOutEntry *entry = &self->entries[i];
        entry->isReceived = 1;
    }
    if (everythingBeforeThis == self->chunkCount) {
        self->isComplete = 1;
        CLOG_VERBOSE("blobStreamOut: remote has received everything")
        return;
    }

    BitArrayAtom accumulator = maskReceived;

    for (size_t i = 0; i < BIT_ARRAY_BITS_IN_ATOM; ++i)
    {
        size_t index = everythingBeforeThis + i + 1;
        if (index >= self->chunkCount) {
            return;
        }
        if (accumulator & 0x1)
        {
            BlobStreamOutEntry *entry = &self->entries[index];
            entry->isReceived = 1;
            CLOG_VERBOSE("blobStreamOut: remote has received chunkId %04zX", index)
        }
        accumulator = accumulator >> 1;
    }
}

/// Calculates which chunks that needs to be sent
/// @param self
/// @param now current time
/// @param resultEntries the resulting entries that needs to be sent/resent.
/// @param maxEntriesCount the maximum size of the resultEntries
/// @return the number of resultEntries filled, or if negative: the error code.
int blobStreamOutGetChunksToSend(BlobStreamOut *self, MonotonicTimeMs now, const BlobStreamOutEntry **resultEntries, size_t maxEntriesCount)
{
    if (maxEntriesCount == 0)
    {
        return 0;
    }

    if (maxEntriesCount > 3)
    {
        maxEntriesCount = 3;
    }

    static MonotonicTimeMs threshold = 500;
    size_t resultCount = 0;

    for (size_t i = 0; i < self->chunkCount; ++i)
    {
        BlobStreamOutEntry *entry = &self->entries[i];
        if (!entry->isReceived && ( ( (now - entry->lastSentAtTime > threshold) && entry->octetCount != 0) || entry->sendCount == 0))
        {
            resultEntries[resultCount] = entry;
            entry->lastSentAtTime = now;
            resultCount++;
            entry->sendCount++;

            CLOG_VERBOSE("blobStreamOut: send chunkId %04X", entry->chunkId)

            if (resultCount == maxEntriesCount)
            {
                return resultCount;
            }
        } else {
          //CLOG_OUTPUT_STDERR("blobStreamOut: not sending: %zu (%zu) %zu %d", entry->lastSentAtTime, now, entry->octetCount, entry->isReceived)
        }
    }

    return resultCount;
}

/// Returns a string describing the internal state of the outgoing blob stream. Not implemented!
/// @param self
/// @param buf
/// @param maxBuf
/// @return
const char* blobStreamOutToString(const BlobStreamOut *self, char *buf, size_t maxBuf)
{
    buf[0] = 0;
    return buf;
}
