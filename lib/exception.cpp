#include "tidelog.hpp"

namespace tide { namespace log {
	IOException::IOException(const std::string& msg) : msg(msg) {
	}
	IllegalArgumentException::IllegalArgumentException(const std::string& msg) : msg(msg) {
	}
}}
