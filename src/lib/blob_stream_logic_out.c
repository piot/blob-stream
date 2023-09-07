/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <blob-stream/blob_stream_logic_out.h>
#include <blob-stream/commands.h>
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <inttypes.h>

/// Initializes the logic for sending a blob stream.
/// @param self outgoing stream logic
/// @param blobStream the blobStream to send.
void blobStreamLogicOutInit(BlobStreamLogicOut* self, BlobStreamOut* blobStream)
{
    CLOG_VERBOSE("blobStreamLogicOutInit")
    self->blobStream = blobStream;
}

/// Calculates which chunks (parts) that needs to be resent.
/// @param self outgoing stream logic
/// @param now the current time. Is used to figure out if the resend-timer has been triggered.
/// @param entries the target entries
/// @param maxEntriesCount maximum number of entries to fill
/// @return returns the number of entries filled, or less than zero if an error occurred.
int blobStreamLogicOutPrepareSend(BlobStreamLogicOut* self, MonotonicTimeMs now, const BlobStreamOutEntry* entries[],
                                  size_t maxEntriesCount)
{
    return blobStreamOutGetChunksToSend(self->blobStream, now, entries, maxEntriesCount);
}

static void sendCommand(FldOutStream* outStream, uint8_t cmd)
{
    CLOG_VERBOSE("BlobStreamLogicOut SendCmd: %02X", cmd)
    fldOutStreamWriteUInt8(outStream, cmd);
}

/// Serialize the specified entry to the target outStream.
/// @param tempStream the target stream
/// @param entry specifies which chunk (part) of the blob stream to serialize
/// @return if error occurred it returns a negative error code.
int blobStreamLogicOutSendEntry(FldOutStream* tempStream, const BlobStreamOutEntry* entry)
{
    if (tempStream->pos + 1100 > tempStream->size) {
        CLOG_ERROR("stream is too small, needed room for a complete UDP payload (1100), but has:%zu",
                   tempStream->size - tempStream->pos)
        // return -2;
    }

    sendCommand(tempStream, BLOB_STREAM_LOGIC_CMD_SET_CHUNK);
    fldOutStreamWriteUInt32(tempStream, entry->chunkId);
    fldOutStreamWriteUInt16(tempStream, (uint16_t) entry->octetCount);
    return fldOutStreamWriteOctets(tempStream, entry->octets, entry->octetCount);
}

/// Checks if the blob stream is fully received by the receiver.
/// @param self outgoing stream logic
/// @return if error occurred it returns a negative error code.
bool blobStreamLogicOutIsComplete(BlobStreamLogicOut* self)
{
    return blobStreamOutIsComplete(self->blobStream);
}

static int ackChunk(BlobStreamLogicOut* self, FldInStream* inStream)
{
    uint32_t waitingForChunkId;
    int readErr = fldInStreamReadUInt32(inStream, &waitingForChunkId);
    if (readErr < 0) {
        return readErr;
    }

    uint32_t receiveMask;
    int readLengthErr = fldInStreamReadUInt32(inStream, &receiveMask);
    if (readLengthErr < 0) {
        return readLengthErr;
    }

    CLOG_VERBOSE("ack chunk: %d mask:%d", waitingForChunkId, receiveMask)

    blobStreamOutMarkReceived(self->blobStream, (BlobStreamChunkId) waitingForChunkId, receiveMask);

    return 0;
}

/// Receive a blob stream command
/// Only BLOB_STREAM_LOGIC_CMD_ACK_CHUNK is supported.
/// @param self outgoing stream logic
/// @param inStream the stream to read from
/// @return negative value if error was encountered.
int blobStreamLogicOutReceive(BlobStreamLogicOut* self, struct FldInStream* inStream)
{
    uint8_t cmd;
    int cmdResult = fldInStreamReadUInt8(inStream, &cmd);
    if (cmdResult < 0) {
        return cmdResult;
    }

    switch (cmd) {
        case BLOB_STREAM_LOGIC_CMD_ACK_CHUNK:
            return ackChunk(self, inStream);
        default:
            CLOG_ERROR("blobStreamLogicOutReceive: Unknown command %02X", cmd)
    }
}

/// Frees up the memory for the outgoing logic
/// @param self outgoing stream logic
void blobStreamLogicOutDestroy(BlobStreamLogicOut* self)
{
    (void) self;
}

/// Returns a string describing the internal state of the outgoing logic. Not Implemented!
/// @param self outgoing stream logic
/// @param buf target character buffer
/// @param maxBuf maximum number of characters for buffer
/// @return buf
const char* blobStreamLogicOutToString(const BlobStreamLogicOut* self, char* buf, size_t maxBuf)
{
    (void) self;
    (void) maxBuf;

    buf[0] = 0;
    return buf;
}
