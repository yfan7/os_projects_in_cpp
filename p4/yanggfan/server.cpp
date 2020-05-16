#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "globals.h"
#include "socket_helper.h"
#include "fs_crypt.h"
#include <iostream>
#include "fs_handlers.h"
#include <limits.h>

const int MAX_HEADER_SIZE = 22;  // FS_MAXUSERNAME 2147483647<NULL>
const int MIN_REQUEST_SIZE = 15; // FS_SESSOIN 0 0<NULLL>
const int MAX_REQUEST_SIZE = 36 + FS_MAXPATHNAME + 12 + FS_BLOCKSIZE; //FS_WRITEBLOCK 4294967295 4294967295 FS_MAXPATHNAME 4294967295<NULL>FS_BLOCKSIZE

// validate and parse header. Data received passed in input and size of input
// header will contain the parsed parts of input
bool parse_header(char* input, std::vector<char*> &header, int size);
// validate and parse request. Data received passed in input and size of input
// messages will contain the parsed parts of input
bool parse_request(char* input, std::vector<char*> &messages, int size);
// check the format of request message by comparing the reconstructed message glued by space and
// terminated by null terminator against the original message
bool reconstruct_request(char* input, std::vector<char*> &messages, int num_parts);
// check the formats, ownership of session & sequence numbers 
bool session_check(char* username, char* session_char, char* sequence_char);

// allocates size_in number of c-strings of length width_in and stores them in input_in
// deallocates all of the c-strings during destructor
class RAII_allocate_delete {
	public:
	RAII_allocate_delete(std::vector<char*> &input_in, int size_in, int width_in) : input{input_in}, size{size_in}, width{width_in} {		
		// set size of vector
		input.resize(size);
		for (int i = 0; i < size; i++ ) {
			input[i] = (char*) malloc(width);
		}
	}

	~RAII_allocate_delete() {
		// free all c-strings
		for(int i = 0; i < size; ++i) {
			free(input[i]);
		}
	}

	private:
		std::vector<char*> &input;
		int size;
		int width;
};

// calls close with parameter connectionfd_in during destructor
class RAII_close_connection {
	public:
	RAII_close_connection(int connectionfd_in) : connectionfd{connectionfd_in} {}

	~RAII_close_connection() {
		// close
		close(connectionfd);
	}

	private:
		int connectionfd;
};

// initializes the file system
void fs_init(int inode_block){
	// mark current inode block as useed
	used_blocks_lock.lock();
	used_blocks[inode_block] = true;
	used_blocks_lock.unlock();
	
	fs_inode inode;
	disk_readblock(inode_block, (void*)&inode);
	// mark all data blocks as used
	for (unsigned int i = 0; i < inode.size; ++i) {
		used_blocks_lock.lock();
		used_blocks[inode.blocks[i]] = true;
		used_blocks_lock.unlock();
	}
	if (inode.type == 'd') {
		// recursively call this function on entities contained in this directory
		for (unsigned int i = 0; i < inode.size; ++i) {
			fs_direntry direnties[FS_DIRENTRIES];
		
			disk_readblock(inode.blocks[i], (void*)&direnties);

			for (unsigned int j = 0; j < FS_DIRENTRIES; ++j) {
				if (direnties[j].inode_block != 0)
					fs_init(direnties[j].inode_block);
			}
		}
	}
}

/**
 * Receives a null-terminated string message from the client, handle the requests 
 * then sends the return message back to the client upon success. Close connection
 * upon failure.
 * 
 * Parameters:
 * 		connectionfd: 	File descriptor for a socket connection (e.g. the one
 *				returned by accept())
 * Returns:
 *		0 on success, -1 on failure.
 */
