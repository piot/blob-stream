# Blob Stream

Serializing a blob (octet payload) reliably. Usually used for transmitting over an unreliable datagram transport.

Recommended to be wrapped by other custom commands that includes the a blob stream channel id, so it can support multiple simultaneous channels.

## Commands

### Send Chunk

Serialized from payload holder to receiver.

| type                |         octets | name                                                                                                                 |
| :------------------ | -------------: | :------------------------------------------------------------------------------------------------------------------- |
| uint8               |              1 | BLOB_STREAM_LOGIC_CMD_SET_CHUNK (1)                                                                                  |
| [ChunkId](#chunkid) |              4 | The chunkId for the following data.                                                                                  |
| uint16              |              2 | **octetCount** in this packet. Same as the Fixed Chunk Size (default 1024) for all chunks, except for the last one. |
| Payload             | **octetCount** | payload window content                                                                                               |

### Ack Set Chunk

Sent from the receiving end.

| type                | octets | name                                                                                                 |
| :------------------ | -----: | :--------------------------------------------------------------------------------------------------- |
| uint8               |      1 | BLOB_STREAM_LOGIC_CMD_ACK_CHUNK (2)                                                                  |
| [ChunkId](#chunkid) |      4 | **waitingForChunkId**                                                                                |
| uint32              |      4 | **receiveMask**. Bit is 1 for each packet received and 0 for windows from **waitingForChunkId** + 1. |

#### Example

```console
waitingForChunkId = 32
receiveMask = 0000 1000 0000 0000 0000 0000 0000 0101
```

The following chunks have been received: 0 to 31 inclusively (because remote is waiting for 32). Chunks 33, 35 and 60 has also been received.

## Types

### ChunkId

`uint32`, 4 octets. ChunkId is the zero based index of Fixed Chunk Size (usually 1024 octets) in the blob.
