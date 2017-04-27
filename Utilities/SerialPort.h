#ifndef _SERIAL_PORT
#define _SERIAL_PORT

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <unistd.h>
#	include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <sys/types.h>
#   include <sys/stat.h>
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
		conn = CreateFileA(port.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0, NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
#else
		conn = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
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
			struct termios opts;
			tcgetattr(conn, &opts);
			// Set baudrate
            cfmakeraw(&opts);
			cfsetispeed(&opts, baudrate);
			cfsetospeed(&opts, baudrate);

            opts.c_cflag |= (CLOCAL | CREAD);
			opts.c_cflag &= ~PARENB;
			opts.c_cflag &= ~CSTOPB;
			opts.c_cflag &= ~CSIZE;
			opts.c_cflag |= CS8;
			opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
            opts.c_oflag &= ~OPOST;
            opts.c_cc[VMIN] = 0;
            opts.c_cc[VTIME] = 100;

			tcsetattr(conn, TCSANOW, &opts);
            int status;
            ioctl(conn, TIOCMGET, &status);
            status |= TIOCM_DTR;
            status |= TIOCM_RTS;
            ioctl(conn, TIOCMSET, &status);
            usleep(10000);

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
	int write(const char * buffer, unsigned int len) {
#ifdef _WIN32
		unsigned long sent;
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
