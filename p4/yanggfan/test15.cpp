#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;

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

    fs_create("user1", "password1", session, seq++, "/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file2", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/dir2", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/dir/file", 'f');

    fs_writeblock("user1", "password1", session, seq++, "dir/file", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/file/", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/file/file", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    unsigned int session2, seq2 = 0;
    fs_session("user2", "password2", &session2, seq2++);
    fs_create("user2", "password2", session, seq++, "/u2file", 'f');
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);

    fs_writeblock("user2", "password2", session2, seq2++, "/dir/file", 0, writedata);
    --seq2;
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/invalidfile/file", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user2", "password2", session2, seq2++, "/file", 0, writedata);
    --seq2;
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/invalidfile", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/dir/invalidfile", 0, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/dir/file", 1, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/dir/file", 2, writedata);
    --seq;
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

}