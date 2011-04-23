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
            inline uint64_t timeval2tstamp(const timeval& tv) {
                return ((uint64_t) tv.tv_sec * 10e6) + (uint64_t) tv.tv_usec;
            }
        }

        struct HEADER {
            const char tag[4]; /* Indicates the type of block. Typically a 4-byte character string. */
            const uint64_t block_size; /* Size of the block in bytes, excluding the tag and size values. */
            
            HEADER(const char stag[4], uint64_t block_size) : tag({stag[0], stag[1], stag[2], stag[3]}), block_size(block_size) {
            };
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
                start(timeval2tstamp(start)), end(timeval2tstamp(end)), compression(compression) {
                
            }
            CHUNK(const uint32_t id, const uint32_t count, const uint64_t start,
                const uint64_t end, uint8_t compression) : id(id), count(count),
                start(start), end(end), compression(compression) {
                
            }
        } __attribute__((__packed__));
    }
}

#endif	/* TIDESTRUCT_HPP */
