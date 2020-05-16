#include "fs_handlers.h"
#include "fs_server.h"
#include "globals.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <limits.h>  

// handles the FS_SESSION request from client
int session_handler(char* username, std::vector<char*> message, char* &return_msg) {
    // check if the global_session variable has already reached it's maximum value
    session_lock.lock();
    if(max_sessions) {
        session_lock.unlock();
        return -1;
    }
    session_lock.unlock();

    // retrieve next session value to return
    session_lock.lock();
    unsigned int cur_session = global_session++;
    max_sessions = (global_session == 0);
    session_lock.unlock();

    // error handling for sequence number
    unsigned long sequence;
    try
    {
        size_t t1;
        sequence = std::stoul(message[2], &t1, 10);
        if (message[2][t1] != '\0' || *message[2] == '-') {
            return -1;
        }
    }
    catch(...)
    {
        return -1;
    }
    if(sequence > UINT_MAX) return -1;

    // add session number and username, sequence pair to session_map
    session_lock.lock();
    session_map[cur_session] = std::make_pair(std::string(username), (unsigned int)sequence);
    session_lock.unlock();

    std::string str_session = std::to_string(cur_session);
    std::string str_sequence = std::to_string((unsigned int)sequence);

    // construct return message
    int return_msg_size = str_session.length() + str_sequence.length() + 2;
    return_msg = (char*)malloc(return_msg_size);
    construct_return_msg(return_msg, str_session.c_str(), str_sequence.c_str());
    return return_msg_size;
}