int handle_connection(int connectionfd) {

	// initialize class to call close(connectionfd) during it's destructor
	RAII_close_connection raii_close(connectionfd);

	// (1) Receive header from client.
	char header_recv[MAX_HEADER_SIZE];

	for (int i = 0; i < MAX_HEADER_SIZE; i++) {
		// Receive exactly one byte
		int rval = recv(connectionfd, header_recv + i, 1, MSG_WAITALL);
		if (rval == -1) {
			return -1;
		}
		// Stop if we received a null character
		if (header_recv[i] == '\0') {
			break;
		}
		// return if header is longer than MAX_HEADER_SIZE
		if (i == MAX_HEADER_SIZE - 1) {
			return -1;
		}
	}

	// allocate 2 c-strings for the 2 parts of the header
	std::vector<char*> header;
	RAII_allocate_delete header_raii(header, 2, MAX_HEADER_SIZE);

	// validate and parse header
	if (!parse_header(header_recv, header, strlen(header_recv))) {
		return -1;
	}
	int request_size = std::stoi(header[1]);
	// if request is longer than the longest possible encrypted request or shorter
	// than the shortest possible encrypted request return
	if(request_size < MIN_REQUEST_SIZE || request_size > (2 * MAX_REQUEST_SIZE) + 64) {
		return -1;
	}

	// check if username is longer than FS_MAXUSERNAME
	if(strlen(header[0]) > FS_MAXUSERNAME) {
		return -1;
	}
	
	// check if username exists in the user_password map
	auto password_it = user_password.find(std::string(header[0]));
	if (password_it == user_password.end()) {
		return -1;
	}
	std::string password = password_it->second;

	// receive all bytes of request
	char request_recv[request_size];
	int rval = recv(connectionfd, request_recv, request_size, MSG_WAITALL);
	if (rval == -1) {
		return -1;
	}

	// attach a null terminator to prevent sscanf from reading beyond allocated memory
	char decrypt_request[request_size + 1];
	decrypt_request[request_size] = '\0';

	// decrypte request and ceck decrypted message size
	int decrypt_size = fs_decrypt(password.c_str(), request_recv, request_size, (void*)decrypt_request);
	if (decrypt_size < MIN_REQUEST_SIZE || decrypt_size > MAX_REQUEST_SIZE) {
		return -1;
	}

	// allocate 6 c-strings for the 6 parts of the header
	std::vector<char*> request;
	RAII_allocate_delete request_raii(request, 6, decrypt_size);

	// validate and parse request
	if(!parse_request(decrypt_request, request, decrypt_size)) {
		return -1;
	}
	// check the formats and onwership of session & sequence when it is not a session request
	if(request[0][3] != 'S' && !session_check(header[0], request[1], request[2])) {
		return -1;
	}

	int return_size = -1;
	char* return_msg = nullptr;

	// switch on 4th character in request[0] which is the request type
	switch(request[0][3]) {
		case 'S': // FS_SESSION
			return_size = session_handler(header[0], request, return_msg);
			break;
		case 'R': // FS_READBLOCK
			return_size = readblock_handler(header[0], request, return_msg);
			break;
		case 'W': // FS_WRITEBLOCK
			return_size = writeblock_handler(header[0], request, return_msg);
			break;
		case 'C': // FS_CREATE
			return_size = create_handler(header[0], request, return_msg);
			break;
		case 'D': // FS_DELETE
			return_size = delete_handler(header[0], request, return_msg);
			break;
	}
	
	// if return_size is -1 that means that the handlers returned with error
	// and we would not send a response message to the client
	if(return_size != -1) {
		// encrypt response message
		char encrypted_msg[2 * return_size + 64];
		int encrypt_size = fs_encrypt(password.c_str(), return_msg, return_size, encrypted_msg);
		
		std::string str_encrypt_size = std::to_string(encrypt_size);
		// send clear text header to client
		if (send(connectionfd, str_encrypt_size.c_str(), str_encrypt_size.length()+1, MSG_NOSIGNAL) == -1) {
			free(return_msg);
			return -1;
		}

		// send encrypted response message to client
		if (send(connectionfd, encrypted_msg, encrypt_size, MSG_NOSIGNAL) == -1) {
			free(return_msg);
			return -1;
		}
		
		free(return_msg);

	}

	return 0;


}

/**
 * Endlessly runs a server that listens for connections and spawn
 * one thread for each client request.
 *
 * Parameters:
 *		port: 		The port on which to listen for incoming connections.
 *		queue_size: 	Size of the listen() queue
 * Returns:
 *		-1 on failure, does not return on success.
 */
