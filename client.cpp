#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>



#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
	  // create a socket using TCP IP
	  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	  // check for right number of arguments
	  if(argc!=3){
		  std::cerr<<"Error: incorrect number of arguments\n";
		  return 1;
	  }

	  // parse the arguments
	  int port = std::stoi(argv[1]);
	  std::string filename = argv[2];

	  // Bind to a port and an address
	  struct sockaddr_in serverAddr;
	  serverAddr.sin_family = AF_INET; // use IPv4 address
	  serverAddr.sin_port = htons(40000);  // open a socket on port 4000 of the server
	  serverAddr.sin_addr.s_addr = inet_addr("127.0.1.1"); // use localhost as the IP address of the server to set up the socket
	  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	  // connect to the server
	  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("connect");
		return 2;
	  }

	  struct sockaddr_in clientAddr;
	  socklen_t clientAddrLen = sizeof(clientAddr);
	  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
		perror("getsockname");
		return 3;
	  }

	  char ipstr[INET_ADDRSTRLEN] = {'\0'};
	  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	  std::cout << "Set up a connection from: " << ipstr << ":" <<
	  ntohs(clientAddr.sin_port) << std::endl;


	  // Open the file
	  std::ifstream file(filename, std::ios::binary | std::ios::ate);
	  if (!file.is_open()) {
		std::cerr << "Unable to open file: " << filename << std::endl;
		return 4;
	  }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

	//Read teh file name into a buffer
	char *fileNameBuffer = new char[filename.length()];
	memcpy(fileNameBuffer, filename.c_str(), filename.length());

    // Read the file into a buffer
    char *buffer = new char[fileSize];
    if (!file.read(buffer, fileSize)) {
        std::cerr << "Error reading file: " << filename << std::endl;
        delete[] buffer;
        return 5;
    }

	// Send the file name
	size_t fileNameSize = filename.length();
	std::cout << "Sending file name size: " << fileNameSize << std::endl;
	std::cout << "Sending file name: " << fileNameBuffer << std::endl;
	if (send(sockfd, fileNameBuffer, fileNameSize, 0) == -1) {
		perror("send");
		delete[] buffer;
		return 6;
	}

    // Send the file
	size_t bytesSent = send(sockfd, buffer, fileSize, 0);
    if (bytesSent == -1) {
        perror("send");
        delete[] buffer;
        return 6;
    }

	std::cout << "Sent " << bytesSent << " bytes" << std::endl;

    // Cleanup
    delete[] buffer;
    file.close();
    close(sockfd);

    return 0;
}