// handles the FS_CREATE request from client
int readblock_handler(char* username, std::vector<char*> message, char* &return_msg) {
    // The session and the sequence passed in already succeed in parsing checking for stoul function
    // So it's safe to call stoul here directly and cast them to unsigned int            
    unsigned int session = (unsigned int)std::stoul(message[1]);
    unsigned int sequence = (unsigned int)std::stoul(message[2]);
    std::string pathname = message[3];

    // update sequence number
    session_lock.lock();
    session_map[session].second = sequence;
    session_lock.unlock();

    // check if the pathname is valid             
    if (!is_pathname_valid(pathname)) return -1;

    // error handling for block number
    unsigned int block = 0;
    try {
        size_t  temp;
        block = std::stoi(message[4], &temp, 10);
        if (message[4][temp] != '\0') {
            return -1;
        }
    }
    catch(...) {
        return -1;
    }
    
    // finds all the indices of '/' in the pathname
    std::vector<int> indices;
    file_splitter(pathname, indices);

    fs_inode cur_inode;
    // start from the root     
    unsigned int cur_inode_block = 0;    
    if(!traverse(cur_inode, cur_inode_block, pathname, false, username)) return -1;
    // cur_inode now contains inode of parent directory and it is read locked.

    // traversing to the inode of file    
    fs_direntry direntries[FS_DIRENTRIES];
    bool found = false;
    // loop through the blocks of the file inode        
    for (unsigned int i = 0; i < cur_inode.size; ++i) {
        // read in the data block           
        disk_readblock(cur_inode.blocks[i], (void*)&direntries);
        // loop through all direntries in the data block    
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            if (direntries[j].inode_block != 0 && std::string(direntries[j].name) == pathname.substr(indices.back()+1)){
                
                //found the file
                mutex_map[direntries[j].inode_block].lock_shared();
                mutex_map[cur_inode_block].unlock_shared();

                cur_inode_block = direntries[j].inode_block;
                disk_readblock(cur_inode_block, (void*)&cur_inode);

                //check ownership and type
                if (strcmp(cur_inode.owner, username) != 0 || cur_inode.type != 'f') {
                    mutex_map[cur_inode_block].unlock_shared();
                    return -1;
                }
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // allocate buffer to read disk block into
    char buff[FS_BLOCKSIZE];

    // file not found or invalid block number to read from
    if(!found || block >= cur_inode.size) {
        mutex_map[cur_inode_block].unlock_shared();
        return -1;
    }
    
    // read disk block
    disk_readblock(cur_inode.blocks[block], (void*)buff);
   
    // unlock all held locks
    mutex_map[cur_inode_block].unlock_shared();

    // construct return message
    int return_msg_size = strlen(message[1]) + strlen(message[2]) + 2 + FS_BLOCKSIZE;
    return_msg = (char*) malloc(return_msg_size);
    construct_return_msg(return_msg, message[1], message[2]);
    memcpy(return_msg + strlen(message[1]) + 2 + strlen(message[2]), buff, FS_BLOCKSIZE);
    return return_msg_size;

}

// handles the FS_WRITEBLOCK request from client
int writeblock_handler(char* username, std::vector<char*> message, char* &return_msg) {
    // The session and the sequence passed in already succeed in parsing checking for stoul function
    // So it's safe to call stoul here directly and cast them to unsigned int    
    unsigned int session = (unsigned int)std::stoul(message[1]);
    unsigned int sequence = (unsigned int)std::stoul(message[2]);
    std::string pathname = message[3];

    // update sequence number
    session_lock.lock();
    session_map[session].second = sequence;
    session_lock.unlock();

    // check if the pathname is valid             
    if (!is_pathname_valid(pathname)) return -1;

    // error handling for block number
    size_t temp;
    unsigned int block = 0;
    try{
        block = std::stoi(message[4], &temp, 10);
        if (message[4][temp] != '\0') {
            return -1;
        }
    }
    catch(...) {

        return -1;
    }

    // finds all the indices of '/' in the pathname
    std::vector<int> indices;
    file_splitter(pathname, indices);

    
    fs_inode cur_inode;
    // start from the root     
    unsigned int cur_inode_block = 0;
    if (!traverse(cur_inode, cur_inode_block, pathname, false, username)) return -1;
    // cur_inode now contains inode of parent directory and it is read locked.

    // traversing to the inode of file    
    fs_direntry direntries[FS_DIRENTRIES];
    bool found = false;
    // loop through the blocks of the file inode        
    for (unsigned int i = 0; i < cur_inode.size; ++i) {
        // read in the data block            
        disk_readblock(cur_inode.blocks[i], (void*)&direntries);
        // loop through all direntries in data block
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            if (direntries[j].inode_block != 0 && std::string(direntries[j].name) == pathname.substr(indices.back()+1)){
                //found the file
                mutex_map[direntries[j].inode_block].lock();
                mutex_map[cur_inode_block].unlock_shared();
                cur_inode_block = direntries[j].inode_block;
                
                // read in inode of found file
                disk_readblock(cur_inode_block, (void*)&cur_inode);
                
                //check ownership and type
                if (strcmp(cur_inode.owner, username) != 0 || cur_inode.type != 'f') {
                    mutex_map[cur_inode_block].unlock();
                    return -1;
                }

                found = true;
                break;
            }
        }
        if (found) break;
    }

    // file not found or invalid block number to write to
    if(!found || block > cur_inode.size) {
        mutex_map[cur_inode_block].unlock_shared();
        return -1;
    }
    
    if (block == cur_inode.size){
        // writing to a new block (grow file)
        int file_block = get_empty_block();
        if (file_block == -1) {
            mutex_map[cur_inode_block].unlock();

            return -1;
        }         
        
        cur_inode.blocks[block] = file_block;
        ++cur_inode.size;

        //write to datablocks
        disk_writeblock(cur_inode.blocks[block], (void*)message[5]);
        //write to file inode
        disk_writeblock(cur_inode_block, (void*)&cur_inode);
    }
    else {
        //writing to an existing block
        disk_writeblock(cur_inode.blocks[block], (void*)message[5]);
    }

    // unlock all held locks
    mutex_map[cur_inode_block].unlock();
    
    // construct return message
    int return_msg_size = strlen(message[1]) + strlen(message[2]) + 2;
    return_msg = (char*) malloc(return_msg_size);
    construct_return_msg(return_msg, message[1], message[2]);
    return return_msg_size;
}

// handles the FS_READBLOCK request from client
int create_handler(char* username, std::vector<char*> message, char* &return_msg) {
    // The session and the sequence passed in already succeed in parsing checking for stoul function
    // So it's safe to call stoul here directly and cast them to unsigned int     
    unsigned int session = (unsigned int)std::stoul(message[1]);
    unsigned int sequence = (unsigned int)std::stoul(message[2]);
    std::string pathname = message[3];
    
    // update sequence number
    session_lock.lock();
    session_map[session].second = sequence;
    session_lock.unlock();

    // check if the pathname is valid             
    if (!is_pathname_valid(pathname)) return -1;

    // finds all the indices of '/' in the pathname
    std::vector<int> indices;
    file_splitter(pathname, indices);

    // reserve one disk block for the inode of the new file
    int new_inode_block = -1;
    new_inode_block = get_empty_block();
    if(new_inode_block == -1) {        
        return -1;
    }
    
    fs_inode cur_inode;
    // start from the root     
    unsigned int cur_inode_block = 0;
    if(!traverse(cur_inode, cur_inode_block, pathname, true, username)) {
        // deallocate the reserved data block
        used_blocks_lock.lock();
        used_blocks[new_inode_block] = false;
        used_blocks_lock.unlock();
        return -1;
    }  
    // cur_inode now contains inode of parent directory and it is write locked.

    // traversing to the inode of file    
    fs_direntry direntries[FS_DIRENTRIES];
    fs_direntry temp_direntries[FS_DIRENTRIES];
    int empty_direntry = -1;
    int direntry_block = -1;
    for (unsigned int i = 0; i < cur_inode.size; ++i) {
        // if empty direntry already found read into temp_direntries to not overwrite direntries
        fs_direntry* direntries_read;
        if(direntry_block == -1) {
            // read in the direntries block
            disk_readblock(cur_inode.blocks[i], (void*)&direntries);
            direntries_read = direntries;
        }
        else {
            // read in the direntries block
            disk_readblock(cur_inode.blocks[i], (void*)&temp_direntries);
            direntries_read = temp_direntries;
        }

        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            
            if (direntries_read[j].inode_block == 0 && empty_direntry == -1) {
                // found empty direntry
                direntry_block = cur_inode.blocks[i];
                empty_direntry = j;
            }
            if (direntries_read[j].inode_block != 0 && std::string(direntries_read[j].name) == pathname.substr(indices.back()+1)){
                // file with this name already exists
                mutex_map[cur_inode_block].unlock();
                
                // deallocate the reserved data block
                used_blocks_lock.lock();
                used_blocks[new_inode_block] = false;
                used_blocks_lock.unlock();
                return -1;
            }
        }
    }

    // inode for new file or dir
    fs_inode new_inode;
    // assign owner for the new file or dir
    strcpy(new_inode.owner, username);
    // initialize the size
    new_inode.size = 0;
    // assign inode type
    if (*message[4] == 'f') {
        new_inode.type = 'f';   
    }
    else if (*message[4] == 'd') {
        new_inode.type = 'd';   
    }
    else {
        // neither of 'f' or 'd'
        mutex_map[cur_inode_block].unlock();
        return -1;
    }

    fs_direntry new_entries[FS_DIRENTRIES];
    if (empty_direntry == -1) {
        if(cur_inode.size == FS_MAXFILEBLOCKS) {
            mutex_map[cur_inode_block].unlock();
            
            // deallocate the reserved data block
            used_blocks_lock.lock();
            used_blocks[new_inode_block] = false;
            used_blocks_lock.unlock();
            return -1;
        }
        
        // get disk block for new direntries block
        direntry_block = get_empty_block();
        if (direntry_block == -1) {          
            mutex_map[cur_inode_block].unlock();

            // deallocate the reserved data block
            used_blocks_lock.lock();
            used_blocks[new_inode_block] = false;
            used_blocks_lock.unlock();
            return -1;
        }     
               
        // assign file or dir name        
        strcpy(new_entries[0].name, pathname.substr(indices[indices.size() - 1] + 1).c_str()); 

        // add inode of new file to new direntries block
        new_entries[0].inode_block = new_inode_block;

        for(unsigned int i = 1; i < FS_DIRENTRIES; ++i) {
            new_entries[i].inode_block = 0;
        }
        cur_inode.blocks[cur_inode.size] = direntry_block;
        cur_inode.size++;

    }
    else {
        // update the empty_direntry to point to new file inode
        direntries[empty_direntry].inode_block = new_inode_block;
        strcpy(direntries[empty_direntry].name, pathname.substr(indices[indices.size() - 1] + 1).c_str());
    }

    // write inode of new file to disk
    disk_writeblock(new_inode_block, (void*)&new_inode);
    if (empty_direntry == -1) {
        // write new direntries block to disk
        disk_writeblock(direntry_block, (void*)&new_entries);
        // update inode block of parent
        disk_writeblock(cur_inode_block, (void*)&cur_inode);
    }
    else {
        // update direntries block
        disk_writeblock(direntry_block, (void*)&direntries);
    }
    
    // unlock all held locks
    mutex_map[cur_inode_block].unlock();

    // construct return message
    int return_msg_size = strlen(message[1]) + strlen(message[2]) + 2;
    return_msg = (char*) malloc(return_msg_size);
    construct_return_msg(return_msg, message[1], message[2]);
    return return_msg_size;
}

