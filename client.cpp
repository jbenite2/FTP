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
#include <signal.h>
#include <netdb.h>
#include <sys/time.h>

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

volatile sig_atomic_t serverDisconnected = 0;

void signalHandler(int signal) {
    if (signal == SIGPIPE) {
        serverDisconnected = 1;
    }
}

int main(int argc, char *argv[]) {
    // Set up signal handler for SIGPIPE (server disconnect)
    signal(SIGPIPE, signalHandler);

    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // check for the correct number of arguments
    if (argc != 4) {
        std::cerr << "ERROR: incorrect number of arguments\n";
        return 1;
    }

    // parse the arguments
    char *ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string filename = getLastToken(argv[3], '/');

    // Make sure it's a valid port
    if (port < 1024 || port > 65534) {
        std::cerr << "ERROR: invalid port number\n";
        return 1;
    }

    // Bind to a port and an address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // use IPv4 address
    serverAddr.sin_port = htons(port); // open a socket on the specified port
    if (strcmp(ip, "localhost") == 0) {
        serverAddr.sin_addr.s_addr = inet_addr("127.0.1.1"); // use localhost as the IP address of the server
    } else {
        serverAddr.sin_addr.s_addr = inet_addr(ip); // use the specified IP address of the server
    }
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;

	int state;
	std::string portStr = std::to_string(port);
	if((state = getaddrinfo(ip, portStr.c_str(), &hints, &result)) != 0) {
		std::cerr << "ERROR: getaddrinfo: " << gai_strerror(state) << std::endl;
		return 2;
	}


    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET,SO_SNDTIMEO, (const char*)&timeout, sizeof(struct timeval));

    /* if (select(sockfd + 1, NULL, &writefds, NULL, &timeout) <= 0 || !FD_ISSET(sockfd, &writefds)) { */
    /*     std::cerr << "ERROR: Connection attempt timed out\n"; */
    /*     close(sockfd); */
    /*     return 2; */
    /* } */

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("ERROR: connect");
        close(sockfd);
		freeaddrinfo(result);
        return 3;
    }
	freeaddrinfo(result);

    // Set up signal handler for SIGPIPE (server disconnect)
    signal(SIGPIPE, signalHandler);

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
        perror("getsockname");
        close(sockfd);
        return 4;
    }

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

    std::string filePath = argv[3];

    // Open the file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "ERROR: Unable to open file: " << filePath << std::endl;
        close(sockfd);
        return 5;
    }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == 0) {
        std::cerr << "ERROR: file is empty" << std::endl;
        close(sockfd);
        return 6;
    } else if (fileSize > 100 * 1024 * 1024) {
        std::cerr << "ERROR: file is too large" << std::endl;
        close(sockfd);
        return 7;
    }

    // Read the file into a buffer
    /* char *buffer = new char[fileSize]; */
    /* if (!file.read(buffer, fileSize)) { */
    /*     std::cerr << "Error reading file: " << filename << std::endl; */
    /*     delete[] buffer; */
    /*     close(sockfd); */
    /*     return 8; */
    /* } */

   /* /1* / Send the file *1/ */
    /* size_t bytesSent = send(sockfd, buffer, fileSize, 0); */
    /* if (bytesSent <= 0) { */
    /*     if (serverDisconnected) { */
    /*         std::cerr << "ERROR: Server disconnected\n"; */
			/* abort(); */
    /*     } else { */
    /*         perror("send"); */
    /*     } */
    /*     delete[] buffer; */
    /*     close(sockfd); */
    /*     return 9; */
    /* } */

	char buffer[1024];
	while (file) {
		file.read(buffer, sizeof(buffer));
		size_t bytesRead = file.gcount();

		// Send file contents to server
		ssize_t bytesSent = send(sockfd, buffer, bytesRead, 0);
		if (bytesSent <= 0) {
			if (bytesSent == 0) {
				std::cerr << "ERROR: Server disconnected during file transfer\n";
			} else {
				perror("ERROR: send");
			}
			//delete buffer
			close(sockfd);
			return 9;
		}
	}

	// Check for server disconnection after file transfer loop
	if (serverDisconnected) {
		std::cerr << "ERROR: Server disconnected\n";
		close(sockfd);
		return 10;
	}


    // Cleanup
    file.close();
    close(sockfd);

    std::cout << "File sent successfully" << std::endl;
    std::cout << "Connection closed" << std::endl;
    std::cout << "Client exits with code 0" << std::endl;
    return 0;
}


