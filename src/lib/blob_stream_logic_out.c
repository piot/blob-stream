/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/

#include <blob-stream/blob_stream_logic_out.h>
#include <blob-stream/commands.h>
#include <blob-stream/debug.h>
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <inttypes.h>

/// Initializes the logic for sending a blob stream.
/// @param self outgoing stream logic
/// @param blobStream the blobStream to send.
void blobStreamLogicOutInit(BlobStreamLogicOut* self, BlobStreamOut* blobStream, BlobStreamTransferId transferId)
{
    CLOG_VERBOSE("blobStreamLogicOutInit")
    self->blobStream = blobStream;
    self->transferId = transferId;
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
    CLOG_VERBOSE("BlobStreamLogicOut SendCmd: %02X %s", cmd, blobStreamCmdToString(cmd))
    fldOutStreamWriteUInt8(outStream, cmd);
}

const static size_t BlobStreamLogicMaxEntryOctetSize = 1080;

int blobStreamLogicOutStartTransfer(BlobStreamLogicOut* self, FldOutStream* tempStream)
{
    sendCommand(tempStream, BLOB_STREAM_LOGIC_CMD_START_TRANSFER);
    fldOutStreamWriteUInt16(tempStream, self->transferId);
    fldOutStreamWriteUInt32(tempStream, (uint32_t) self->blobStream->octetCount);
    return fldOutStreamWriteUInt16(tempStream, (uint16_t) self->blobStream->fixedChunkSize);
}

/// Serialize the specified entry to the target outStream.
/// @param tempStream the target stream
/// @param entry specifies which chunk (part) of the blob stream to serialize
/// @return if error occurred it returns a negative error code.
int blobStreamLogicOutSendEntry(FldOutStream* tempStream, const BlobStreamOutEntry* entry,
                                BlobStreamTransferId transferId)
{
    if (entry->octetCount > BlobStreamLogicMaxEntryOctetSize) {
    }

    if (tempStream->pos + entry->octetCount + sizeof(uint32_t) + sizeof(uint16_t) > tempStream->size) {
        CLOG_ERROR("stream is too small, needed room for a complete blob stream part (%zu), but has:%zu",
                   BlobStreamLogicMaxEntryOctetSize, tempStream->size - tempStream->pos)
        // return -2;
    }

    sendCommand(tempStream, BLOB_STREAM_LOGIC_CMD_SET_CHUNK);
    fldOutStreamWriteUInt16(tempStream, transferId);
    fldOutStreamWriteUInt32(tempStream, entry->chunkId);
    fldOutStreamWriteUInt16(tempStream, (uint16_t) entry->octetCount);
    return fldOutStreamWriteOctets(tempStream, entry->octets, entry->octetCount);
}

/// Checks if the blob stream is fully received by the receiver.
/// @param self outgoing stream logic
/// @return true if fully received
bool blobStreamLogicOutIsComplete(BlobStreamLogicOut* self)
{
    return blobStreamOutIsComplete(self->blobStream);
}

/// Checks if the blob stream is fully sent to the receiver.
/// @param self outgoing stream logic
/// @return true if all chunks are sent.
bool blobStreamLogicOutIsAllSent(BlobStreamLogicOut* self)
{
    return blobStreamOutIsAllSent(self->blobStream);
}

static int ackStart(BlobStreamLogicOut* self, FldInStream* inStream)
{
    BlobStreamTransferId transferId;

    int transferErr = fldInStreamReadUInt16(inStream, &transferId);
    if (transferErr < 0) {
        return transferErr;
    }

    if (transferId != self->transferId) {
        CLOG_SOFT_ERROR("ack start for wrong transferId %04X vs %04X", transferId, self->transferId)
        return -1;
    }

    return 0;
}

static int ackChunk(BlobStreamLogicOut* self, FldInStream* inStream)
{
    BlobStreamTransferId transferId;

    int transferErr = fldInStreamReadUInt16(inStream, &transferId);
    if (transferErr < 0) {
        return transferErr;
    }

    uint32_t waitingForChunkId;
    int readErr = fldInStreamReadUInt32(inStream, &waitingForChunkId);
    if (readErr < 0) {
        return readErr;
    }

    uint64_t receiveMask;
    int readLengthErr = fldInStreamReadUInt64(inStream, &receiveMask);
    if (readLengthErr < 0) {
        return readLengthErr;
    }

    if (transferId != self->transferId) {
        CLOG_SOFT_ERROR("ack chunk for wrong transferId %04X vs %04X", transferId, self->transferId)
        return -1;
    }

    CLOG_VERBOSE("ack chunk: %u mask:%" PRIx64, waitingForChunkId, receiveMask)

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
        case BLOB_STREAM_LOGIC_CMD_ACK_START_TRANSFER:
            return ackStart(self, inStream);
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