// handles the FS_DELETE request from client
int delete_handler(char* username, std::vector<char*> message, char* &return_msg) {
    // The session and the sequence passed in already succeed in parsing checking for stoul function
    // So it's safe to call stoul here directly and cast them to unsigned int    
    unsigned int session = (unsigned int)std::stoul(message[1]);
    unsigned int sequence = (unsigned int)std::stoul(message[2]);
    std::string pathname = message[3];
    
    // update sequence number
    session_lock.lock();
    session_map[session].second = sequence;
    session_lock.unlock();

    // check if the pathname is valid             
    if (!is_pathname_valid(pathname)) return -1;

    // finds all the indices of '/' in the pathname
    std::vector<int> indices;
    file_splitter(pathname, indices);

    fs_inode cur_inode;
    // start from the root     
    unsigned int cur_inode_block = 0;
    if(!traverse(cur_inode, cur_inode_block, pathname, true, username)) {
        return -1;
    }
    // cur_inode now contains inode of parent directory and it is write locked.


    // traversing to the inode of entity    
    fs_direntry direntries[FS_DIRENTRIES];

    bool all_empty = true;
    int direntry_index = -1;
    int direntry_block = -1;
    // loop through the blocks of the parent directory inode        
    for (unsigned int i = 0; i < cur_inode.size; ++i) {
        all_empty = true;
        // read in the data block            
        disk_readblock(cur_inode.blocks[i], (void*)&direntries);
        // loop through all direntries in data block
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            if (direntries[j].inode_block != 0 && std::string(direntries[j].name) == pathname.substr(indices.back()+1)){
                //found the entity to be deleted
                direntry_index = j;
                direntry_block = i;
            }
            else if (direntries[j].inode_block != 0) {
                // if there is another file in the same direntry block
                all_empty = false;
            }
        }
        // break the loop after finding the entity
        if(direntry_index != -1) break;      
    }

    // entity to be deleted not found
    if (direntry_index == -1 || direntry_block == -1) {
        mutex_map[cur_inode_block].unlock();
        return -1;
    }

    // inode for the entity to be deleted
    fs_inode delete_inode;
    int delete_inode_block = direntries[direntry_index].inode_block;

    mutex_map[delete_inode_block].lock();
    // read in the inode of the entity to be deleted
    disk_readblock(delete_inode_block, (void*)&delete_inode);
    
    if (strcmp(delete_inode.owner,username) != 0 || (delete_inode.size > 0 && delete_inode.type == 'd')) {
        //advanced features (deleting non-empty directories which we don't need to handle)
        mutex_map[cur_inode_block].unlock();
        mutex_map[delete_inode_block].unlock();
        return -1;
    }

    // if the entity to be deleted is a file, release all data blocks
    if (delete_inode.type == 'f') {
        used_blocks_lock.lock();
        for (unsigned int i = 0; i < delete_inode.size; ++i) {
            used_blocks[delete_inode.blocks[i]] = false;
        }
        used_blocks_lock.unlock();
    }
    
    // parent data block is not all empty        
    if (!all_empty) {
        direntries[direntry_index].inode_block = 0;
        // update direntries block on disk
        disk_writeblock(cur_inode.blocks[direntry_block], (void*)&direntries);
    }

    // release inode block of deleted entity
    used_blocks_lock.lock();
    used_blocks[delete_inode_block] = false;
    used_blocks_lock.unlock();
    
    // parent data block is all empty
    // need to release that data block    
    if(all_empty) {
        // release direntries block
        used_blocks_lock.lock();
        used_blocks[cur_inode.blocks[direntry_block]] = false;
        used_blocks_lock.unlock();

        // shift all data blocks of parent inode down
        for (unsigned int i = direntry_block; i < cur_inode.size-1; ++i) {
            cur_inode.blocks[i] = cur_inode.blocks[i+1];
        }
        --cur_inode.size;

        // update parent inode
        disk_writeblock(cur_inode_block, (void*)&cur_inode);
    }

    // unlock all held locks
    mutex_map[delete_inode_block].unlock();
    mutex_map[cur_inode_block].unlock();

    // construct return message
    int return_msg_size = strlen(message[1]) + strlen(message[2]) + 2;
    return_msg = (char*) malloc(return_msg_size);
    construct_return_msg(return_msg, message[1], message[2]);
    return return_msg_size;

}

