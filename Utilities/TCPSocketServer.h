#ifndef _TCP_SOCKET_SERVER
#define _TCP_SOCKET_SERVER

#ifdef _WIN32
#	include <WinSock2.h>
#	include <WS2tcpip.h>
#	include <Windows.h>
#else
#	include <errno.h>
#	include <arpa/inet.h>
#	include <string.h>
#	include <sys/socket.h>
#	include <unistd.h>
#	define Sleep(x) usleep(x*1000)
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
#endif

#include <thread>

#define DEFAULT_BUFFER_SIZE 4096
#define SOCKET_READ_TIMEOUT_SEC 1
#ifndef INVALID_SOCKET
#	define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#	define SOCKET_ERROR (-1)
#endif

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
	SOCKET socketId;

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
	void handle(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
		SOCKET clientId;

		while (!closeMessage && (clientId = accept(socketId, 0, 0))) {
			// TODO maintain a list of all clients
			std::thread(&TCPSocketServer::handleClient, this, clientId, handler).detach();
		}
	};

	/**
	* Continuously handle received data from a client (done in a new thread)
	*
	* @param handler	The function to handle data from clients
	*/
	void handleClient(SOCKET clientId, std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
		// Sent connect event to handler
		handler(clientId, CLIENT_CONNECT, 0, 0);

		char * buffer = new char[DEFAULT_BUFFER_SIZE];
		unsigned int buffer_len = DEFAULT_BUFFER_SIZE;
		unsigned int len = 0;

		memset(buffer, 0, buffer_len);
		while (!closeMessage) {
			len = recv(clientId, buffer, buffer_len, 0);
#ifdef _WIN32
			int error = WSAGetLastError();
			if (error != 0) {
#else
			int error = errno;
			if (error > 0) {
#endif
				// Send close event
				handler(clientId, CLIENT_DISCONNECT, 0, 0);
				break;
			}
			else if (len > 0) {
				handler(clientId, CLIENT_DATA, buffer, len);
				memset(buffer, 0, buffer_len);
			}
			Sleep(10);
		}

		closeMessage = 0;
		delete buffer;
	};

public:
	/**
	* Initialize a new UDP socket server.
	*
	* @param address	The address to bind to
	* @param port		The port to listen on
	*/
	TCPSocketServer(std::string inAddress, int inPort) :
		address(inAddress), port(inPort), listening(0), closeMessage(0) {
		if ((socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
			printf("TCPSocketServer: Could not create socket.\n");
			return;
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(address.c_str());
		addr.sin_port = htons(port);

		if (bind(socketId, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			printf("TCPSocketServer: Error binding to the socket address.\n");
			return;
		}

		if (listen(socketId, SOMAXCONN) == SOCKET_ERROR) {
			printf("TCPSocketServer: Error listening on socket address.\n");
			return;
		}
	};

	/**
	* Deconstructor
	*/
	~TCPSocketServer() {
		close();
	};

	/**
	* Whether or not the server is listening.
	*
	* @return		Boolean whether or not the server is listening
	*/
	int isListening() {
		return listening;
	};

	/**
	* Start the server and begin listening.
	*
	* @param handler	The function to handle data from clients
	*/
	void server(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
		std::thread(&TCPSocketServer::handle, this, handler).detach();
		listening = 1;
	};

	/**
	* Close the server and stop listening.
	*/
	void close() {
#ifdef _WIN32
		shutdown(socketId, SD_SEND);
		closesocket(socketId);
#else
		::close(socketId);
#endif
		closeMessage = 1;
	};

	// Different connection types
	const static unsigned int CLIENT_CONNECT = 1;
	const static unsigned int CLIENT_DATA = 0;
	const static unsigned int CLIENT_DISCONNECT = -1;
};

#endif