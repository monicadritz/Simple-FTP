/**********************************************************************************************************************************************************************************************
**Monica Pineda 
**CS 372
**June 4, 2017
**Program 2: This is a simple file transfer  program where the server side is implemented in
**C and the client side is implemented in python.
**ftserver.c: is the server side of this program. once started it will wait for a client
**to connect and then the conntected user can either request a list of what is in the current directory 
**or request a file via the file name to be transfered from the server.
**************************************************************************************************************************************************************************************************/
/***************************************************************************************
**   Title: Beej's Guide to Network Programming
**   Author: Brian “Beej Jorgensen” Hall
**   Date: June 8, 2016
**   Code version: <code version>
**   Availability: https://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf
**	 Used for Addrinfo and Sockets
***************************************************************************************/
/***************************************************************************************
**   Title: stack overflow how to list files in a directory in c 
**   Author: erip
**   Date: Dec 7, 2015
**   Code version: <code version>
** 	 http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program 
**	 Used for get_files
***************************************************************************************/
/***************************************************************************************
**   Title: stack overflow Send and Receive a file in socket programming in Linux with C/C++ (GCC/G++)
**   Author: azendale
**   Date: Mar 4, 2016
**   Code version: <code version>
** 	 https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
**	 Used for send_files
***************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

/**************************************************************************************************************************************************************
 ** struct addrinfo * create_address_info(char*)
 ** 
 ** creates a pointer to an address info linked list with port
 ** Args: two strings: the address and port number
 ** Returns: An address info linked list
 ************************************************************************************************************************************************************/
struct addrinfo * create_address_info(char * port){
	int status;
	struct addrinfo serverSide;
	struct addrinfo * res;
	
	memset(&serverSide, 0, sizeof serverSide);
	serverSide.ai_family = AF_UNSPEC;
	serverSide.ai_socktype = SOCK_STREAM;
	serverSide.ai_flags = AI_PASSIVE;

	if((status = getaddrinfo(NULL, port, &serverSide, &res)) != 0){
		fprintf(stderr,
				"getaddrinfo error: %s\nDid you enter the correct IP/Port?\n",
				gai_strerror(status));
		exit(1);
	}
	
	return res;
}

/**************************************************************************************************************************************************************
 ** struct addrinfo ** create_address_info(char**, char**)
 ** 
 ** Creates a pointer to an address info linked list with a address and port
 ** Args: two strings: the address and port number
 ** Returns: An address info linked list
 ************************************************************************************************************************************************************/

struct addrinfo * create_address_info_with_ip(char* input_addr, char* port)
{
	// create satus indicator and structures  for addrinfo 
	int status;
	struct addrinfo serverSide;
	struct addrinfo *res;

	// clear all fields of serverSide
	memset(&serverSide, 0, sizeof serverSide);
	serverSide.ai_family = AF_INET;
	serverSide.ai_socktype = SOCK_STREAM;
// if status is not 0 then an  error exists 
//print error otherwise return the addrinfo  
	if((status = getaddrinfo(input_addr, port, &serverSide, &res))!= 0)
	{
		fprintf(stderr,
				"getaddrinfo has an error: %s\nDid you enter correct IP/Port?\n",
					gai_strerror(status));
		exit(1);
	}
	return res;
}
/**************************************************************************************************************************************************************
 ** int create_socket(struct addrinfo **)
 ** 
 ** Creates a socket from an address info linked list
 ** Args: The address info linked list
 ** Returns: socket file descriptor
 ************************************************************************************************************************************************************/
int create_socket(struct addrinfo * res)
{
	int sockfd;
	// if the socketfd=-1, exit, otherwise return it
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		fprintf(stderr, "Error in creating socket\n");
		exit(1);
	}
	return sockfd;
}
/**************************************************************************************************************************************************************
 ** void connect_socket(int, struct addrinfo **)
 ** 
 ** Connects the socket to the address specified in the address info linked list
 ** Args: a socket file descriptor and an address info linked list
 ** Returns: nothing
 ************************************************************************************************************************************************************/
void connect_socket(int sockfd, struct addrinfo * res)
{
	int status;
	// if the status is -1, we were unable to connect if its not -1 connect the socket to the addrinfo information
	if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
	{
		fprintf(stderr, "Error in connecting socket\n");
		exit(1);
	}
}
/*******************************************************************************
 ** void bind_socket(int, struct addrinfo *)
 ** 
 ** Binds the socket to a port
 ** Args: socket file descriptor and an address info of the linked list
 ******************************************************************************/
void bind_socket(int sockfd, struct addrinfo * res){
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(sockfd);
		fprintf(stderr, "Error in binding socket\n");
		exit(1);
	}
}
/*******************************************************************************
 ** void listen_socket(int)
 ** 
 ** listens on the bound port
 ** Args: a socket file descriptor
 ******************************************************************************/
