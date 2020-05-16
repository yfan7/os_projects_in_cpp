#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include "fs_client.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <vector>

using std::cout;

char *server;
int server_port;

void func1() {

    unsigned int session, seq = 0;

    fs_session("user1", "password1", &session, seq++);

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    // fs_readblock("user1", "password1", session, seq++, "/dir/file1", 0, readdata);
    
    fs_delete("user1", "password1", session, seq++, "/dir/file1");

}

void func2() {

    unsigned int session, seq = 0;

    fs_session("user1", "password1", &session, seq++);

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    fs_writeblock("user1", "password1", session, seq++, "/dir/file1",0, writedata);

}


int main(int argc, char *argv[])
{
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    unsigned int session, seq = 0;
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file1", 'f');
    
    std::thread t2(func2);
    std::thread t1(func1);


    t1.join();
    t2.join();


    // const int num_threads = 6;

    // std::vector<std::thread> threads;
    // int i = 0;
    // for (i = 0; i < num_threads; i++)
    // {
    //     if(i%2 == 0)
    //         threads.push_back(std::thread(func1, 0));
    //     else
    //         threads.push_back(std::thread(func2, 0));

    //     //std::this_thread::sleep_for (std::chrono::milliseconds(10));

    // }

    // auto it = threads.begin();
    // while (it != threads.end())
    // {
    //     it->join();
    //     it++;
    // }
}
