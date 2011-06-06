/* 
 * File:   chunk.hpp
 * Author: ingo
 *
 * Created on 22. April 2011, 23:46
 */

#ifndef CHUNK_HPP
#define	CHUNK_HPP

#include "tidestruct.hpp"
#include <iosfwd>

namespace tide {
    namespace log {
        class ChunkEntry {
        private:
            const uint64_t timestamp;
            const uint64_t length;
            
        public:
            ChunkEntry(const uint64_t timestamp, const Array& data);
            
            uint64_t get_timestamp() const;
            uint64_t get_length() const;
        };
        
        class Chunk {
        private:
            const int id;
            const off_t start_filepos;
            uint64_t start_timestamp, end_timestamp;
            uint64_t chunk_length;
            unsigned int num_entries;
            
        public:
            Chunk(const int id, const off_t start);
            void update(const ChunkEntry& entry);
            
            uint64_t get_size();
            CHUNK get_header() const;
        };
    
        std::ostream& operator<<(std::ostream& out, const Chunk& c);

    }    
}

#endif	/* CHUNK_HPP */

