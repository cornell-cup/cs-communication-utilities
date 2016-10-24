#include "TCPSocketClient.h"

TCPSocketClient::TCPSocketClient(std::string inAddress, int inPort) :
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
		printf("socket failed with error: %ld\n", WSAGetLastError());
		printf("TCPSocketClient: Error connecting to the socket address.\n");
		return;
	}

	connected = true;
}

TCPSocketClient::~TCPSocketClient() {
	if (connected) {
		closesocket(socketId);
	}

	connected = false;
}

int TCPSocketClient::isConnected() {
	return connected;
}

int TCPSocketClient::read(char * outBuffer, unsigned int buffer_len) {
	unsigned int len;

	len = recv(socketId, outBuffer, buffer_len, NULL);

	return len;
}

int TCPSocketClient::write(const char * buffer, unsigned int len) {
	if (send(socketId, buffer, len, NULL) < 0) {
		return 0;
	}
	// TODO Handle error codes and reconnection

	return 1;
}

void TCPSocketClient::close() {
	if (connected) {
		closesocket(socketId);
	}

	connected = false;
}
