#include "TCPSocketServer.h"

TCPSocketServer::TCPSocketServer(std::string inAddress, int inPort) :
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
}

TCPSocketServer::~TCPSocketServer() {
	shutdown(socketId, SD_SEND);
	closesocket(socketId);
}

int TCPSocketServer::isListening() {
	return listening;
}

void TCPSocketServer::server(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
	std::thread(&TCPSocketServer::handle, this, handler).detach();
	listening = 1;
}

void TCPSocketServer::handle(std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
	SOCKET clientId;

	while (!closeMessage && (clientId = accept(socketId, NULL, NULL))) {
		// TODO maintain a list of all clients
		std::thread(&TCPSocketServer::handleClient, this, clientId, handler).detach();
	}
}

void TCPSocketServer::handleClient(SOCKET clientId, std::function<void(SOCKET, unsigned int, char *, unsigned int)> handler) {
	// Sent connect event to handler
	handler(clientId, CLIENT_CONNECT, NULL, 0);

	char * buffer = new char[DEFAULT_BUFFER_SIZE];
	unsigned int buffer_len = DEFAULT_BUFFER_SIZE;
	unsigned int len = 0;

	memset(buffer, 0, buffer_len);
	while (!closeMessage) {
		len = recv(clientId, buffer, buffer_len, NULL);
		int error = WSAGetLastError();
		if (error != 0) {
			// Send close event
			handler(clientId, CLIENT_DISCONNECT, NULL, 0);
			break;
		}
		else if (len > 0) {
			handler(clientId, CLIENT_DATA, buffer, len);
			memset(buffer, 0, buffer_len);
		}
	}

	closeMessage = 0;
	delete buffer;
}

void TCPSocketServer::close() {
	closeMessage = 1;
}
