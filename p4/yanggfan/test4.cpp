#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, session1, seq(0), seq1=0;

    const char *writedata = "You should write We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    const char *overwritedata = "Overwritten! We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    
    const char *should_not_writedata = "You should not write! We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    // if (argc != 3) {
    //     cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
    //     exit(1);
    // }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_session("user2", "password2", &session1, seq1++);

    // valid create
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    // invalid create
    fs_create("user1", "password1", session, seq++, "/dir1/", 'd');
    fs_create("user1", "password1", session, seq++, "dir1", 'd');
    fs_create("user1", "password1", session, seq++, "/dir1", 'c');
    fs_create("user1", "password2", session, seq++, "/dir1", 'd');
    fs_create("user2", "password2", session, seq1++, "/dir1", 'd');
    fs_create("user2", "password2", session1, seq1++, "/dir/file1", 'f');
    

    // valid write
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 1, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 1, readdata);
    for(unsigned int i = 0; i < FS_BLOCKSIZE; ++i) {
        printf("%c", readdata[i]);
    }
    printf("\n");
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 1, overwritedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 1, readdata);
    for(unsigned int i = 0; i < FS_BLOCKSIZE; ++i) {
        printf("%c", readdata[i]);
    }
    printf("\n");

    // invalid write
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", FS_MAXFILEBLOCKS, writedata);
    fs_writeblock("user1", "password1", session, seq++, "dir/file", 0, should_not_writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir", 0, should_not_writedata);
    fs_writeblock("user1", "password1", session, seq++, "dir/", 0, should_not_writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file1", 0, should_not_writedata);
    fs_writeblock("user2", "password2", session1, seq1++, "/dir/file", 0, should_not_writedata);


    // invalid read
    fs_readblock("user1", "password1", session, seq++, "/dir/file/", 1, readdata);
    fs_readblock("user1", "password1", session, seq++, "dir/file/", 1, readdata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file1", 0, readdata);
    fs_readblock("user2", "password2", session1, seq1++, "/dir/file", 0, readdata);
    
    
    // valid delete
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    // invalid delete
    fs_delete("user2", "password2", session1, seq1++, "/dir");
    // valid delete
    fs_delete("user1", "password1", session, seq++, "/dir");

    // invalid delete
    fs_delete("user1", "password1", session, seq++, "/dir/file/");
    fs_delete("user1", "password1", session, seq++, "dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir/file1");
    fs_delete("user2", "password2", session1, seq1++, "/dir/file");
    fs_delete("user2", "password2", session1, seq1++, "/dir");

}