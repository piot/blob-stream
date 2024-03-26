/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <blob-stream/blob_stream_logic_in.h>
#include <blob-stream/commands.h>
#include <clog/clog.h>
#include <flood/in_stream.h>

/// Initializes the receive logic for a blobstream
/// @param self incoming blob stream logic
/// @param blobStream the blob stream to set chunks to.
void blobStreamLogicInInit(BlobStreamLogicIn* self, BlobStreamIn* blobStream)
{
    CLOG_VERBOSE("blobStreamLogicInInit: with blobstream of octetCount %zu", blobStream->octetCount)
    self->blobStream = blobStream;
}

static int setChunk(BlobStreamLogicIn* self, FldInStream* inStream)
{
    uint32_t chunkId;
    int readErr = fldInStreamReadUInt32(inStream, &chunkId);
    if (readErr < 0) {
        return readErr;
    }

    uint16_t octetLength;
    int readLengthErr = fldInStreamReadUInt16(inStream, &octetLength);
    if (readLengthErr < 0) {
        return readLengthErr;
    }

    if (octetLength > self->blobStream->fixedChunkSize) {
        CLOG_ERROR("octetLength overrun %hu", octetLength)
    }

    blobStreamInSetChunk(self->blobStream, (BlobStreamChunkId) chunkId, inStream->p, octetLength);
    inStream->p += octetLength;
    inStream->pos += octetLength;

    return 0;
}

/// Receive a incoming blob stream command
/// Only BLOB_STREAM_LOGIC_CMD_SET_CHUNK is supported.
/// @param self incoming blob stream logic
/// @param inStream stream to receive from
/// @return negative on error
int blobStreamLogicInReceive(BlobStreamLogicIn* self, FldInStream* inStream)
{
    uint8_t cmd;
    int cmdResult = fldInStreamReadUInt8(inStream, &cmd);
    if (cmdResult < 0) {
        return cmdResult;
    }

    switch (cmd) {
        case BLOB_STREAM_LOGIC_CMD_SET_CHUNK:
            return setChunk(self, inStream);
        default:
            CLOG_ERROR("blobStreamLogicInReceive: Unknown command %02X", cmd)
            // return -2;
    }
}

static void sendCommand(FldOutStream* outStream, uint8_t cmd)
{
    CLOG_VERBOSE("BlobStreamLogicIn: SendCmd: %02X", cmd)

    fldOutStreamWriteUInt8(outStream, cmd);
}

/// Writes the receive status to the outstream
/// @param self incoming blob stream logic
/// @param outStream stream where the BLOB_STREAM_LOGIC_CMD_ACK_CHUNK will be written to
/// @return the result code. if less than zero it indicates and error.
int blobStreamLogicInSend(BlobStreamLogicIn* self, FldOutStream* outStream)
{
    size_t waitingForChunkId = bitArrayFirstUnset(&self->blobStream->bitArray);
    BitArrayAtom receiveMask = bitArrayGetAtomFrom(&self->blobStream->bitArray, waitingForChunkId + 1);

    CLOG_VERBOSE("blobStreamLogicIn: send. We are waiting for %04zX, mask %08X", waitingForChunkId, receiveMask)
    sendCommand(outStream, BLOB_STREAM_LOGIC_CMD_ACK_CHUNK);
    fldOutStreamWriteUInt32(outStream, (uint32_t) waitingForChunkId);

    return fldOutStreamWriteUInt32(outStream, receiveMask);
}

/// Clears the logic
/// Similar to blobStreamLogicInInit(), but it reuses the same target blobStream.
/// @param self incoming blob stream logic
void blobStreamLogicInClear(BlobStreamLogicIn* self)
{
    (void) self;
    // blobStreamInClear(&self->blobStream);
}

/// Frees up the memory for the logic
/// @param self incoming blob stream logic
void blobStreamLogicInDestroy(BlobStreamLogicIn* self)
{
    (void) self;
}

/// creates debug string from internal state
/// @param self incoming blob stream logic
/// @param buf target char buffer
/// @param maxBuf maximum char count for buf
/// @return returns buf
const char* blobStreamLogicInToString(const BlobStreamLogicIn* self, char* buf, size_t maxBuf)
{
    (void) self;
    (void) maxBuf;

    buf[0] = 0;
    return buf;
}
