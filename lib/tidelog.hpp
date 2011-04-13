#ifndef __LIBTIDELOG_TIDELOG_HPP
#define __LIBTIDELOG_TIDELOG_HPP

#include <fstream>
#include <string>
#include <map>
#include <stdint.h>

namespace tide { namespace log {
	class BufferReference {
	public:
		BufferReference(const char* bytes, const uint64_t length, const uint64_t offset);

		const char* bytes;
		const uint64_t length;
		const uint64_t offset;
	};

	class Channel {
	public:
		const int id;
		const uint32_t data_size;

		Channel(int id, uint32_t size) : id(id), data_size(size) {};
	};
	class Chunk {
	private:
		const int id;
		const uint64_t start_filepos;
	public:
		Chunk(int id, uint64_t start_filepos);
	};

	class TIDELog {
	private:
		FILE* logfile;
		uint32_t num_chunks;
		std::map<int, uint32_t> channel_sizes;

		TIDELog(const TIDELog&); // not implemented to prevent copying
		void write_small(uint8_t size, const void* data);
		void write_array(uint32_t size, const void* data);
		void writeHeader(const char tag[4], uint64_t size);
		void writeTIDE();
	public:
		TIDELog(const std::string& logfile_name);
		~TIDELog();

		Channel writeCHAN(const std::string& name, const std::string& type, const std::string& fmt_description, 
			const BufferReference& source_spec, const BufferReference& fmt_spec, uint32_t data_size);
	};

	class IOException : public std::exception {
	private:
		const std::string& msg;
	public:
		IOException(const std::string& msg);
		const char* what() throw();
	};

	class IllegalArgumentException : public std::exception {
	private:
		const std::string& msg;
	public:
		IllegalArgumentException(const std::string& msg);
		const char* what() throw();
	};


}}

#endif
