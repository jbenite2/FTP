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

int main(int argc, char *argv[])
{
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
  /* int port = 40000; */

  for(int i=0; i<argc; i++){
		std::cout<<argv[i]<<"\n";
  }

  std::cout<<"Port number: "<<port<<'\n';

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); // the server will listen on port 4000
  addr.sin_addr.s_addr = inet_addr("127.0.1.1"); // open socket on localhost IP address for server
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection from a client
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;

  // receive/send file from/to the client
  std::ofstream outputFile(argv[2]); 

  char buffer[10240] = {0};
  ssize_t valread = recv(clientSockfd, buffer, 10240, 0);
  if (valread == -1) {
	perror("recv");
	return 5;
  }
  std::cout << "Bytes received: " << valread << '\n';
  outputFile << "Received: " << buffer << '\n';

  std::string message= "Hello from server!";
  if(send(clientSockfd, message.c_str(), message.length(), 0)==-1){
	  perror("send");
	  return 6;
  }
  std::cout<<"Message received successfully\n";

  shutdown(clientSockfd, SHUT_RDWR);
  close(clientSockfd);

  return 0;
}
