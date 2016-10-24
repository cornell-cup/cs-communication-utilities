#pragma once

#include <stdio.h>
#include <string>
#include <functional>
#include <thread>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFFER_SIZE 4096
#define SOCKET_READ_TIMEOUT_SEC 1

/**
* Asynchronous TCP socket server
*
* Refer to the following link for Microsoft's Server C++ code:
* https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx
*/
class TCPSocketServer {
private:
	/**
	* The address to bind to
	*/
	std::string address;

	/**
	* The port to listen on
	*/
	int port;

	/**
	* Associated socket id
	*/
	int socketId;

	/**
	* Bound socket address
	*/
	SOCKADDR_IN addr;

	/**
	* Whether or not the server is listening
	*/
	int listening;

	/**
	* Internal message to close
	*/
	volatile int closeMessage;

	/**
	* Continuously handle new clients (done in a new thread)
	*
	* @param handler	The function to handle data from clients
	*					Note, the first parameter passed to the handler
	*					will be available only within the call to the
	*					handler, otherwise it may be deleted at any time.
	*/
	void handle(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler);

	/**
	* Continuously handle received data from a client (done in a new thread)
	*
	* @param handler	The function to handle data from clients
	*/
	void handleClient(SOCKET clientId, std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler);

public:
	/**
	* Initialize a new UDP socket server.
	*
	* @param address	The address to bind to
	* @param port		The port to listen on
	*/
	TCPSocketServer(std::string inAddress, int inPort);

	/**
	* Deconstructor
	*/
	~TCPSocketServer();

	/**
	* Whether or not the server is listening.
	*
	* @return		Boolean whether or not the server is listening
	*/
	int isListening();

	/**
	* Start the server and begin listening.
	*
	* @param handler	The function to handle data from clients
	*/
	void server(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler);

	/**
	* Close the server and stop listening.
	*/
	void close();

	// Different connection types
	const static unsigned int CLIENT_CONNECT = 1;
	const static unsigned int CLIENT_DATA = 0;
	const static unsigned int CLIENT_DISCONNECT = -1;
};
