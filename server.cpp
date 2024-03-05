/* #include <sys/_select.h> */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <vector>

#include <iostream>
#include <sstream>
#include <string>
#include <csignal>

int terminateSignalReceived = 0;
int connectionId = 1;

void signalHandler(int signal) {
    if (signal == SIGQUIT || signal == SIGTERM) {
        terminateSignalReceived = 1;
    }
}

int main(int argc, char *argv[])
{
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // Bind to a port and an address
  if(argc!=3){
	  std::cerr<<"Error: incorrect number of arguments\n";
	  return 1;
  }

  int port = std::stoi(argv[1]);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); // the server will listen on port 4000
  addr.sin_addr.s_addr = inet_addr("127.0.1.1"); // open socket on localhost IP address for server
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  //Explicit error checking
  //Greater than 1023
  //Less than 65535
  if(port<1024 || port>65534){
		std::cerr<<"ERROR: invalid port number\n";
		return 1;
  }


  // bind to the address and port and gracefully handle any error
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  while (1) {
	  // accept a new connection from a client
	  struct sockaddr_in clientAddr;
	  socklen_t clientAddrSize = sizeof(clientAddr);
	  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
	  
	  /* select(clientSockfd+1, vfd_set *, fd_set *, fd_set *, struct timeval *); */

	  if (clientSockfd == -1) {
		perror("accept");
		return 4;
	  }

	  char ipstr[INET_ADDRSTRLEN] = {'\0'};
	  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	  std::cout << "Accept a connection from: " << ipstr << ":" <<
		ntohs(clientAddr.sin_port) << std::endl;


	  // Receive the file name from the client
		char filenameBuffer[100] = {0};
		ssize_t filenameSize = recv(clientSockfd, filenameBuffer, sizeof(filenameBuffer), 0);
		if (filenameSize == -1) {
			perror("recv filename");
			return 5;
		}
		std::string filename(filenameBuffer, filenameSize);
		std::cout << "Received file name: " << filename << std::endl;
		std::cout << "FILE NAME SIZE: " << filenameSize << std::endl;


		/* std::string directoryPath = std::string(argv[2]).substr(1); */
		std::string directoryPath = std::string(argv[2]);
		if (mkdir(directoryPath.c_str(), 0777) == -1 && errno != EEXIST) {
				// Directory doesn't exist, try to create it
				perror("Error creating directory");
				return 2;
		}
		// Open the output file
		/* std::string filePath = directoryPath+"/"+filename; */
		std::string filePath = directoryPath+"/"+ std::to_string(connectionId) +".file";
		connectionId++;
		
		std::fstream outputFile;    
	  outputFile.open(filePath, std::ios::out);
	  if (!outputFile.is_open()) {
		std::cerr << "Error: cannot open the file (" << filePath << ")" << std::endl;
		return 6;
	  }

	  std::vector<char> buffer(100*1024*1024, 0);
	  ssize_t valread = recv(clientSockfd, buffer.data(), buffer.size(), 0);
	  if (valread == -1) {
		perror("recv");
		return 7;
	  }
	  std::cout << "Bytes received: " << valread << '\n';
	  outputFile.write(buffer.data(), valread);

	 std::string message= "Hello from server!";
	  if(send(clientSockfd, message.c_str(), message.length(), 0)==-1){
		  perror("send");
		  return 8;
	  }
	 std::cout<<"Message received successfully\n";
	 shutdown(clientSockfd, SHUT_RDWR);
	 close(clientSockfd);
  }

  close(sockfd);

  return 0;
}
