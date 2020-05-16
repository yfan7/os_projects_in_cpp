#include "globals.h"
#include "fs_server.h"

// used to return the next available session number on FS_SESSION calls
unsigned int global_session = 0;

// a boolean to keep track of if the session number overflows
bool max_sessions = false;

// maps a session number to a pair representing the username and the sequence number
std::unordered_map<unsigned int, std::pair<std::string, unsigned int> > session_map;

// lock for the session_map
std::mutex session_lock;

// record which disk blocks are currently in use and which are free
std::vector<bool> used_blocks(FS_DISKSIZE, false);

// lock for used_blocks vector
std::mutex used_blocks_lock;

// maps username to password
std::unordered_map<std::string, std::string> user_password;

// vector of mutexes for inodes of fixed size
std::vector<std::shared_mutex> mutex_map(FS_DISKSIZE);
