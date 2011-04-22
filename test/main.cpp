#include <cstdio>
#include <cstring>
#include <iostream>
#include <tidelog.hpp>

using namespace tide::log;
using namespace std;

int main(int argc, char* argv[])
{
    const unsigned char expected[]  = {
        // FILE HEADER
        'T', 'I', 'D', 'E', 
        78, 0, 0, 0, // size 1-4
        0, 0, 0, 0, // size, 5-8
        1, 0,  // major/minor
        0, 0, 0, 0, // num chans
        0, 0, 0, 0, // num_chunks
        // CHANNEL
        'C', 'H', 'A', 'N',
        55, 0, 0, 0,
        0, 0, 0, 0,
        1, 0, 0, 0, // channel id
        6, // name size
        'M', 'Y', 'C', 'H', 'A', 'N',
        'M', 'Y', 'T', 'Y', 'P', 'E', 0, 0, 0, 0,
        8, // source string size
        'M', 'Y', 'S', 'O', 'U', 'R', 'C', 'E',
        1, // source size 
        'S',
        3, 0, 0, 0, // fmt size
        'F', 'M', 'T',
        1, 0, 0, 0 // data size
    };

    const size_t BUFSIZE = 1024;
    unsigned char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    try {
        FILE* streambuf = fmemopen(buf, BUFSIZE, "wb");
        
        TIDELog logf(streambuf);
        logf.writeCHAN("MYCHAN", "MYTYPE", "MYSOURCE", BufferReference("S"), BufferReference("FMT"), 1);
    } catch(const std::exception& ex) {
        cerr << ex.what() << endl;
    }
    for(int i = 0; i < 14; ++i) {
        if(expected[i] != buf[i]) {
            cerr << "mismatch at " << i << ": " << (int)buf[i] << " != " << (int)expected[i] << endl;
        }
    }
    
    return 0;
}
