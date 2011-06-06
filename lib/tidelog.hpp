#ifndef __LIBTIDELOG_TIDELOG_HPP
#define __LIBTIDELOG_TIDELOG_HPP

#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <stdint.h>
#include <sys/time.h>
#include <list>

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
            const std::string name, source_name, type;
        private:
            std::vector<uint8_t> source_config, format;
            
        public:
            
            Channel(int id, const std::string& name, const std::string& type,
                const std::string& source_name, const Array& source_spec,
                const Array& fmt_spec) : id(id), name(name), source_name(source_name), type(type) {
                source_config.reserve(source_spec.length);
                memcpy(&(source_config[0]), source_spec.bytes, source_spec.length);
                format.reserve(fmt_spec.length);
                memcpy(&(format[0]), fmt_spec.bytes, fmt_spec.length);                
            };
            
            const std::vector<uint8_t>& get_source_config() const {
                return source_config;
            }
            const std::vector<uint8_t>& get_format() const {
                return format;
            }
        };

        class Chunk;

        // NOTE: This class is most definitely not thread-safe. Sync before.
        class TIDELog {
        private:
            FILE* logfile;
            uint32_t num_chunks, num_chans;
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
                    const SArray& source_spec, const Array& fmt_spec);
            void write(const Channel& c, const timeval& tstamp, const Array& data);
            
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