void listen_socket(int sockfd){
	if(listen(sockfd, 5) == -1){
		close(sockfd);
		fprintf(stderr, "Error in listening on socket\n");
		exit(1);
	}
}
/*******************************************************************************
 ** char ** create_string_array(int)
 ** 
 ** Creates a string array on the heap used for files 
 ** Args: integer number of files
 ******************************************************************************/
char ** create_string_array(int size){
	char ** stringArray = malloc(size*sizeof(char *));
	int i = 0;
	for(; i < size; i++){
		stringArray[i] = malloc(100*sizeof(char));
		memset(stringArray[i],0,sizeof(stringArray[i]));
	}
	return stringArray;
}

/*******************************************************************************
 ** void delete_string_array(char**, int)
 ** 
 ** deletes the string array off of the heap
 ** Args: string array and the number of files it had
 ******************************************************************************/
void delete_string_array(char ** stringArray, int size){
	int i = 0;
	for (; i < size; i++){
		free(stringArray[i]);
	}
	free(stringArray);
}


/*******************************************************************************
 ** int get_files(char **)
 ** 
 ** Counts how many files are in the directory and places them into the string
 ** array. Source is below
 ** Args: the string array 
 ******************************************************************************/
int get_files(char ** files)
{
	DIR * d= opendir(".");//open path
	struct dirent * dir;//the directory entries
	int i = 0;
	if (d)// while we are able to open the path
	{
		while ((dir = readdir(d)) != NULL)//while were able to read something from the directory
		{
			if (dir->d_type == DT_REG)//check if its really a file
			{
				strcpy(files[i], dir->d_name);//copy the files using string copy
				i++;
			}
		}
		closedir(d);
	}
	return i;
}

/*******************************************************************************
 * int file_exist(char **, int, char *)
 * 
 * Checks if file exists in file array
 * Args: the file array, the number of files, and the filename
 ******************************************************************************/
int file_exist(char ** files, int numFiles, char * fileName){
	int exist = 0;
	int i = 0;
	// loop through files to see if the filename we want is in there, set exist to one
	for (; i < numFiles; i++){
		if(strcmp(files[i], fileName) == 0){
			exist = 1;
		}
	}
	return exist;
}

/*******************************************************************************
 * void send_file(char *, char *, char *)
 * 
 * Sends the file using the socket to the given ip address and port number. 
 * Args: client ip address, the port number of the dataport and the filename
 ******************************************************************************/
void send_file(char * ip_address, char * port, char * fileName)
{
	// connect to the data socket
	// sleep makes sure that the client python has enough time to setup its data socket
	sleep(2);
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int dataSocket = create_socket(res);
	connect_socket(dataSocket, res);
	char buffer[1000];	// create a buffer 
	memset(buffer, 0, sizeof(buffer));// make sure the buffer is clean
	int fd = open(fileName, O_RDONLY);// open the file
	// Read data into buffer. There might not be enough to fill up buffer. So we store how many bytes were actually read in readBytes.
	while (1) 
	{
		int readBytes = read(fd, buffer, sizeof(buffer)-1);
		if (readBytes == 0) // We're done reading from the file
			break;

		if (readBytes < 0) {
			fprintf(stderr, "Error reading file\n");
			return;
		}
		//loop to write all of the data scince it can't be written in one call. This will return how many bytes were written
		// p keeps track of where we are in the buffer and readBytes is decrmented to keep track of what is left to write.
		void *p = buffer;
		while (readBytes > 0) {
			int writtenBytes = send(dataSocket, p, sizeof(buffer),0);
			if (writtenBytes < 0) {
				fprintf(stderr, "Error writing to socket\n");
				return;
			}
			readBytes -= writtenBytes;
			p += writtenBytes;
		}
		// clear out the buffer 
		memset(buffer, 0, sizeof(buffer));
	}
	// clear out the buffer again and send the done message
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "__done__");
	send(dataSocket, buffer, sizeof(buffer),0);
	// close the socket and free the address info
	close(dataSocket);
	freeaddrinfo(res);
}

/*******************************************************************************
 * void send_directory(char *, char *, char **, int)
 * 
 * Sends the contents of the directory to the client
 * Args: client ip address, data port number and the number of files in the directory
 ******************************************************************************/
void send_directory(char * ip_address, char * port, char ** files, int numFiles){
	// connect the data socket
	// sleep makes sure that the client python has a chance to setup its data socket
	sleep(2);
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int dataSocket = create_socket(res);
	connect_socket(dataSocket, res);
	// loop through filenames and send over to client
	int i = 0;
	for (; i < numFiles; i++){
		send(dataSocket, files[i], 100,0);
	}
	// send done message when done
	char * doneMessage = "done";
	send(dataSocket, doneMessage, strlen(doneMessage),0);
	// close socket and free address information
	close(dataSocket);
	freeaddrinfo(res);
}

