#include "tidelog.hpp"

namespace tide {
    namespace log {

        TIDEException::TIDEException(const std::string& msg) : msg(msg) {

        }
        TIDEException::~TIDEException() throw() {
            
        }

        const char* TIDEException::what() const throw () {
            return msg.c_str();
        }

        IOException::IOException(const std::string& msg) : TIDEException(msg) {
        }

        IllegalArgumentException::IllegalArgumentException(const std::string& msg) : TIDEException(msg) {
        }
    }
}
