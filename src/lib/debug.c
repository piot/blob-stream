/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/blob-stream
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/

#include <blob-stream/debug.h>
#include <clog/clog.h>

/// Converts a BlobStream command to a string
/// Dependant on the `commands.h` header file
/// @param cmd command to log
/// @return return string representation of cmd
const char* blobStreamCmdToString(uint8_t cmd)
{
    static const char* lookup[] = {
        "_",
        "SetChunk",
        "StartTransfer",
        "AckStartTransfer",
        "AckChunk",
    };

    if (cmd >= sizeof(lookup) / sizeof(lookup[0])) {
        CLOG_ERROR("Unknown blob stream cmd: %02X", cmd)
        // return 0;
    }

    return lookup[cmd];
}
