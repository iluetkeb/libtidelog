#include "tidelog.hpp"

#include <sstream>
#include <errno.h>
#include <cstring>
#include <limits.h>
#include <iostream>
#include <cassert>

#include "tidestruct.hpp"

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
            const char TAG_CHNK[] = {'C', 'H', 'N', 'K'};
            const unsigned char TIDE_VERSION[] = {1, 0};
        }

        BufferReference::BufferReference(const char* bytes, const uint64_t length, const uint64_t offset) :
        bytes(bytes), length(length), offset(offset) {
        }

        TIDELog::TIDELog(const std::string& logfile_name) : current_chunk(NULL) {
            logfile = fopen(logfile_name.c_str(), "wb");
            if (logfile == NULL) {
                throw IOException(strerror(errno));
            }
            writeTIDE();
        }
        TIDELog::TIDELog(FILE* stream) : logfile(stream), current_chunk(NULL) {
            if (logfile == NULL) {
                throw IllegalArgumentException("Stream must not be null");
            }
            writeTIDE();
        }

        TIDELog::~TIDELog() {
            writeTIDE(); // update header
            if (current_chunk != NULL)
                delete current_chunk;
            fclose(logfile);
        }

        template<typename T, unsigned int SIZE>
        inline void TIDELog::write_checked(const T& data, const char* name) {
            assert(sizeof(T) == SIZE);
            check_io(1, fwrite(&data, sizeof (T), 1, logfile), name);
        }

        template<>
        inline void TIDELog::write_checked<SArray>(const SArray& data, const char* name) {
            check_io(1, fwrite(&(data.size), sizeof (data.size), 1, logfile), "small array size field");
            check_io(1, fwrite(&data, data.size, 1, logfile), name);
        }

        template<>
        inline void TIDELog::write_checked<Array>(const Array& data, const char* name) {
            check_io(1, fwrite(&(data.size), sizeof (data.size), 1, logfile), "array size field");
            check_io(1, fwrite(&data, data.size, 1, logfile), name);
        }

        template<>
        void TIDELog::write_checked<timeval>(const timeval& tv, const char* name) {
            uint64_t timestamp = ((uint64_t) tv.tv_sec * 10e6) + (uint64_t) tv.tv_usec;
            check_io(TIMESTAMP_SIZE, fwrite(&timestamp, sizeof (timestamp), 1, logfile), "timeval");
        }

        void TIDELog::writeTIDE() {
            fseek(logfile, 0, SEEK_SET);
            HEADER hdr = {
                { 'T', 'I', 'D', 'E'}, -1
            };

            write_checked<HEADER,HDR_SIZE>(hdr, "TIDE header");
            write_checked<TIDE,TIDE_SIZE>(TIDE(1, 0, channel_sizes.size(), num_chunks), "TIDE block");

            check_io(0, fflush(logfile), "flush");
        }

        void TIDELog::start_chunk() {
            finish_chunk();
        }

        void TIDELog::finish_chunk() {
            if (current_chunk == NULL)
                return;

            off_t curpos = ftell(logfile);
            fseek(logfile, current_chunk->start_filepos, SEEK_SET);
            writeCHUNK();
            fseek(logfile, curpos, SEEK_SET);

            current_chunk = NULL;
        }

        void TIDELog::writeCHUNK() {
            uint64_t size = ftello(logfile) - current_chunk->start_filepos - CHUNK_SIZE;
            HEADER hdr = {
                { 'C', 'H', 'N', 'K'}, size
            };
            write_checked<HEADER,HDR_SIZE>(hdr);
            write_checked<CHUNK,CHUNK_SIZE>(CHUNK(current_chunk->id, current_chunk->num_entries, current_chunk->tv_start, current_chunk->tv_end, 0));
            check_io(1, fputc(0, logfile), "flush");
        }

        Channel TIDELog::writeCHAN(const std::string& name, const std::string& type, const std::string& fmt_description,
                const BufferReference& source_spec, const BufferReference& fmt_spec, uint32_t data_size) {
            // argument checking
            check_bounds("name", 256, name.length());
            check_bounds("type", 10, type.length());
            check_bounds("fmt_description", 256, fmt_description.length());
            check_bounds("source_spec", 256, source_spec.length);
            check_bounds("fmt_spec", UINT_MAX, fmt_spec.length);

            // header
            HEADER hdr = {
                { 'C', 'H', 'A', 'N'}, 4 + 1 + name.length() + 10 + 1 + fmt_description.length() + source_spec.length + fmt_spec.length + 4
            };
            write_checked<HEADER,HDR_SIZE>(hdr);

            // ID
            const uint32_t id = channel_sizes.size() + 1;
            check_io(4, fwrite(&id, 4, 1, logfile), "id");
            // name
            write_checked(SArray(name.length(), name.c_str()));
            // type
            write_checked(SArray(std::max((size_t) 10, type.length()), type.c_str()));
            // human-readable source description
            write_checked(SArray(fmt_description.length(), fmt_description.c_str()));
            // source string
            write_checked(SArray(source_spec.length, source_spec.bytes + source_spec.offset));
            // format spec
            write_checked(Array(fmt_spec.length, fmt_spec.bytes + fmt_spec.offset));
            // data size
            check_io(4, fwrite(&data_size, 4, 1, logfile), "flush");

            fflush(logfile);

            Channel c(id, data_size);
            channel_sizes[c.id] = data_size;
            return c;
        }
    }
}
