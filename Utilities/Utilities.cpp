// Utilities.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string>
#include <chrono>

#if _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#endif

#include "SerialPort.h"
#include "UDPSocketServer.h"
//#include "TCPSocketServer.h"
#include "UDPSocketClient.h"
//#include "TCPSocketClient.h"

#if _WIN32
#else
#include <signal.h>
#endif

// Exit
volatile int quit = 0;

#if _WIN32
int interruptHandler(unsigned int type) {
	switch (type) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		quit = 1;
		return TRUE;
	default:
		return FALSE;
	}
}
#else
void interruptHandler(int type) {
	quit = 1;
}
#endif

/**
 * Initialize WinSock
 */
void initWSA() {
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
}

// This main function is for testing all the socket and serial port classes

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("Usage: %s <command> [additional options]\n", argv[0]);
		printf("        Commands:\n");
		printf("            serial    <COM> <baudrate>\n");
		printf("            udpserver <address> <port>\n");
		printf("            tcpserver <address> <port>\n");
		printf("            udpclient <address> <port>\n");
		printf("            tcpclient <address> <port>\n");
		return 1;
	}

	// Set interrupt handler
#if _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) interruptHandler, TRUE);
#else
	signal(SIGINT, interruptHandler);
#endif

	initWSA();

	if (strncmp(argv[1], "serial", 6) == 0) {
		// Monitor a serial port
		if (argc < 4) {
			printf("Usage: %s %s <COM> <baudrate>\n", argv[0], argv[1]);
			return 1;
		}
		SerialPort conn(argv[2], std::atoi(argv[3]));
		if (conn.isConnected()) {
			printf("Opened %s\n", argv[2]);
			char * buffer = new char[4096];
			unsigned int len;
			memset(buffer, 0, 4096);
			while (!quit) {
				len = conn.read(buffer, 4096);
				printf(buffer);
				if (len > 0) {
					memset(buffer, 0, 4096);
				}
			}
		}
	}
	else if (strncmp(argv[1], "udpserver", 9) == 0) {
		// Echo server
		if (argc < 4) {
			printf("Usage: %s %s <address> <port>\n", argv[0], argv[1]);
			return 1;
		}
		UDPSocketServer conn(std::string(argv[2]), std::atoi(argv[3]));
		conn.server([](char * buffer, unsigned int buffer_len) {
			printf(buffer);
		});
		if (conn.isListening()) {
			while (!quit) {

			}
			conn.close();
		}
	}
	else if (strncmp(argv[1], "tcpserver", 9) == 0) {
		// Echo server with connections
	}
	else if (strncmp(argv[1], "udpclient", 9) == 0) {
		// Send a count every second
		if (argc < 4) {
			printf("Usage: %s %s <address> <port>\n", argv[0], argv[1]);
			return 1;
		}
		UDPSocketClient conn(std::string(argv[2]), std::atoi(argv[3]));
		if (conn.isConnected()) {
			std::chrono::time_point<std::chrono::system_clock> start, end;

			start = std::chrono::system_clock::now();
			char * buffer = new char[4096];
			memset(buffer, 0, 4096);
			int counter = 0;
			while (!quit) {
				printf("Sending %d\n", counter);
				std::string send = std::to_string(counter) + "\n";
				conn.write(send.c_str(), send.size());
				counter++;
				Sleep(1000);
			}
		}
	}
	else if (strncmp(argv[1], "tcpclient", 9) == 0) {
		// Send a count every second with a connection
	}

    return 0;
}

