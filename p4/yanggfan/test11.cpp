#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session1, seq1=0;
    unsigned int session2, seq2=0;

    // const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    // char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session1, seq1++);
    fs_session("user2", "password22", &session2, seq2++);

    fs_create("user1", "password1", session1, seq1++, "/dir", 'd');
    fs_create("user1", "password1", session1, seq1++, "/dir/file", 'f');

    fs_create("user2", "password2", session2, seq2++, "/dir", 'd');
    fs_create("user2", "password2", session2, seq2++, "/dir/file", 'f');

    fs_create("user1", "password1", session2, seq2++, "/dir/file2", 'f');
    --seq2;
    fs_create("user2", "password2", session2, seq2++, "/dir/file2", 'f');

    fs_create("user2", "password2", session1, seq1++, "/dir/file3", 'f');
    fs_create("user2", "password2", session1, seq1++, "/dir/file4", 'f');
    --seq1;
    --seq1;
    --seq1;
    fs_create("user1", "password1", session1, seq1++, "/dir/file3", 'f');
    fs_create("user1", "password1", session1, seq1++, "/dir/file4", 'f');
    fs_create("user1", "password1", session1, seq1++, "/dir/file5", 'f');


    fs_delete("user1", "password1", session1, seq1++, "/dir/file5");
    fs_delete("user1", "password1", session1, seq1++, "/dir/file4");
    fs_delete("user1", "password1", session1, seq1++, "/dir/file3");
    fs_delete("user1", "password1", session1, seq1++, "/dir/file2");
    fs_delete("user1", "password1", session1, seq1++, "/dir/file1");
    fs_delete("user1", "password1", session1, seq1++, "/dir");

    fs_delete("user2", "password2", session2, seq2++, "/dir/file4");
    fs_delete("user2", "password2", session2, seq2++, "/dir/file3");
    fs_delete("user2", "password2", session2, seq2++, "/dir/file2");
    fs_delete("user2", "password2", session2, seq2++, "/dir/file1");
    fs_delete("user2", "password2", session2, seq2++, "/dir");
}