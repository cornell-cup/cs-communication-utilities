#ifndef _TCP_SOCKET_CLIENT
#define _TCP_SOCKET_CLIENT

#ifdef _WIN32
#	include <WinSock2.h>
#	include <WS2tcpip.h>
#	include <Windows.h>
#else
#	include <arpa/inet.h>
#	include <string.h>
#	include <sys/socket.h>
#	include <unistd.h>
#	define Sleep(x) usleep(x*1000)
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
#endif

#define DEFAULT_BUFFER_SIZE 4096
#define SOCKET_READ_TIMEOUT_SEC 1
#ifndef INVALID_SOCKET
#	define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#	define SOCKET_ERROR (-1)
#endif

/**
* TCP socket client
*
* Refer to the following link for Microsoft's Server C++ code:
* https://msdn.microsoft.com/en-us/library/windows/desktop/ms737591(v=vs.85).aspx
*/
class TCPSocketClient {
private:
	/**
	* The address to connect to
	*/
	std::string address;

	/**
	* The port to connect to
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
	int connected;

public:
	/**
	* Initialize a new UDP socket server.
	*
	* @param address	The address to bind to
	* @param port		The port to listen on
	*/
	TCPSocketClient(std::string inAddress, int inPort) :
		address(inAddress), port(inPort), connected(0) {
		if ((socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
			printf("TCPSocketClient: Could not create socket.\n");
			return;
		}

		addr.sin_family = AF_INET;
		// TODO Support IPv6, use InetPton (inet_addr is deprecated)
		addr.sin_addr.s_addr = inet_addr(address.c_str());
		addr.sin_port = htons(port);

		if (connect(socketId, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
			int error = WSAGetLastError();
#else
			int error = errno;
#endif
			printf("socket failed with error: %d\n", error);
			printf("TCPSocketClient: Error connecting to the socket address.\n");
			return;
		}

		connected = true;
	};

	/**
	* Deconstructor
	*/
	~TCPSocketClient() {
		close();
	};

	/**
	* Whether or not the client is connected to the server.
	*
	* @return		Boolean whether or not the client is connected to the server
	*/
	int isConnected() {
		return connected;
	};

	/**
	* Read bytes of data from the port.
	* TODO Provide an asynchronous version of the function
	*
	* @param buffer	The buffer to read data to
	* @param maxlen	The maximum number of bytes to read
	* @return		The number of bytes read
	*/
	int read(char * outBuffer, unsigned int buffer_len) {
		unsigned int len;

		len = recv(socketId, outBuffer, buffer_len, 0);

		return len;
	};

	/**
	* Write bytes of data to the port.
	*
	* @param buffer	The buffer to write data from
	* @param len		The number of bytes to write
	* @return		Whether or not the write was successful
	*/
	int write(const char * buffer, unsigned int len) {
		if (send(socketId, buffer, len, 0) < 0) {
			return 0;
		}
		// TODO Handle error codes and reconnection

		return 1;
	};

	/**
	* Close the connection.
	*/
	void close() {
		if (connected) {
#ifdef _WIN32
			closesocket(socketId);
#else
			::close(socketId);
#endif
		}

		connected = false;
	};
};

#endif