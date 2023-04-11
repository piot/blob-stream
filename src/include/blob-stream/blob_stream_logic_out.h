/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef BLOB_STREAM_LOGIC_OUT_H
#define BLOB_STREAM_LOGIC_OUT_H

#include <stdint.h>
#include <stdlib.h>

#include <blob-stream/blob_stream_out.h>

struct FldInStream;
struct FldOutStream;

typedef struct BlobStreamLogicOut {
    BlobStreamOut* blobStream;
} BlobStreamLogicOut;


void blobStreamLogicOutInit(BlobStreamLogicOut* self, BlobStreamOut* blobStream);
int blobStreamLogicOutPrepareSend(BlobStreamLogicOut* self, MonotonicTimeMs now, const BlobStreamOutEntry * entries[], size_t maxEntriesCount);
int blobStreamLogicOutSendEntry(struct FldOutStream *tempStream, const BlobStreamOutEntry* entry);
int blobStreamLogicOutReceive(BlobStreamLogicOut* self, struct FldInStream* inStream);
void blobStreamLogicOutDestroy(BlobStreamLogicOut* self);
const char* blobStreamLogicOutToString(const BlobStreamLogicOut* self, char* buf, size_t maxBuf);
int blobStreamLogicOutIsComplete(BlobStreamLogicOut* self);

#endif
