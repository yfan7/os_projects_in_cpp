#include <iostream>
#include <cstdlib>
#include "fs_client.h"
#include <string>
using std::cout;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file1", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file2", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file3", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file4", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file5", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file6", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file7", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file8", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file9", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file9");

    // use up blocks for the directory /dir
    for (unsigned int i = 0; i < FS_MAXFILEBLOCKS - 1; ++i) {
        int suffix = (i + 1) * 8;
        std::string filename = "/dir/file" + std::to_string(suffix);
        fs_create("user1", "password1", session, seq++, filename.c_str(), 'f');
    }

    // this creation should be invalid
    fs_create("user1", "password1", session, seq++, "/dir/file_final", 'f');

}