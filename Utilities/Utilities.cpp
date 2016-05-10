// Utilities.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string>

#include "SerialPort.h"
//#include "UDPSocketServer.h"
//#include "TCPSocketServer.h"
//#include "UDPSocketClient.h"
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

// This main function is for testing all the socket and serial port classes

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("Usage: %s <command> [additional options]\n", argv[0]);
		printf("        Commands:\n");
		printf("            serial    <COM?> <baudrate>\n");
		printf("            udpserver <port>\n");
		printf("            tcpserver <port>\n");
		printf("            udpclient <port>\n");
		printf("            tcpclient <port>\n");
		return 1;
	}

	// Set interrupt handler
#if _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) interruptHandler, TRUE);
#else
	signal(SIGINT, interruptHandler);
#endif

	if (strncmp(argv[1], "serial", 6)) {
		if (argc < 4) {
			printf("Usage: %s %s <COM?> <baudrate>\n", argv[0], argv[1]);
			return 1;
		}
		SerialPort conn(argv[2], std::atoi(argv[3]));
		if (conn.isConnected()) {
			char * buffer = new char[4096];
			while (!quit) {
				conn.read(buffer, 4096);
				printf(buffer);
			}
		}
		else {
			printf("Failed to connect to %s\n", argv[2]);
		}
	}
	else if (strncmp(argv[1], "udpserver", 9)) {

	}
	else if (strncmp(argv[1], "tcpserver", 9)) {

	}
	else if (strncmp(argv[1], "udpclient", 9)) {

	}
	else if (strncmp(argv[1], "tcpclient", 9)) {

	}

    return 0;
}