int run_server(int port, int queue_size) {

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		return -1;
	}

	// (2) Set the "reuse port" socket option
	int yesval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
		return -1;
	}

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	if (make_server_sockaddr(&addr, port) == -1) {
		return -1;
	}
	if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
		return -1;
	}

	// (3b) Detect which port was chosen
	port = get_port_number(sockfd);

	cout_lock.lock();
	std::cout << "\n@@@ port " << port << std::endl;
	cout_lock.unlock();

	// (4) Begin listening for incoming connections.
	if (listen(sockfd, queue_size) == -1) return -1;

	// (5) Serve incoming connections by spawning one thread for each request.
	while (true) {
		int connectionfd = accept(sockfd, 0, 0);
		if (connectionfd == -1) {
			return -1;
		}

		std::thread(handle_connection, connectionfd).detach();
		
	}
}

// validate and parse header. Data received passed in input and size of input
// header will contain the parsed parts of input
bool parse_header(char* input, std::vector<char*> &header, int size){
	//check if there are at least two strings
	if (sscanf(input, "%s %s", header[0], header[1]) < 2) {
		return false;

	}
	int msg_len;
	try {
		//index to the end of the number + 1
	 	size_t temp;
		msg_len = std::stoi(header[1], &temp, 10);
		//check if the recv size is followed by non-digit characters or if the size is negative
		if (header[1][temp] != '\0' || msg_len < 0) return false;
	}
	catch(...) {
		return false;
	}
	// check if the reconstructed header using the 2 parsed parts is
	// the same as the original header 			
	if(!reconstruct_request(input, header, 2)) return false;
	
	return true;
}

// validate and parse request. Data received passed in input and size of input
// header will contain the parsed parts of input
bool parse_request(char* input, std::vector<char*> &request, int size){

	// error checking to prevent segfault
	if (size < 4) return false;

	// switch on 4th character of request type
	switch (input[3])
	{
		case 'S': // FS_SESSION
		{
			// parse 3 parts of FS_SESSION request
			if (sscanf(input,"%s %s %s", request[0], request[1], request[2]) < 3) return false;
			// confirm that 1st part of parsing is FS_SESSION
			if (strcmp(request[0], "FS_SESSION") != 0) return false;
			// confirm that reconstructed message is equal to the received request message
			if(!reconstruct_request(input, request, 3)) return false;
		
			break;
		}
		case 'R': // FS_READBLOCK
		{
			// parse 5 parts of FS_READBLOCK request
			if (sscanf(input,"%s %s %s %s %s", request[0], request[1], 
			   request[2], request[3], request[4]) < 5) return false;
			// confirm that 1st part of parsing is FS_READBLOCK
			if (strcmp(request[0], "FS_READBLOCK") != 0) return false;
			// confirm that reconstructed message is equal to the received request message
			if(!reconstruct_request(input, request, 5)) return false;
			// error handling on block number
			try {
				if (std::stoi(request[4]) >= (int)FS_MAXFILEBLOCKS || std::stoi(request[4]) < 0 || *request[4] == '-') return false;
			}
			catch(...) {
				return false;
			}
			break;
		}
		case 'W': // FS_WRITEBLOCK
		{
			// parse 5 parts of FS_WRITEBLOCK request
			if (sscanf(input,"%s %s %s %s %s", request[0], request[1], 
			   request[2], request[3], request[4]) < 5) return false;
			// confirm that 1st part of parsing is FS_WRITEBLOCK
			if (strcmp(request[0], "FS_WRITEBLOCK") != 0) return false;
			// confirm that reconstructed message is equal to the received request message
			if (!reconstruct_request(input, request, 5)) return false;
			// error handling on block number
			try {
				if (std::stoi(request[4]) >= (int)FS_MAXFILEBLOCKS || std::stoi(request[4]) < 0 || *request[4] == '-') return false;
			}
			catch(...) {
				return false;
			}
    		if (strlen(request[3]) > FS_MAXPATHNAME) return false;

			// confirm that size of data to be written is == FS_BLOCKSIZE
			int constructed_size = 0;
			for (int i = 0; i < 5; i++) constructed_size += strlen(request[i]) + 1;
			if(size - constructed_size != FS_BLOCKSIZE) return false;
			memcpy(request[5], input + constructed_size, FS_BLOCKSIZE);
			break;
		}
		case 'C': // FS_CREATE
		{
			// parse 5 parts of FS_CREATE request
			if (sscanf(input,"%s %s %s %s %s", request[0], request[1], 
			   request[2], request[3], request[4]) < 5) return false;
			// confirm that 1st part of parsing is FS_CREATE
			if (strcmp(request[0], "FS_CREATE") != 0) return false;
			// confirm that reconstructed message is equal to the received request message
			if(!reconstruct_request(input, request, 5)) return false;
			if (strlen(request[3]) > FS_MAXPATHNAME) return false;
			// confirm that type is file or directory
			if (strcmp(request[4], "d") != 0 && strcmp(request[4], "f") != 0) return false;
			break;
		}
		case 'D': // FS_DELETE
		{
			// parse 5 parts of FS_DELETE request
			if (sscanf(input,"%s %s %s %s", request[0], request[1], 
			   request[2], request[3]) < 4) return false;
			// confirm that 1st part of parsing is FS_DELETE
			if (strcmp(request[0], "FS_DELETE") != 0) return false;
			// confirm that reconstructed message is equal to the received request message
			if(!reconstruct_request(input, request, 4)) return false;
			if (strlen(request[3]) > FS_MAXPATHNAME) return false;
			break;
		}
		default:
			return false;
			break;
	}	

	return true;

}

