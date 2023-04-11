/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <blob-stream/blob_stream_logic_in.h>
#include <flood/in_stream.h>
#include <clog/clog.h>
#include <blob-stream/commands.h>

/// Initializes the receive logic for a blobstream
/// @param self
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


    blobStreamInSetChunk(self->blobStream, chunkId, inStream->p, octetLength);
    inStream->p += octetLength;
    inStream->pos += octetLength;

    return 0;
}

/// Receive a incoming blob stream command
/// Only BLOB_STREAM_LOGIC_CMD_SET_CHUNK is supported.
/// @param self
/// @param inStream
/// @return
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
            return -2;
    }
}

static void sendCommand(FldOutStream* outStream, uint8_t cmd)
{
  CLOG_INFO("BlobStreamLogicIn: SendCmd: %02X", cmd);

  fldOutStreamWriteUInt8(outStream, cmd);
}

/// Writes the receive status to the outstream
/// @param self
/// @param outStream stream where the BLOB_STREAM_LOGIC_CMD_ACK_CHUNK will be written to
/// @return the result code. if less than zero it indicates and error.
int blobStreamLogicInSend(BlobStreamLogicIn* self, FldOutStream* outStream)
{
    size_t waitingForChunkId = bitArrayFirstUnset(&self->blobStream->bitArray);
    BitArrayAtom receiveMask = bitArrayGetAtomFrom(&self->blobStream->bitArray, waitingForChunkId + 1);

    CLOG_VERBOSE("blobStreamLogicIn: send. We are waiting for %04zX, mask %08X", waitingForChunkId, receiveMask)
    sendCommand(outStream, BLOB_STREAM_LOGIC_CMD_ACK_CHUNK);
    fldOutStreamWriteUInt32(outStream, waitingForChunkId);

    return fldOutStreamWriteUInt32(outStream, receiveMask);
}

/// Clears the logic
/// Similar to blobStreamLogicInInit(), but it reuses the same target blobStream.
/// @param self
void blobStreamLogicInClear(BlobStreamLogicIn* self)
{
  //blobStreamInClear(&self->blobStream);
}


/// Frees up the memory for the logic
/// @param self
void blobStreamLogicInDestroy(BlobStreamLogicIn* self)
{
}

const char* blobStreamLogicInToString(const BlobStreamLogicIn* self, char* buf, size_t maxBuf)
{
    buf[0]=0;
    return buf;
}
