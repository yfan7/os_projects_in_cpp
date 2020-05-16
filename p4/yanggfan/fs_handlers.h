#include "fs_server.h"
#include <vector>
#include <string>

// handles the FS_SESSION request from client
extern int session_handler(char* username, std::vector<char*> message, char* &return_msg);

// handles the FS_CREATE request from client
extern int create_handler(char* username, std::vector<char*> message, char* &return_msg);

// handles the FS_WRITEBLOCK request from client
extern int writeblock_handler(char* username, std::vector<char*> message, char* &return_msg);

// handles the FS_READBLOCK request from client
extern int readblock_handler(char* username, std::vector<char*> message, char* &return_msg);

// handles the FS_DELETE request from client
extern int delete_handler(char* username, std::vector<char*> message, char* &return_msg);

// splits the pathname using '/' delimiter, and put all the indices of the '/' in the pathname
// to the vec variable.
extern void file_splitter(std::string path, std::vector<int> &indices);

// finds the first non-used disk block and returns it's number
extern int get_empty_block();

// checks that the string parameter starts with a '/', does not end with '/', does not contain whitespace,
// and pathname < FS_MAXPATHNAME and that all filenames that are part of the pathname are < FS_MAXFILENAME.
extern bool is_pathname_valid(std::string pathname);

// traverse until the parent directory, return while holding shared/exclusive lock for parent inode
// if parent_write_lock is true, then it will return with a write lock on cur_inode which is the parent
// directory of the last file in pathname. If parent_write_lock is false, it will return with a read lock
// on cur_inode.
extern bool traverse(fs_inode& cur_inode, unsigned int &cur_inode_block, std::string pathname, bool parent_write_lock, char* username);

// construct a string of session and sequence number
extern void construct_return_msg(char* &return_msg, const char* session, const char* sequence);