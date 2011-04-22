#include <cstdio>
#include <cstring>
#include <iostream>
#include <tidelog.hpp>

using namespace tide::log;
using namespace std;

int main(int argc, char* argv[])
{
    try {
        const size_t BUFSIZE = 1024;
        unsigned char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE);
        FILE* streambuf = fmemopen(buf, BUFSIZE, "wb");
        
        TIDELog logf(streambuf);
        for(int i = 0; i < 30; ++i)
            fprintf(stdout, "%x/%d/%c\n", buf[i], buf[i], buf[i]);
        fflush(stdout);
    } catch(const std::exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
