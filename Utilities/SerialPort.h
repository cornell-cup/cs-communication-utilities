#ifndef _SERIAL_PORT
#define _SERIAL_PORT

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <unistd.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <termios.h>
#	define INVALID_HANDLE_VALUE (-1)
#endif

#include <string>

/**
 * Connection to serial ports
 * Based on playground.arduino.cc/Interfacing/CPPWindows
 */
class SerialPort {
private:
	/**
	 * The port connected to (//./COMn for Windows, /dev/ttyxxxx for Linux)
	 */
	std::string port;

	/**
	 * Connection baudrate
	 */
	int baudrate;

	/**
	 * Whether or not the connection is up and alive
	 */
	int connected;

	/**
	 * The connection handle
	 */
#ifdef _WIN32
	HANDLE conn;
#else
	int conn;
#endif

	/**
	 * The connection status
	 */
#ifdef _WIN32
	COMSTAT status;
#else
	int status;
#endif

	/**
	 * The connection error statuses
	 */
	unsigned long errors;

public:
	/**
	 * Initialize a new serial port connection.
	 *
	 * @param port		The name of the serial port, e.g. "//./COM5" (Windows) or "/dev/tty/ACM0" (Linux)
	 * @param baudrate	The baudrate, e.g. 9600 (Windows) or B9600 (Linux)
	 */
	SerialPort(std::string inPort, int inBaudrate) :
		port(inPort), baudrate(inBaudrate), connected(0) {
#ifdef _WIN32
		conn = CreateFile(std::wstring(port.begin(), port.end()).c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0, NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
#else
		conn = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
#endif

		if (conn == INVALID_HANDLE_VALUE) {
			printf("SerialPort: Error connecting to %s\n", port.c_str());
		}
		else {
#ifdef _WIN32
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
#else
			// https://chrisheydrick.com/2012/06/24/how-to-read-serial-data-from-an-arduino-in-linux-with-c-part-4/
			struct termios opts;
			tcgetattr(conn, &opts);
			// Set baudrate
			cfsetispeed(&opts, baudrate);
			cfsetospeed(&opts, baudrate);
			// 8 bits, no parity, no stop bits
			opts.c_cflag &= ~PARENB;
			opts.c_cflag &= ~CSTOPB;
			opts.c_cflag &= ~CSIZE;
			opts.c_cflag |= CS8;
			// Canonical mode
			opts.c_lflag |= ICANON;
			// Commit the serial port settings
			tcsetattr(conn, TCSANOW, &opts);
			connected = true;
#endif
		}
	};

	/**
	 * Deconstructor
	 */
	~SerialPort() {
		if (connected) {
#ifdef _WIN32
			CloseHandle(conn);
#else
			::close(conn);
#endif
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

#ifdef _WIN32
		unsigned int n;
		ClearCommError(conn, &errors, &status);

		if (status.cbInQue > 0) {
			if (status.cbInQue > maxlen) {
				n = maxlen;
			}
			else {
				n = status.cbInQue;
			}

			unsigned long len;
			if (ReadFile(conn, outBuffer, n, &len, NULL)) {
				return len;
			}
		}
		return 0;
#else
		int n = ::read(conn, outBuffer, maxlen);
		if (n == -1) {
			return -1;
		}

		return n;
#endif
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

#ifdef _WIN32
		if (!WriteFile(conn, (void *)buffer, len, &sent, NULL)) {
			ClearCommError(conn, &errors, &status);
#else
		if (::write(conn, (void *)buffer, len) == -1) {
#endif
			return 0;
		}
		else {
			return 1;
		}
	};
};

#endif