// check the format of request message by comparing the reconstructed message glued by space and
// terminated by null terminator against the original
bool reconstruct_request(char* input, std::vector<char*> &messages, int num_parts) {
	int size = 0;	
	// comparing all parts of the string with attached space against the original request except for the last part
	for(int i = 0; i < num_parts - 1; ++i) {
		if (strncmp(input+size, messages[i], strlen(messages[i])) != 0) return false;
		size += strlen(messages[i]);
		if (input[size] != ' ') return false;
		++size;
	}
	// compare the last part of the request (not counting block to write) 
	// attached with a null terminator against the original request
	if (strncmp(input+size, messages[num_parts - 1], strlen(messages[num_parts - 1])) != 0) return false;
	size += strlen(messages[num_parts - 1]);
	if (input[size] != '\0') return false;

	return true;
}

// check the formats, ownership of session & sequence numbers 
bool session_check(char* username, char* session_char, char* sequence_char) {
    bool return_val = false;

    unsigned int session, sequence;
	// used for error checking before casting
    unsigned long temp_session, temp_sequence;

	// error checking on session and sequence number
    try
    {
        size_t temp_session_size, temp_sequence_size;
        temp_session = std::stoul(session_char, &temp_session_size, 10);
        temp_sequence = std::stoul(sequence_char, &temp_sequence_size, 10);
        if (session_char[temp_session_size] != '\0' || sequence_char[temp_sequence_size] != '\0'|| *session_char == '-' || *sequence_char == '-') {
			
        	return false;
		}
    }
    catch(...)
    {
        return false;
    }
	// check if the session and sequence numbers overflow
    if(temp_session > UINT_MAX || temp_sequence > UINT_MAX) return false;

	// passed error checking so we can cast to unsigned int
    session = (unsigned int)temp_session;
    sequence = (unsigned int)temp_sequence;

	// confirm that session number is the one that was returned by a previous call to FS_SESSION
	// and that the username of this session number == username of current request
    session_lock.lock();
    auto it = session_map.find(session);
    if(it != session_map.end() && it->second.first == std::string(username) && sequence > it->second.second) {
        return_val = true;
    }
    session_lock.unlock();

    return return_val;
}

int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc > 2) {
		printf("Usage: ./server (port_num)\n");
		return 1;
	}
	
	int port = 0;
	if (argc == 2)
		port = atoi(argv[1]);


	// read in all username and password pairs
	std::string username;
	std::string password;
	while (std::cin >> username >> password) {
		user_password[username] = password;
	}

	// initialize the file system
	fs_init(0);

	// start server
	if (run_server(port, 10) == -1) {
		return 1;
	}

	return 0;
}
