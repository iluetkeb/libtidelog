/* 
 * File:   tidestruct.hpp
 * Author: ingo
 *
 * Created on 22. April 2011, 15:15
 */

#ifndef TIDESTRUCT_HPP
#define	TIDESTRUCT_HPP

#include <sys/time.h>

#define HDR_SIZE 12
#define TIDE_SIZE 10
#define TIMESTAMP_SIZE 8
#define CHUNK_SIZE 25

#define SMALL_ARRAY_SIZE_SIZE 1
#define ARRAY_SIZE_SIZE 4
#define TIMESTAMP_SIZE 8

namespace tide {
    namespace log {
        namespace {
            inline uint64_t timestamp(const timeval& tv) {
                return ((uint64_t) tv.tv_sec * 10e6) + (uint64_t) tv.tv_usec;
            }
        }

        struct HEADER {
            unsigned char tag[4]; /* Indicates the type of block. Typically a 4-byte character string. */
            uint64_t block_size; /* Size of the block in bytes, excluding the tag and size values. */
        } __attribute__((__packed__));

        struct TIDE {
            uint8_t major_version; /* Major version of the TIDE format used. */
            uint8_t minor_version; /* Minor version of the TIDE format used. */
            uint32_t num_channels; /* Number of channels of data in the file. */
            uint32_t num_chunks; /* Number of chunk blocks that are in the file. */
            
            TIDE(uint8_t major, uint8_t minor, uint32_t num_channels, uint32_t num_chunks) : 
                major_version(major), minor_version(minor), num_channels(num_channels), num_chunks(num_chunks) {};
        } __attribute__((__packed__));

        struct CHUNK {
            uint32_t id; /* Chunk identification, used to link entries to chunks.*/
            uint32_t count; /* Number of entries in this chunk. */
            uint64_t start; /* Time stamp of the first entry in this chunk. */
            uint64_t end; /* Time stamp of the last entry in this chunk. */
            uint8_t compression; /* Indicates the compression used on the entries. */

            CHUNK(const uint32_t id, const uint32_t count, const timeval& start,
                const timeval& end, uint8_t compression) : id(id), count(count),
                start(timestamp(start)), end(timestamp(end)), compression(compression) {
                
            }
        } __attribute__((__packed__));

        struct Array {
            const uint32_t size;
            const void* data;

            Array(const uint32_t _size, const void* _data) : size(_size), data(_data) {
            };
        };

        struct SArray {
            const uint8_t size;
            const void* data;

            SArray(const uint8_t _size, const void* _data) : size(_size), data(_data) {
            };
        };
    }
}

#endif	/* TIDESTRUCT_HPP */
