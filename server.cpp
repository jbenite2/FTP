#include <iostream>
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
#include <signal.h>

int terminateSignalReceived = 0;

void signalHandler(int signal) {
    if (signal == SIGQUIT || signal == SIGTERM) {
        terminateSignalReceived = 1;
    }
}

int main(int argc, char *argv[]) {
	
	// Incorrect number of arguments
    if (argc != 3) {
        std::cerr << "ERROR: incorrect number of arguments\n";
        return 3;
    }

    // Server handles SIGTERM / SIGQUIT signals
    signal(SIGQUIT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		std::cerr << "ERROR: " << strerror(errno) << std::endl;
		return 1;
	}

    // Allow others to reuse the address
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("ERROR: setsockopt");
        return 2;
    }

	// Initialize the port
    int port = std::stoi(argv[1]);
    if (port < 1024 || port > 65534) {
        std::cerr << "ERROR: invalid port number\n";
        return 1;
    }

	// Initialize the address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));


    // Bind to a port and an address
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 2;
    }

	// Listen for incoming connections
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return 3;
    }

	// Connection Counter
	int connectionId = 1;

    while (1) {
		// Accept a new connection from a client
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if(clientSockfd == -1) {
			if (errno == EINTR) {
				break;
			}
			std::cerr << "ERROR: " << strerror(errno) << std::endl;
			continue;
		}

		std::string directoryPath = std::string(argv[2]);
		int result = mkdir(directoryPath.c_str(), 0777);
		if (result == -1 && errno != EEXIST) {
			std::cerr << "Error creating directory: " << strerror(errno) << std::endl;
		}

		std::string filePath = directoryPath + "/" + std::to_string(connectionId) + ".file";
		connectionId++;

		std::fstream outputFile;
		outputFile.open(filePath, std::ios::out);
		if (!outputFile.is_open()) {
			std::cerr << "Error: cannot open the file (" << filePath << ")" << std::endl;
			return 6;
		}

		// Initialize buffer to read data from the client
		char buffer[1024];
		ssize_t bytesRead;

		// Set timeout for the socket
		struct timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		setsockopt(clientSockfd, SOL_SOCKET,SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		while (true) {
			bytesRead = recv(clientSockfd, buffer, sizeof(buffer), 0);
			if (bytesRead < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					std::cerr << "ERROR: Timeout occurred. " << std::endl; 
					outputFile.seekp(0) ;
					outputFile.clear();
					outputFile<<"ERROR";
					break;
				} else {
					std::cerr << "ERROR: " << strerror(errno) << std::endl;
					break;
				}
			} else if (bytesRead == 0) {
				break;
			} else {
				outputFile.write(buffer, bytesRead);
			}
		}

		outputFile.close();
		shutdown(clientSockfd, SHUT_RDWR);
		close(clientSockfd);
    }

    close(sockfd);

    return 0;
}

