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
	SerialPort(std::string inPort, int inBaudrate);

	/**
	 * Deconstructor
	 */
	~SerialPort();

	/**
	 * Connect or reconnect to the serial port.
	 */
	void connect();

	/**
	 * Return whether or not the connection is established.
	 * 
	 * @return		Boolean whether or not the connection is alive
	 */
	int isConnected();

	/**
	 * Read bytes of data from the port.
	 * 
	 * @param buffer	The buffer to read data to
	 * @param maxlen	The maximum number of bytes to read
	 * @return		The number of bytes read
	 */
	int read(char * outBuffer, unsigned int maxlen);

	/**
	 * Write bytes of data to the port.
	 * 
	 * @param buffer	The buffer to write data from
	 * @param len		The number of bytes to write
	 * @return		Whether or not the write was successful
	 */
	int write(char * buffer, unsigned int len);

	/**
	 * Reads lines of data from the port.
	 */
};
