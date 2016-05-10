#include "SerialPort.h"

SerialPort::SerialPort(std::string inPort, int inBaudrate) :
	port(inPort), baudrate(inBaudrate), connected(0) {
	conn = CreateFile(std::wstring(port.begin(), port.end()).c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (conn == INVALID_HANDLE_VALUE) {
		printf("SerialPort: Error connecting to %s\n", port.c_str());
	}
	else {
		DCB params = { 0 };

		if (!GetCommState(conn, &params)) {
			printf("SerialPort: Error getting serial parameters for %s\n", port.c_str());
		}
		else {
			// Set params
			params.BaudRate = baudrate;
			params.ByteSize = 8;
			params.StopBits = ONESTOPBIT;
			params.Parity = NOPARITY;
			// Uncomment below to reset Arduinos upon connection
			//params.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(conn, &params)) {
				printf("SerialPort: Error setting serial parameters for %s\n", port.c_str());
			}
			else {
				PurgeComm(conn, PURGE_RXCLEAR | PURGE_TXCLEAR);

				connected = true;
			}
		}
	}
}

SerialPort::~SerialPort() {
	if (connected) {
		CloseHandle(conn);
	}
}

void SerialPort::connect() {

}

int SerialPort::isConnected() {
	return connected;
}

int SerialPort::read(char * outBuffer, unsigned int maxlen) {
	unsigned long len;
	unsigned int n;

	ClearCommError(conn, &errors, &status);

	if (status.cbInQue > 0) {
		if (status.cbInQue > maxlen) {
			n = maxlen;
		}
		else {
			n = status.cbInQue;
		}

		if (ReadFile(conn, outBuffer, n, &len, NULL)) {
			return len;
		}
	}

	return 0;
}

int SerialPort::write(char * buffer, unsigned int len) {
	unsigned long sent;

	if (!WriteFile(conn, (void *)buffer, len, &sent, NULL)) {
		ClearCommError(conn, &errors, &status);

		return 0;
	}
	else {
		return 1;
	}
}
