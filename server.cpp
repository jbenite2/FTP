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
int connectionId = 1;

void signalHandler(int signal) {
    if (signal == SIGQUIT || signal == SIGTERM) {
        terminateSignalReceived = 1;
    }
}

int main(int argc, char *argv[]) {
    // Server handles SIGTERM / SIGQUIT signals
    signal(SIGQUIT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Allow others to reuse the address
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    // Bind to a port and an address
    if (argc != 3) {
        std::cerr << "Error: incorrect number of arguments\n";
        return 1;
    }

    int port = std::stoi(argv[1]);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if (port < 1024 || port > 65534) {
        std::cerr << "ERROR: invalid port number\n";
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 2;
    }

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return 3;
    }

    while (1) {

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
   
		if (FD_ISSET(sockfd, &readfds)) {
            // Accept a new connection from a client
            struct sockaddr_in clientAddr;
            socklen_t clientAddrSize = sizeof(clientAddr);
            int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
            std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

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

            std::vector<char> buffer(1024);
            ssize_t bytesRead;

			struct timeval timeout;
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;

			setsockopt(clientSockfd, SOL_SOCKET,SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

            while ((bytesRead = recv(clientSockfd, buffer.data(), buffer.size(), 0)) > 0) {
                if (bytesRead <= 0) {
					/* if (errno == EAGAIN || errno == EWOULDBLOCK) { */
					std::cerr << "ERROR: Timeout occurred. No data recieved." << std::endl; 
					outputFile.seekp(0) ;
					outputFile.clear();
					outputFile<<"ERROR";
					/* } */
				} else {
					outputFile.write(buffer.data(), bytesRead);
				}
            }

            std::cout << "Message received successfully\n";
            shutdown(clientSockfd, SHUT_RDWR);
            close(clientSockfd);
        }
    }

    close(sockfd);

    return 0;
}

