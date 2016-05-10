#pragma once

#include <stdio.h>
#include <string>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")




/**
* Refer to the following link for Microsoft's Server C++ code:
* https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx
*
*/

#define DEFAULT_BUFFER_SIZE 512
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKET_READ_TIMEOUT_SEC 1


class TCPSocketServer{
	
private:
		
		std::string IPADDRESS;
		int PORT;
		int socket;
		struct sockaddr_in addr;

public:
	/**
	 * Create a new TCP Socket Server
	 * 
	 */
	TCPSocketServer(std::string address, int port);
	
	~TCPSocketServer();

	int server();

	int send();

};




