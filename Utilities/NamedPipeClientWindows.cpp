#include "NamedPipeClientWindows.h"


NamedPipeClientWindows::NamedPipeClientWindows(LPTSTR name):_pipeName(name)
{
	
}

NamedPipeClientWindows::~NamedPipeClientWindows()
{

}

void NamedPipeClientWindows::send(unsigned char * msg, size_t size)
{
	unsigned char * msg_out = new unsigned char[size + 4];
	msg_out[0] = size & 0xff >> 24;
	msg_out[1] = (size & 0xff) >> 16;
	msg_out[2] = (size & 0xff) >> 8;
	msg_out[3] = (size & 0xff);

	memcpy(msg_out + 4, msg, size);

	BOOL success = WriteFile(
		_pipe,                  // pipe handle 
		msg_out,             // message 
		size+4,              // message length 
		&_cbWritten,             // bytes written 
		NULL); // not overlapped 


	printf("Sent message of size: %d \n" , _cbWritten);

	if (!success)
	{
		sendError("Unabled to send message", -3);
	}
}

void NamedPipeClientWindows::close()
{
	_reading = false;
	CloseHandle(_pipe);
}

//This will stall
bool NamedPipeClientWindows::open()
{

	_pipe = CreateFile(
	_pipeName,   // pipe name 
	GENERIC_READ |  // read and write access 
	GENERIC_WRITE,
	0,              // no sharing 
	NULL,           // default security attributes
	OPEN_EXISTING,  // opens existing pipe 
	0,              // default attributes 
	NULL);          // no template file 

	// Break if the pipe handle is valid. 
	if (_pipe == INVALID_HANDLE_VALUE)
	{
		sendError( "Invalid Handle Value", 0);
	}
	else
	{
		printf("Pipe Connected: %s \n", _pipeName);
		if (setPipeState(PIPE_READMODE_BYTE))
		{
			startReadPolling();
			return true;
		}
		else
		{ 
			sendError("Failed to set PIPE_READMODE", -2);
			return false;
		}
	}
		sendError("Insert unhelpful error message here", GetLastError());

	// All pipe instances are busy, so wait for 20 seconds. 

	if (!WaitNamedPipe(_pipeName, 10000))
	{
		sendError("Could not open pipe: 10 second wait timed out.", -1);
	}

	return false;
}

void NamedPipeClientWindows::onRecieve(void(*returnFunct)(unsigned char *, size_t size))
{
	_onRecieveCallBack = returnFunct;
}

void NamedPipeClientWindows::errorCallback(void(*errorFunct)(std::string msg, unsigned int ec))
{
	_onErrorCallback = errorFunct;
}

bool NamedPipeClientWindows::setPipeState(DWORD state)
{
	return SetNamedPipeHandleState(
		_pipe,    // pipe handle 
		&state,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
}

void NamedPipeClientWindows::sendError(std::string msg, unsigned int ec)
{
	if (_onErrorCallback)
		_onErrorCallback(msg,ec);
}

void NamedPipeClientWindows::sendRecieve(unsigned char * data, size_t size)
{
	if (_onRecieveCallBack)
	{
		_onRecieveCallBack(data, size);
	}
	else
	{
		sendError("Recieve Callback not set. \n", -5);
	}
}

void NamedPipeClientWindows::startReadPolling()
{
	_reading = true;
	std::thread poll_thread(&NamedPipeClientWindows::pollServer, this);
	poll_thread.detach();
}

void NamedPipeClientWindows::pollServer()
{
	printf("Started polling %s server. \n", _pipeName);
		unsigned int size_of_data = 0;
		unsigned char* r2_data = nullptr;
		while (_reading) {
			BOOL success = FALSE;
			FlushFileBuffers(_pipeName);
			while (!success && _reading) {

			success = ReadFile(
				_pipe,    // pipe handle 
				&size_of_data,    // buffer to receive reply 
				4,  // size of buffer 
				&_cbRead,  // number of bytes read 
				NULL);    // not overlapped 

				size_of_data = _byteswap_ulong(size_of_data);
				
				r2_data = new unsigned char[size_of_data];
				success = ReadFile(
				_pipe,    // pipe handle 
				r2_data,    // buffer to receive reply 
				size_of_data,  // size of buffer 
				&_cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			if (!success && GetLastError() != ERROR_MORE_DATA)
			{
				sendError("Failed on read. Stopping Read.", GetLastError());
				_reading = false;
			}
		}
		if(success)
			sendRecieve(r2_data, size_of_data);
	}
}
