#include <iostream>
#include <cstdlib>
#include "fs_client.h"
#include <string>
#include <assert.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

using std::cout;
using std::string;
int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session=1, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir2", 'd');
    for(int i = 0; i < 1500; ++i) {
        std::string temp = std::string("/dir/file") + std::to_string(i);
        fs_create("user1", "password1", session, seq++, temp.c_str(), 'f');
    }


    // fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    // fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    // // assert(string(writedata,512) == string(readdata, 512));
    // for(unsigned int i = 0; i < FS_BLOCKSIZE; ++i) {
    //     printf("%c", readdata[i]);
    // }
    // printf("\n");
    fs_delete("user1", "password1", session, seq++, "/dir/file0");
    fs_delete("user1", "password1", session, seq++, "/dir/file1");
    fs_delete("user1", "password1", session, seq++, "/dir/file2");
    fs_delete("user1", "password1", session, seq++, "/dir/file3");
    fs_delete("user1", "password1", session, seq++, "/dir/file4");
    fs_delete("user1", "password1", session, seq++, "/dir2/file5");
    fs_delete("user1", "password1", session, seq++, "/dir2/file6");
    fs_delete("user1", "password1", session, seq++, "/dir2/file7");
    fs_delete("user1", "password1", session, seq++, "/dir2/file8");
    fs_delete("user1", "password1", session, seq++, "/dir2/file9");

    //std::this_thread::yield();
    fs_delete("user1", "password1", session, seq++, "/dir");
    fs_delete("user1", "password1", session, seq++, "/dir2");

}