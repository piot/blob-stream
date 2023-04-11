/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef BLOB_STREAM_LOGIC_IN_H
#define BLOB_STREAM_LOGIC_IN_H

#include <stdint.h>
#include <stdlib.h>

#include <blob-stream/blob_stream_in.h>
#include <flood/out_stream.h>

struct FldInStream;

typedef struct BlobStreamLogicIn {
    BlobStreamIn* blobStream;
} BlobStreamLogicIn;


void blobStreamLogicInInit(BlobStreamLogicIn* self, BlobStreamIn* blobStream);
int blobStreamLogicInReceive(BlobStreamLogicIn* self, struct FldInStream* inStream);
int blobStreamLogicInSend(BlobStreamLogicIn* self, FldOutStream* outStream);
void blobStreamLogicInDestroy(BlobStreamLogicIn* self);
void blobStreamLogicInClear(BlobStreamLogicIn* self);

const char* blobStreamLogicInToString(const BlobStreamLogicIn* self, char* buf, size_t maxBuf);

#endif
