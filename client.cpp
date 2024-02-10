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



#include <iostream>
#include <sstream>

int main()
{
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

   // File name to send
    std::string filename = "Testing.txt";

    // Open the file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 4;
    }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file into a buffer
    char *buffer = new char[fileSize];
    if (!file.read(buffer, fileSize)) {
        std::cerr << "Error reading file: " << filename << std::endl;
        delete[] buffer;
        return 5;
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
