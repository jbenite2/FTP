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

std::string getLastToken(const std::string& str, char delimiter) {
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        if (!token.empty()) {
            if (iss.peek() != EOF) continue;
            return token;
        }
    }
    return "";
}

int main(int argc, char *argv[])
{
	  // create a socket using TCP IP
	  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	  // check for right number of arguments
	  if(argc!=4){
		  std::cerr<<"Error: incorrect number of arguments\n";
		  return 1;
	  }

	  // parse the arguments
	  char * ip = argv[1];
	  int port = std::stoi(argv[2]);
	  std::string filename = getLastToken(argv[3], '/');


	  //Print all arguments
	  for (int i = 0; i < argc; i++) {
		std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	  }

	  //Make sure it's a valid port
	  //Greater than 1023
	  //Less than 65535
	  if(port<1024 || port>65534){
		std::cerr<<"ERROR: invalid port number\n";
		return 1;
	  }

	  printf("IP address: %s\n", ip);
	  
	  //Make sure it's a valid IP address
	  //gethostname() function look up 
	  //use this to check if the IP address is valid
	  /* if(gethostname(ip, sizeof(ip)) != 0 && strcmp(ip, "localhost")!=0){ */
		  /* printf("IP address: %s\n", ip); */
		/* std::cerr<<"ERROR: invalid IP address\n"; */
		/* return 1; */
	  /* } */

	  // Bind to a port and an address
	  struct sockaddr_in serverAddr;
	  serverAddr.sin_family = AF_INET; // use IPv4 address
	  serverAddr.sin_port = htons(port);  // open a socket on port 4000 of the server
	  if(strcmp(ip, "localhost")==0){
		serverAddr.sin_addr.s_addr = inet_addr("127.0.1.1"); // use localhost as the IP address of the server to set up the socket
	  }else{
		serverAddr.sin_addr.s_addr = inet_addr(ip); // use the IP address of the server to set up the socket
	  }
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

	  std::string filePath = argv[3];

	  // Open the file
	  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	  if (!file.is_open()) {
		std::cerr << "Unable to open file: " << filePath << std::endl;
		return 4;
	  }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

	if(fileSize==0){
		std::cerr << "Error: file is empty" << std::endl;
		return 5;
	} else if(fileSize>100*1024*1024){
		std::cerr << "Error: file is too large" << std::endl;
		return 5;
	}

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
    if (bytesSent <= 0) {
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
