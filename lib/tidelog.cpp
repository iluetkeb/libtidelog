#include "tidelog.hpp"

#include <sstream>
#include <errno.h>
#include <cstring>
#include <limits.h>

// Implementation notes:
//  * I'm using "sizeof" when determining how much to write, but a define in the CHECK, to make
//    sure the size of the data written matches what the protocol expects

#define HDR_TAG_SIZE 4
#define HDR_SIZE_SIZE 8

#define TIDE_MAJOR_SIZE 1
#define TIDE_MINOR_SIZE 1
#define TIDE_NCHAN_SIZE 4
#define TIDE_NCHUNK_SIZE 4

#define SMALL_ARRAY_SIZE_SIZE 1
#define ARRAY_SIZE_SIZE 4

namespace tide { namespace log {
	BufferReference::BufferReference(const char* bytes, const uint64_t length, const uint64_t offset) : 
		bytes(bytes), length(length), offset(offset) {
	}


	TIDELog::TIDELog(const std::string& logfile_name) {
		logfile = fopen(logfile_name.c_str(), "wb");
		if(logfile == NULL) {
			throw IOException(strerror(errno));
		}
		writeTIDE();
	}
	TIDELog::~TIDELog() {
		writeTIDE(); // update header
		fclose(logfile);
	}

namespace {
	inline void check_io(const int expected, const int actual) {
		if(expected != actual) {
			throw IOException(strerror(errno));
		}
	}

	inline void check_bounds(const std::string& name, uint32_t max, uint32_t actual) {
		if(actual > max) {
			std::ostringstream msg;
			msg << "Size for " << name << "(" << actual << ") larger than max (" <<max + ")";
			throw IllegalArgumentException(msg.str());
		}
	}
}
	void TIDELog::writeHeader(const char tag[HDR_TAG_SIZE], uint64_t size) {
		check_io(HDR_TAG_SIZE, fwrite(tag, HDR_TAG_SIZE, 1, logfile));
		check_io(HDR_TAG_SIZE, fwrite(&size, sizeof(size), 1, logfile));
	}

	void TIDELog::writeTIDE() {
		fseek(logfile, 0, SEEK_SET);
		unsigned char TIDE[2];
		TIDE[0] = 1;
		TIDE[1] = 0;
		uint32_t num_channels = channel_sizes.size();

		check_io(2, fwrite(TIDE, 2, 1, logfile));
		check_io(TIDE_NCHAN_SIZE, fwrite(&num_channels, sizeof(num_channels), 1, logfile));
		check_io(TIDE_NCHUNK_SIZE, fwrite(&num_chunks, sizeof(num_chunks), 1, logfile));
		check_io(0, fflush(logfile));
	}

	void TIDELog::write_small(uint8_t size, const void* data) {
		check_io(SMALL_ARRAY_SIZE_SIZE, fwrite(&size, sizeof(size), 1, logfile));
		check_io(size, fwrite(data, size, 1, logfile));
	}
	void TIDELog::write_array(uint32_t size, const void* data) {
		check_io(ARRAY_SIZE_SIZE, fwrite(&size, sizeof(size), 1, logfile));
		check_io(size, fwrite(data, size, 1, logfile));
	}

	Channel TIDELog::writeCHAN(const std::string& name, const std::string& type, const std::string& fmt_description, 
			const BufferReference& source_spec, const BufferReference& fmt_spec, uint32_t data_size) {
		// argument checking
		check_bounds("name", 256, name.length());
		check_bounds("type", 10, type.length());
		check_bounds("fmt_description", 256, fmt_description.length());
		check_bounds("source_spec", 256, source_spec.length);
		check_bounds("fmt_spec", UINT_MAX, fmt_spec.length);

		// ID
		const uint32_t id = channel_sizes.size() +1;
		check_io(4, fwrite(&id, 4, 1, logfile));
		// name
		write_small(name.length(), name.c_str());
		// type
		write_small(std::max((size_t)10, type.length()), type.c_str());
		// human-readable source description
		write_small(fmt_description.length(), fmt_description.c_str());
		// source string
		write_small(source_spec.length, source_spec.bytes + source_spec.offset);
		// format spec
		write_array(fmt_spec.length, fmt_spec.bytes + fmt_spec.offset);
		// data size
		check_io(4, fwrite(&data_size, 4, 1, logfile));
		
		fflush(logfile);

		Channel c(id, data_size);
		channel_sizes[c.id] = data_size;
		return c;
	}
}}
