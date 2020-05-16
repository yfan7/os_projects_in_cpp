#include <iostream>
#include <cstdlib>
    #include <stdio.h>

#include "fs_client.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <vector>
#include <stdlib.h>     /* srand, rand */

using std::cout;

char *server;
int server_port;

void func1(std::pair<std::string, char> filename) {

    unsigned int session, seq = 0;

    fs_session("user1", "password1", &session, seq++);

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    srand (time(NULL));
    
    fs_create("user1", "password1", session, seq++, filename.first.c_str(), filename.second);

    for(int j = 0; j < 500; ++j) {
        int y = rand() % 2;
        while(y == 0) {
            std::this_thread::yield();
            y = rand() % 2;
        }

        int num = rand() % 2;


        if(num == 0) {
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 0, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 1, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 2, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 3, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 4, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 5, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 6, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 7, writedata);
            fs_writeblock("user1", "password1", session, seq++, filename.first.c_str(), 8, writedata);
        }
        else {
            fs_delete("user1", "password1", session, seq++, filename.first.c_str());
            fs_create("user1", "password1", session, seq++, filename.first.c_str(), filename.second);
            fs_delete("user1", "password1", session, seq++, filename.first.c_str());
            fs_create("user1", "password1", session, seq++, filename.first.c_str(), filename.second);
            fs_delete("user1", "password1", session, seq++, filename.first.c_str());
            fs_create("user1", "password1", session, seq++, filename.first.c_str(), filename.second);
            fs_delete("user1", "password1", session, seq++, filename.first.c_str());
            fs_create("user1", "password1", session, seq++, filename.first.c_str(), filename.second);
        }

    }

}



int main(int argc, char *argv[])
{
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);


    const int num_threads = 31;

    std::vector<std::thread> threads;
    std::vector<std::pair<std::string, char> > names;
    names.push_back(std::make_pair("/file", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file2", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file3", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file4", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file5", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file6", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file7", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file8", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir/file9", 'f'));

    names.push_back(std::make_pair("/dir/dir/dir/file1", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/file2", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir2/file2", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir2/file1", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/file", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/file2", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/file3", 'f'));
    names.push_back(std::make_pair("/dir/file5", 'f'));
    names.push_back(std::make_pair("/dir/file6", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir2/file6", 'f'));
    names.push_back(std::make_pair("/dir/dir/dir2/file6", 'f'));
    names.push_back(std::make_pair("/dir/dir/file6", 'f'));
    names.push_back(std::make_pair("/dir/dir/file5", 'f'));
    names.push_back(std::make_pair("/dir/dir/file3", 'f'));
    names.push_back(std::make_pair("/dir/dir/file2", 'f'));


    names.push_back(std::make_pair("/dir", 'd'));
    names.push_back(std::make_pair("/dir2", 'd'));
    names.push_back(std::make_pair("/dir/dir", 'd'));
    names.push_back(std::make_pair("/dir/dir/dir", 'd'));
    names.push_back(std::make_pair("/dir/dir/dir2", 'd'));
    names.push_back(std::make_pair("/dir/dir/dir/dir", 'd'));
    names.push_back(std::make_pair("/dir/dir/dir/dir/dir", 'd'));


    int i = 0;
    for (i = 0; i < num_threads*2; i++)
    {

        threads.push_back(std::thread(func1, names[i%num_threads]));
        //std::this_thread::sleep_for (std::chrono::milliseconds(10));

    }

    auto it = threads.begin();
    while (it != threads.end())
    {
        it->join();
        it++;
    }
}