/*******************************************************************************
 * void handle_request(int)
 * 
 * Handles the request from the client
 * Args: the newly created socket from the request
 ******************************************************************************/
void handle_request(int new_fd){
	// get the port number the client is expecting for a data connection
	char * okMessage = "ok";
	char * badMessage = "bad";
	char port[100];
	memset(port, 0, sizeof(port));
	recv(new_fd, port, sizeof(port)-1, 0);
	send(new_fd, okMessage, strlen(okMessage),0);
	// get the command from the client
	char command[100];
	memset(command,0,sizeof(command));
	recv(new_fd, command, sizeof(command)-1, 0);
	// send ok message 
	send(new_fd, okMessage, strlen(okMessage),0);
	// receive the ip of the client
	char ip_address[100];
	memset(ip_address,0,sizeof(ip_address));
	recv(new_fd, ip_address, sizeof(ip_address)-1,0);
	// print that we have a connection
	printf("Incoming connection from %s\n", ip_address);
	// if the command was -l:
	if(strcmp(command,"l") == 0){
		// send an ok message to let the client know that the command was ok
		send(new_fd, okMessage, strlen(okMessage),0);
		printf("File list requested on port %s\n", port);
		printf("Sending file list to %s on port %s\n", ip_address, port);
		// create a string array for the files in the dir
		char ** files = create_string_array(100);
		// get those files
		int numFiles = get_files(files);
		// send the contents of the file array to the client
		send_directory(ip_address, port, files, numFiles);
		// free up the space from the file array
		delete_string_array(files,100);
	}
	else if(strcmp(command, "g") == 0){
		// else if the command was -g:
		// send an ok message to let the client know that the command was ok
		send(new_fd, okMessage, strlen(okMessage),0);
		// get the file name from the client
		char fileName[100];
		memset(fileName, 0, sizeof(fileName));
		recv(new_fd, fileName, sizeof(fileName)-1,0);
		printf("File: %s requested on port %s\n", fileName, port);
		// create a list of files and check the file exists
		char ** files = create_string_array(100);
		int numFiles = get_files(files);
		int doesFileExists = file_exist(files, numFiles, fileName);
		if(doesFileExists){
			// if the file exists, let the user and client know
			printf("File found, sending %s to client\n", fileName);
			char * fileFound = "File found";
			send(new_fd, fileFound, strlen(fileFound),0);
			// create a new filename based on current location
			char newFileName[100];
			memset(newFileName,0,sizeof(newFileName));
			strcpy(newFileName, "./");
			char * end = newFileName + strlen(newFileName);
			end += sprintf(end, "%s", fileName);
			// send that file to the client
			send_file(ip_address, port, newFileName);
		}
		else{
			// else the file was not found, send that to the client
			printf("File not found, sending error message to client\n");
			char * fileNotFound = "File not found";
			send(new_fd, fileNotFound, 100, 0);
		}
		delete_string_array(files, 100);
	}
	else{// else the command wasn't found, send that to the client
		send(new_fd, badMessage, strlen(badMessage), 0);
		printf("Invalid command sent\n");
	}
	// print this to notify user that we have finished processing request
	printf("Continuing to wait for incoming connections\n");
}

/*******************************************************************************
 * void wait_for_connection
 * 
 * waits for a new connection to the server
 * Args: the file descriptor to wait on
 ******************************************************************************/
void wait_for_connection(int sockfd){
	// create a container for the connection
	struct sockaddr_storage their_addr;
	// create a size for the connection
    socklen_t addr_size;
	// create a new file descriptor for the connection
	int new_fd;
	// run forever
	while(1){
		// get the address size
		addr_size = sizeof(their_addr);
		// accept a new client
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &addr_size);
		// no new client keep waiting
		if(new_fd == -1){
			// no incoming connection, keep waiting
			continue;
		}
		// if we get a client, handle the request
		handle_request(new_fd);
		close(new_fd);
	}
}

/*******************************************************************************
 * int main(int, char*)
 * 
 * main method. sets up server socket, command line args and calls 
 * wait_for_connection
 * Args: the command lin args
 ******************************************************************************/
int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);
	}
	printf("Server open on port %s\n", argv[1]);
	struct addrinfo* res = create_address_info(argv[1]);
	int sockfd = create_socket(res);
	bind_socket(sockfd, res);
	listen_socket(sockfd);
	wait_for_connection(sockfd);
	freeaddrinfo(res);
}
