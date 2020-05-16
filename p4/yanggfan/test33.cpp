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

void func1(int index) {

    unsigned int session, seq = 0;

    fs_session("user1", "password1", &session, seq++);

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    std::string temp2 = std::string("/dir") + std::to_string(index);
    for(int j = 0; j < 10; ++j) {
        fs_create("user1", "password1", session, seq++, temp2.c_str(), 'd');
        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_create("user1", "password1", session, seq++, temp.c_str(), 'f');
            //std::cout << "creating: file" << i << "\n";

        }

        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_writeblock("user1", "password1", session, seq++, temp.c_str(), 0, writedata);
            //std::cout << "writing: file" << i << "\n";
        }

        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_create("user1", "password1", session, seq++, temp.c_str(), 'f');
            //std::cout << "creating: file" << index + " " + i << "\n";
        }

        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_readblock("user1", "password1", session, seq++, temp.c_str(), 0, readdata);
            //std::cout << "reading: file" << i << "\n";
        }


        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_delete("user1", "password1", session, seq++, temp.c_str());
            //std::cout << "deleting: file" << i << "\n";
        }

        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_writeblock("user1", "password1", session, seq++, temp.c_str(), 0, writedata);
            //std::cout << "writing: file" << i << "\n";
        }


        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_readblock("user1", "password1", session, seq++, temp.c_str(), 0, readdata);
            //std::cout << "reading: file" << i << "\n";
        }

        for(int i = 0; i < 10; ++i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_delete("user1", "password1", session, seq++, temp.c_str());
            //std::cout << "deleting: file" << index + " " + i << "\n";
        }
        fs_delete("user1", "password1", session, seq++, temp2.c_str());
    }

    std::cout << "sleeping\n";
    std::this_thread::sleep_for (std::chrono::seconds(15));

}

void func2(int index) {

    unsigned int session, seq = 0;

    fs_session("user1", "password1", &session, seq++);

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    std::string temp2 = std::string("/dir") + std::to_string(index);
    for(int j = 0; j < 10; ++j) {
        fs_create("user1", "password1", session, seq++, temp2.c_str(), 'd');
        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_create("user1", "password1", session, seq++, temp.c_str(), 'f');
            //std::cout << "creating: file" << i << "\n";

        }

        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_writeblock("user1", "password1", session, seq++, temp.c_str(), 0, writedata);
            //std::cout << "writing: file" << i << "\n";
        }

        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_create("user1", "password1", session, seq++, temp.c_str(), 'f');
            //std::cout << "creating: file" << index + " " + i << "\n";
        }

        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_readblock("user1", "password1", session, seq++, temp.c_str(), 0, readdata);
            //std::cout << "reading: file" << i << "\n";
        }

        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/dir") + std::to_string(index) + std::string("/file") + std::to_string(i);
            fs_delete("user1", "password1", session, seq++, temp.c_str());
            //std::cout << "deleting: file" << i << "\n";
        }

        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_writeblock("user1", "password1", session, seq++, temp.c_str(), 0, writedata);
            //std::cout << "writing: file" << i << "\n";
        }

        if(j == 5) {
            std::this_thread::sleep_for (std::chrono::seconds(15));
        }


        
        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_readblock("user1", "password1", session, seq++, temp.c_str(), 0, readdata);
            //std::cout << "reading: file" << i << "\n";
        }


        for(int i = 9; i >= 0; --i) {
            std::string temp = std::string("/file") + std::to_string(index) + "-" + std::to_string(i);
            fs_delete("user1", "password1", session, seq++, temp.c_str());
            //std::cout << "deleting: file" << index + " " + i << "\n";
        }

        fs_delete("user1", "password1", session, seq++, temp2.c_str());
    }

    std::cout << "sleeping\n";
    std::this_thread::sleep_for (std::chrono::seconds(15));

}


int main(int argc, char *argv[])
{
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);


    const int num_threads = 6;

    std::vector<std::thread> threads;
    int i = 0;
    for (i = 0; i < num_threads; i++)
    {
        if(i%2 == 0)
            threads.push_back(std::thread(func1, 0));
        else
            threads.push_back(std::thread(func2, 0));

        //std::this_thread::sleep_for (std::chrono::milliseconds(10));

    }

    auto it = threads.begin();
    while (it != threads.end())
    {
        it->join();
        it++;
    }
}
