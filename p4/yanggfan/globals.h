#ifndef _GLOBALS_H_
#define _GLOBALS_H_
#include <vector>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>

// used to return the next available session number on FS_SESSION calls
extern unsigned int global_session;

// a boolean to keep track of if the session number overflows
extern bool max_sessions;

// maps a session number to a pair representing the username and the sequence number
extern std::unordered_map<unsigned int, std::pair<std::string, unsigned int> > session_map;

// lock for the session_map
extern std::mutex session_lock;

// record which disk blocks are currently in use and which are free
extern std::vector<bool> used_blocks;

// lock for used_blocks vector
extern std::mutex used_blocks_lock;

// maps username to password
extern std::unordered_map<std::string, std::string> user_password;

// vector of mutexes for inodes of fixed size
extern std::vector<std::shared_mutex> mutex_map;

#endif