// splits the pathname using '/' delimiter, and put all the indices of the '/' in the pathname
// to the vec variable.
void file_splitter(std::string path, std::vector<int> &indices) {
    size_t pos = path.find('/');
 
    // loop until end of pathname
    while( pos != std::string::npos) {
        // Add index to vector
        indices.push_back(pos);

        // Get the next '/' from the current position
        pos = path.find('/', pos + 1);
    }
}

// finds the first non-used disk block and returns it's number
int get_empty_block() {
    //block 0 is reserved for root
    unsigned int block = 1;
    used_blocks_lock.lock();
    //find the first unused block
    while (block < FS_DISKSIZE && used_blocks[block]) ++block;

    if (block == FS_DISKSIZE) {
        used_blocks_lock.unlock();
        return -1;
    }
    //change the unused block to be used
    used_blocks[block] = true;

    used_blocks_lock.unlock();
    return block;
}

// checks that the string parameter starts with a '/', does not end with '/', does not contain whitespace,
// and pathname < FS_MAXPATHNAME and that all filenames that are part of the pathname are < FS_MAXFILENAME.
bool is_pathname_valid(std::string pathname) {
    // check if pathname is empty, doesn't start with '/', ends with '/' or contains '\0'
    if (pathname.empty() || pathname.front() != '/' || pathname.back() == '/' || pathname.find('\0') != std::string::npos) 
        return false;
    
    // check that pathname does not contain whitespace
    for(char c : pathname) {
        if(isspace(c)) return false;
    }

    size_t start = pathname.find('/');
    // loop until end of pathname
    while( start != std::string::npos) {
        size_t end = pathname.find('/', start + 1);

        // get individual filename from pathname
        std::string filename;
        if(end == std::string::npos) {
            filename = pathname.substr(start+1);
        }
        else {
             filename = pathname.substr(start+1, end-start-1);
        }
        
        // check that filename is not empty and is less than FS_MAXFILENAME
        if (filename.empty() || filename.length() > FS_MAXFILENAME) return false;

        start = end;

    }

    return true;
}

