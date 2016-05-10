#pragma once

#include "stdafx.h"
#include<stdio.h>
#include<string>
#include<winsock2.h>
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
		int st;
		struct sockaddr_in Addr;

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




