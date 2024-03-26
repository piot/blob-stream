/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include "utest.h"
#include <blob-stream/blob_stream_in.h>
#include <blob-stream/blob_stream_out.h>
#include <imprint/linear_allocator.h>
#include <imprint/slab_allocator.h>

#define TESTA_MEMORY_SIZE (40 * 2231)
static uint8_t memory[TESTA_MEMORY_SIZE];

typedef struct Mem {
    ImprintLinearAllocator linearAllocator;
    ImprintSlabAllocator slabAllocator;
} Mem;

static void createMemory(Mem* info)
{
    imprintLinearAllocatorInit(&info->linearAllocator, memory, TESTA_MEMORY_SIZE, "all memory");
    imprintSlabAllocatorInit(&info->slabAllocator, &info->linearAllocator.info, 12, 1, 5, "slabAllocator");
}

UTEST(BlobStreamIn, verifyChunkComplete)
{
    Mem memory;
    createMemory(&memory);

    BlobStreamIn inStream;

    Clog log;
    log.config = &g_clog;
    log.constantPrefix = "test";
#define TESTA_BLOB_SIZE (2231)

    blobStreamInInit(&inStream, &memory.linearAllocator.info, &memory.slabAllocator.info, TESTA_BLOB_SIZE, BLOB_STREAM_CHUNK_SIZE,
                     log);

    static uint8_t chunk[BLOB_STREAM_CHUNK_SIZE];

    ASSERT_FALSE(blobStreamInIsComplete(&inStream));
    blobStreamInSetChunk(&inStream, 2, chunk, 2231 % (1024));
    ASSERT_FALSE(blobStreamInIsComplete(&inStream));
    blobStreamInSetChunk(&inStream, 0, chunk, BLOB_STREAM_CHUNK_SIZE);
    ASSERT_FALSE(blobStreamInIsComplete(&inStream));
    blobStreamInSetChunk(&inStream, 1, chunk, BLOB_STREAM_CHUNK_SIZE);
    ASSERT_TRUE(blobStreamInIsComplete(&inStream));
}

UTEST(BlobStreamOut, verifyChunkComplete)
{
    Mem memory;
    createMemory(&memory);

    BlobStreamOut outStream;

#define TESTB_BLOB_SIZE (2246)
    static uint8_t blob[TESTB_BLOB_SIZE];

    Clog log;
    log.config = &g_clog;
    log.constantPrefix = "test";

    blobStreamOutInit(&outStream, &memory.linearAllocator.info, &memory.slabAllocator.info, blob, TESTB_BLOB_SIZE,
                      BLOB_STREAM_CHUNK_SIZE, log);
    ASSERT_FALSE(blobStreamOutIsComplete(&outStream));
    blobStreamOutMarkReceived(&outStream, 0, 0x00000000);
    ASSERT_FALSE(blobStreamOutIsComplete(&outStream));
    blobStreamOutMarkReceived(&outStream, 3, 0x00000000);
    ASSERT_TRUE(blobStreamOutIsComplete(&outStream));

    blobStreamOutDestroy(&outStream);
}


UTEST(BlobStreamOut, verifySentAll)
{
    Mem memory;
    createMemory(&memory);

    BlobStreamOut outStream;

#define TESTB_BLOB_SIZE (2246)
    static uint8_t blob[TESTB_BLOB_SIZE];

    Clog log;
    log.config = &g_clog;
    log.constantPrefix = "test";

    blobStreamOutInit(&outStream, &memory.linearAllocator.info, &memory.slabAllocator.info, blob, TESTB_BLOB_SIZE,
                      BLOB_STREAM_CHUNK_SIZE, log);
    ASSERT_FALSE(blobStreamOutIsComplete(&outStream));
    ASSERT_FALSE(blobStreamOutIsAllSent(&outStream));
    blobStreamOutMarkReceived(&outStream, 0, 0x00000000);
    ASSERT_FALSE(blobStreamOutIsComplete(&outStream));
    ASSERT_FALSE(blobStreamOutIsAllSent(&outStream));
    ASSERT_FALSE(blobStreamOutIsAllSent(&outStream));

    BlobStreamOutEntry* entries[4];
    MonotonicTimeMs now = 99;
    blobStreamOutGetChunksToSend(&outStream, now, &entries, 2);
    ASSERT_FALSE(blobStreamOutIsAllSent(&outStream));

    blobStreamOutGetChunksToSend(&outStream, now, &entries, 1);
    ASSERT_TRUE(blobStreamOutIsAllSent(&outStream));

    blobStreamOutMarkReceived(&outStream, 3, 0x00000000);
    ASSERT_TRUE(blobStreamOutIsComplete(&outStream));

    blobStreamOutDestroy(&outStream);
}
