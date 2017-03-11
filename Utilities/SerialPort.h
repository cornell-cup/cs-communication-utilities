#pragma once

#include <Windows.h>
#include <string>

/**
 * Connection to serial ports
 * Based on playground.arduino.cc/Interfacing/CPPWindows
 */
class SerialPort {
private:
	/**
	 * The COM port connected to
	 */
	std::string port;

	/**
	 * Current baudrate
	 */
	int baudrate;

	/**
	 * Whether or not the connection is up and alive
	 */
	int connected;

	/**
	 * The connection handle
	 */
	HANDLE conn;

	/**
	 * The connection status
	 */
	COMSTAT status;

	/**
	 * The connection error statuses
	 */
	unsigned long errors;

public:
	/**
	 * Initialize a new serial port connection.
	 * 
	 * @param port		The name of the serial port, e.g. "COM5"
	 * @param baudrate	The baudrate
	 */
	SerialPort(std::string inPort, int inBaudrate) :
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
	};

	/**
	 * Deconstructor
	 */
	~SerialPort() {
		if (connected) {
			CloseHandle(conn);
		}
	};

	/**
	 * Connect or reconnect to the serial port.
	 */
	void connect() {

	};

	/**
	 * Return whether or not the connection is established.
	 * 
	 * @return		Boolean whether or not the connection is alive
	 */
	int isConnected() {
		return connected;
	};

	/**
	 * Read bytes of data from the port.
	 * 
	 * @param buffer	The buffer to read data to
	 * @param maxlen	The maximum number of bytes to read
	 * @return		The number of bytes read
	 */
	int read(char * outBuffer, unsigned int maxlen) {
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
	};

	/**
	 * Write bytes of data to the port.
	 * 
	 * @param buffer	The buffer to write data from
	 * @param len		The number of bytes to write
	 * @return		Whether or not the write was successful
	 */
	int write(char * buffer, unsigned int len) {
		unsigned long sent;

		if (!WriteFile(conn, (void *)buffer, len, &sent, NULL)) {
			ClearCommError(conn, &errors, &status);

			return 0;
		}
		else {
			return 1;
		}
	};
};
