#include <iostream>
#include <cstdlib>
#include "fs_client.h"
#include <string>
#include <assert.h>
using std::cout;
using std::string;
int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session=1, seq=-3;


    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    
    fs_create("user1", "password1", session, seq++, "/dir", 'd');

    std::string name = "/";
    for(int i = 0; i < 1000; ++i) {
        name += 'a';
    }

    fs_create("user1", "password1", session, seq++, name.c_str(), 'd');
    // fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    // fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    // // assert(string(writedata,512) == string(readdata, 512));
    // for(unsigned int i = 0; i < FS_BLOCKSIZE; ++i) {
    //     printf("%c", readdata[i]);
    // }
    // printf("\n");
    // fs_delete("user1", "password1", session, seq++, "/dir/file");
    // fs_delete("user1", "password1", session, seq++, "/dir");
}