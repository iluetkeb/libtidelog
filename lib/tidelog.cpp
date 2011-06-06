#include "tidelog.hpp"

#include <sstream>
#include <errno.h>
#include <cstring>
#include <limits.h>
#include <iostream>
#include <cassert>
#include <sys/uio.h>

#include "tidestruct.hpp"
#include "chunk.hpp"

namespace tide {
    namespace log {
        namespace {
            inline void check_io(const int expected, const int actual, const char* name) {
                if (expected != actual) {
                    const int err(errno);
                    std::ostringstream msg;                    
                    msg << "Should have written " << expected << " item(s), "
                            "for " << name << " but wrote only " << actual;
                    if(errno != 0)
                        msg << ": " << strerror(err);
                    else
                        msg << ".";
                    throw IOException(msg.str());
                }
            }

            inline void check_bounds(const std::string& name, uint32_t max, uint32_t actual) {
                if (actual > max) {
                    std::ostringstream msg;
                    msg << "Size for " << name << "(" << actual << ") larger than max (" << max << ")";
                    throw IllegalArgumentException(msg.str());
                }
            }

            const char TAG_TIDE[] = {'T', 'I', 'D', 'E'};
            const char TAG_CHAN[] = {'C', 'H', 'A', 'N'};
            const char TAG_CHUNK[] = {'C', 'H', 'N', 'K'};
            const unsigned char TIDE_VERSION[] = {1, 0};
        }

        TIDELog::TIDELog(const std::string& logfile_name) : num_chunks(0), current_chunk(NULL) {
            logfile = fopen(logfile_name.c_str(), "wb");
            if (logfile == NULL) {
                throw IOException(strerror(errno));
            }
            writeTIDE();
        }
        TIDELog::TIDELog(FILE* stream) : logfile(stream), num_chunks(0), current_chunk(NULL) {
            if (logfile == NULL) {
                throw IllegalArgumentException("Stream must not be null");
            }
            writeTIDE();
        }

        TIDELog::~TIDELog() {
            finish_chunk();
            writeTIDE(); // update header
            fclose(logfile);
        }

        template<typename T, unsigned int SIZE>
        inline void TIDELog::write_checked(const T& data, const char* name) {
            assert(sizeof(T) == SIZE);
            check_io(1, fwrite(&data, sizeof (T), 1, logfile), name);
        }

        template<>
        inline void TIDELog::write_checked<SArray>(const SArray& array, const char* name) {
            check_io(1, fwrite(&(array.length), sizeof (array.length), 1, logfile), "size field");
            check_io(1, fwrite(array.bytes, array.length, 1, logfile), name);
        }
        template<>
        inline void TIDELog::write_checked<Array>(const Array& array, const char* name) {
            check_io(1, fwrite(&(array.length), sizeof (array.length), 1, logfile), "size field");
            check_io(1, fwrite(array.bytes, array.length, 1, logfile), name);
        }

        template<>
        void TIDELog::write_checked<timeval>(const timeval& tv, const char* name) {
            uint64_t timestamp = ((uint64_t) tv.tv_sec * 10e6) + (uint64_t) tv.tv_usec;
            assert(sizeof(uint64_t) == 8);
            check_io(1, fwrite(&timestamp, sizeof (timestamp), 1, logfile), "timeval");
        }

        void TIDELog::writeTIDE() {
            int pos = ftell(logfile);
            fseek(logfile, 0, SEEK_SET);
            HEADER hdr(TAG_TIDE, pos);

            write_checked<HEADER,HDR_SIZE>(hdr, "TIDE header");
            write_checked<TIDE,TIDE_SIZE>(TIDE(1, 0, channel_sizes.size(), num_chunks), "TIDE block");

            check_io(0, fflush(logfile), "flush");
        }

        void TIDELog::start_chunk() {
            finish_chunk();
            current_chunk = new Chunk(++num_chunks, ftell(logfile));
            writeCHUNK();
        }

        void TIDELog::finish_chunk() {
            if (current_chunk == NULL)
                return;

            const off_t curpos = ftell(logfile);
            fseek(logfile, -current_chunk->get_size(), SEEK_CUR);
            writeCHUNK();
            fseek(logfile, curpos, SEEK_SET);

            delete current_chunk;
            current_chunk = NULL;
        }

        void TIDELog::writeCHUNK() {
            write_checked<HEADER,HDR_SIZE>(HEADER(TAG_CHUNK, current_chunk->get_size()));
            write_checked<CHUNK,CHUNK_SIZE>(current_chunk->get_header());
            check_io(0, fflush(logfile), "flush");
        }

        Channel TIDELog::writeCHAN(const std::string& name, const std::string& type, const std::string& source,
                const SArray& source_spec, const Array& fmt_spec, uint32_t data_size) {
            // argument checking
            check_bounds("name", 256, name.length());
            check_bounds("type", 10, type.length());
            check_bounds("source", 256, source.length());
            check_bounds("source_spec", 256, source_spec.length);
            check_bounds("fmt_spec", UINT_MAX, fmt_spec.length);

            // header
            HEADER hdr(TAG_CHAN, 4 + 1 + name.length() + 10 + 1 + 
                        source.length() + 1 + source_spec.length + 4 + fmt_spec.length + 4);

            unsigned char typebuf[10];
            memset(typebuf, 0, 10);
            write_checked<HEADER,HDR_SIZE>(hdr);

            // ID
            const uint32_t id = channel_sizes.size() + 1;
            check_io(1, fwrite(&id, sizeof(id), 1, logfile), "id");
            // name
            write_checked(SArray(name.c_str(), name.length()), "name");
            // type
            memcpy(typebuf, type.c_str(), type.length());
            check_io(1, fwrite(typebuf, 10, 1, logfile), "type");
            // human-readable source description
            write_checked(SArray(source.c_str(), source.length()), "source");
            // source string
            write_checked(source_spec, "spec");
            // format spec
            write_checked(fmt_spec, "format");
            // data size
            check_io(1, fwrite(&data_size, 4, 1, logfile), "flush");

            fflush(logfile);

            Channel c(id, data_size);
            channel_sizes[c.id] = data_size;
            return c;
        }
        
        void TIDELog::write(const Channel& c, const timeval& tstamp, const Array& data) {
            if(current_chunk == NULL) {
                start_chunk();
            }
            uint64_t time = timeval2tstamp(tstamp);
            current_chunk->update(ChunkEntry(time, data));
            
            ENTRY entry = { c.id, time };
            write_checked<ENTRY,sizeof(ENTRY)>(entry, "entry header");
            write_checked(data);
            
        }
    }
}