// traverse until the parent directory, return while holding shared/exclusive lock for parent inode
// if parent_write_lock is true, then it will return with a write lock on cur_inode which is the parent
// directory of the last file in pathname. If parent_write_lock is false, it will return with a read lock
// on cur_inode.
bool traverse(fs_inode& cur_inode, unsigned int &cur_inode_block, std::string pathname, bool parent_write_lock, char* username) {
    // finds all the indices of '/' in the pathname
    std::vector<int> indices;
    file_splitter(pathname, indices);

    // start from the root     
    cur_inode_block = 0;

    unsigned int level = 1;
    
    // if the caller wants a write lock and the parent directory is the root write lock the root, else read lock
    if(parent_write_lock && level == indices.size()) {   
        mutex_map[cur_inode_block].lock();
    }
    else {
        mutex_map[cur_inode_block].lock_shared();
    }
    disk_readblock(cur_inode_block, (void*)&cur_inode);
    //traversing until the parent directory of entity
    while (level < indices.size()) {
        bool found = false;
        // loop through the current inode block
        for (unsigned int i = 0; i < cur_inode.size; ++i) {            
            fs_direntry direntries[FS_DIRENTRIES]; 
            // read in the data block                        
            disk_readblock(cur_inode.blocks[i], (void*)&direntries);
            // loop through each direntry in the data block
            for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
                 if (direntries[j].inode_block != 0 && std::string(direntries[j].name) == pathname.substr(indices[level - 1] + 1, indices[level] - (indices[level - 1] + 1))) {
                    //found the directory
                    if(parent_write_lock && level == indices.size() - 1) {
                        //require an exclusive lock on the parent directory of entity 
                        mutex_map[direntries[j].inode_block].lock();

                    }
                    else {
                        //if it is not the parent directory of entity or if an exclusive lock is not required
                        mutex_map[direntries[j].inode_block].lock_shared();
                    }
                    mutex_map[cur_inode_block].unlock_shared();
                    cur_inode_block = direntries[j].inode_block;
                    
                    disk_readblock(cur_inode_block, (void*)&cur_inode);        

                    if(strcmp(cur_inode.owner, username) != 0 || cur_inode.type != 'd') {
                        //if not owned by user or not a directory along the path
                        if (parent_write_lock && level == indices.size() - 1) mutex_map[cur_inode_block].unlock();
                        else mutex_map[cur_inode_block].unlock_shared();
                        return false;
                    }

                    ++level;
                    found = true;
                    break;
                }
            }

            // if the current directory along the path is found, break the data block loop            
            if (found) break;
        }
        // entity not found at a given step in the traversal
        if (!found) {
            mutex_map[cur_inode_block].unlock_shared();
            return false;
        }
    }
    
    return true;
}

// construct a string of session and sequence number
void construct_return_msg(char* &return_msg, const char* session, const char* sequence) {
    // copy the session number to the return msg    
    strcpy(return_msg, session);

    // add a space
    return_msg[strlen(session)] = ' ';

    // copy the sequence number to the return msg    
    strcpy(return_msg + strlen(session) + 1, sequence);

    // add the null terminated char at the end
    return_msg[strlen(session) + 1 + strlen(sequence)] = '\0';
}
