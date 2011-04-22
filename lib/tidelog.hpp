#ifndef __LIBTIDELOG_TIDELOG_HPP
#define __LIBTIDELOG_TIDELOG_HPP

#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <stdint.h>
#include <sys/time.h>

/*
 * This is an implementation of the TIDE log format, described in https://retf.info/svn/drafts/rd-0001.txt
 * (in the version from 4th April 2011).
 * It is intended primarily to demonstrate implementability of that specification.
 *
 * This code is provided as-is, without any guarantees of any kind, not even implied ones. In other words, use
 * it at your own risk. It may be freely used for any purpose.
 *
 * Copyright (c) 2011 Ingo Lütkebohle <iluetkeb@techfak.uni-bielefeld.de>, Bielefeld University
 */

namespace tide {
    namespace log {
        template<typename T>
        class BufferReference {
        public:
            BufferReference(const char* bytes, const T length) : bytes(bytes), length(length) {};
            BufferReference(const char* null_terminated_string) : bytes(null_terminated_string), length(strlen(null_terminated_string)){};

            const void* bytes;
            const T length;
        };
        typedef BufferReference<uint8_t> SArray;
        typedef BufferReference<uint32_t> Array;

        class Channel {
        public:
            const int id;
            const uint32_t data_size;

            Channel(int id, uint32_t size) : id(id), data_size(size) {
            };
        };

        class Entry {
        public:
            const Channel c;
            const timeval timestamp;
            Entry(const Channel& c, const timeval& tv);
        };

        class Chunk {
        public:
            const int id, num_entries;
            const off_t start_filepos;
            timeval tv_start, tv_end;

            Chunk(int id, const off_t start_filepos);

            void add_entry(const Entry& e);
        };

        class TIDELog {
        private:
            FILE* logfile;
            uint32_t num_chunks;
            std::map<int, uint32_t> channel_sizes;
            Chunk *current_chunk;

            TIDELog(const TIDELog&); // not implemented to prevent copying

            template<typename T, unsigned int SIZE> inline void write_checked(const T& data, const char* name = "<unspecified>");
            /* special template for partial specialization with known types. if you get "undefined symbol", use the above instead. */
            template<typename T> inline void write_checked(const T& data, const char* name = "<unspecified>");

            void writeTIDE();
            void writeCHUNK();
            void start_chunk();
            void finish_chunk();
        public:
            TIDELog(const std::string& logfile_name);
            TIDELog(FILE* stream);
            ~TIDELog();

            Channel writeCHAN(const std::string& name, const std::string& type, const std::string& source,
                    const SArray& source_spec, const Array& fmt_spec, uint32_t data_size);
        };

        class TIDEException : public std::exception {
        private:
            const std::string msg;
        public:
            TIDEException(const std::string&);
            ~TIDEException() throw ();
            virtual const char* what() const throw ();
        };

        class IOException : public TIDEException {
        public:
            IOException(const std::string& msg);
        };

        class IllegalArgumentException : public TIDEException {
        public:
            IllegalArgumentException(const std::string& msg);
        };


    }
}

#endif
