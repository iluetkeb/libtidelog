#include <cstdio>
#include <cstring>
#include <iostream>
#include <tidelog.hpp>

using namespace tide::log;
using namespace std;

int main(int argc, char* argv[])
{
    unsigned int length = 135;
    const unsigned char expected[]  = {
        // FILE HEADER
        'T', 'I', 'D', 'E', 
        length, 0, 0, 0, // size 1-4
        0, 0, 0, 0, // size, 5-8
        1, 0,  // major/minor
        1, 0, 0, 0, // num chans
        1, 0, 0, 0, // num_chunks
        // CHANNEL
        'C', 'H', 'A', 'N',
        45, 0, 0, 0,
        0, 0, 0, 0,
        1, 0, 0, 0, // channel id
        6, // name size
        'M', 'Y', 'C', 'H', 'A', 'N',
        6, 0, 0, 0, // type size
        'M', 'Y', 'T', 'Y', 'P', 'E', 
        8, 0, 0, 0, // source name size
        'M', 'Y', 'S', 'O', 'U', 'R', 'C', 'E',
        1, 0, 0, 0, // source config size 
        'S',
        3, 0, 0, 0, // fmt size
        'F', 'M', 'T',
        'C', 'H', 'N', 'K',
        25 /* chunk hdr size */ + 16 /* field header size */ + 3 /* buffer size */, 0, 0, 0, 
        0, 0, 0, 0,
        1, 0, 0, 0, // id
        1, 0, 0, 0, // count
        128, 0, 0, 0, 0, 0, 0, 0, // start
        128, 0, 0, 0, 0, 0, 0, 0, // end
        0, // no compression
        1, 0, 0, 0, // channel id
        128, 0, 0, 0, // tstamp
        0, 0, 0, 0,
        3, 0, 0, 0,
        1, 2, 3                        
    };

    const size_t BUFSIZE = 1024;
    unsigned char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    try {
        FILE* streambuf = fmemopen(buf, BUFSIZE, "wb");
        
        TIDELog logf(streambuf);
        Channel c = logf.writeCHAN("MYCHAN", "MYTYPE", "MYSOURCE", Array("S"), Array("FMT"));
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 128;
        char buf[] = { 1, 2, 3 };
        logf.write(c, tv, Array(buf, 3));
    } catch(const std::exception& ex) {
        cerr << ex.what() << endl;
    }
    for(unsigned int i = 0; i < length; ++i) {
        if(expected[i] != buf[i]) {
            cerr << i << " mismatch " << ": actual " << (int)buf[i] << "( " << (char)(buf[i]) << ") != " << (int)expected[i] << " " << (char)expected[i] << endl;
        } else {
            cerr << i << " " << (int)buf[i] << "(" << (char)buf[i] << ")" << endl;
        }
    }
    
    return 0;
}
