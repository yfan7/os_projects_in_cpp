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

    // char readdata[FS_BLOCKSIZE];

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
    fs_create("user1", "password1", session, seq++, "/dir2", 'd');

    fs_delete("user1", "password1", session, seq++, "dir/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file/");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name/really/long/path/name");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file/file/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    unsigned int session2, seq2 = 0;
    fs_session("user2", "password2", &session2, seq2++);
    fs_create("user2", "password2", session, seq++, "/u2file", 'f');
    fs_writeblock("user2", "password2", session2, seq2++, "/u2file", 0, writedata);

    fs_delete("user2", "password2", session2, seq2++, "/dir/file/file");
    --seq2;
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_create("user2", "password2", session, seq++, "/u2file", 'f');

    fs_delete("user2", "password2", session2, seq2++, "/dir/file");
    --seq2;
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_create("user2", "password2", session, seq++, "/u2file", 'f');

    fs_delete("user1", "password1", session, seq++, "/invalidfile/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/invalidfile/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/invalidfile/file/file");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/dir/invalidfile");
    --seq;
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file");
    --seq;
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file2");
    --seq;
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user2", "password2", session2, seq2++, "/file");
    --seq2;
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_delete("user2", "password2", session2, seq2++, "/u2file");
    fs_create("user2", "password2", session, seq++, "/u2file", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir2");
    --seq;
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_delete("user1", "password1", session, seq++, "/file");
    --seq;
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

}