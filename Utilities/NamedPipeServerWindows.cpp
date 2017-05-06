#include "stdafx.h"
#include "NamedPipeServerWindows.h"


NamedPipeServerWindows::NamedPipeServerWindows(LPTSTR pipe_name): _pipe_name(pipe_name)
{
	
}

void NamedPipeServerWindows::setOnRequestCallback(size_t(*onRequest)(unsigned char * data_in, unsigned char * data_out, size_t size_of_data_in))
{
	_onRequest = onRequest;
}

void NamedPipeServerWindows::setOnErrorCallback(void(*onError)(std::string msg, unsigned int ec))
{
	_onError = onError;
}


NamedPipeServerWindows::~NamedPipeServerWindows()
{

}

void NamedPipeServerWindows::writeToClient(LPPIPEINST client)
{
	bool success = WriteFile(
		client->hPipeInst,
		client->chReply,
		client->cbToWrite,
		&_cbRet,
		&client->oOverlap);

	// The write operation completed successfully. 

	if (success && _cbRet == client->cbToWrite)
	{
		client->fPendingIO = FALSE;
		client->dwState = READING_STATE;
		return;
	}

	// The write operation is still pending. 
	if (!success && (GetLastError() == ERROR_IO_PENDING))
	{
		client->fPendingIO = TRUE;
		return;
	}

	DisconnectAndReconnect(client);
}

void NamedPipeServerWindows::stop()
{
	_running = false;
}

bool NamedPipeServerWindows::isRunning()
{
	return _running;
}

// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 

VOID NamedPipeServerWindows::DisconnectAndReconnect(LPPIPEINST client)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(client->hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	client->fPendingIO = ConnectToNewClient(
		client->hPipeInst,
		&client->oOverlap);

	client->dwState = client->fPendingIO ?
		CONNECTING_STATE : // still connecting 
		READING_STATE;     // ready to read 
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL NamedPipeServerWindows::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

void NamedPipeServerWindows::sendError(std::string msg, unsigned int ec)
{
	if (_onError)
		_onError(msg, ec);
}

void NamedPipeServerWindows::readFromClient(LPPIPEINST client)
{
	bool success = ReadFile(
		client->hPipeInst,
		client->chRequest,
		BUFSIZE * sizeof(unsigned char),
		&client->cbRead,
		&client->oOverlap);

	// The read operation completed successfully. 

	if (success && client->cbRead != 0)
	{
		client->fPendingIO = FALSE;
		client->dwState = WRITING_STATE;
		return;
	}

	// The read operation is still pending. 

	if (!success && (GetLastError() == ERROR_IO_PENDING))
	{
		client->fPendingIO = TRUE;
		return;
	}

	// An error occurred; disconnect from the client. 

	DisconnectAndReconnect(client);
}

bool NamedPipeServerWindows::checkPendingIO(LPPIPEINST client)
{
	if (client->fPendingIO)
	{
		bool success = GetOverlappedResult(
			client->hPipeInst, // handle to pipe 
			&(client->oOverlap), // OVERLAPPED structure 
			&_cbRet,            // bytes transferred 
			FALSE);            // do not wait 

		switch (client->dwState)
		{
			// Pending connect operation 
		case CONNECTING_STATE:
			if (!success)
			{
				printf("Error %d.\n", GetLastError());
				return false;
			}
			client->dwState = READING_STATE;
			break;

			// Pending read operation 
		case READING_STATE:
			if (!success || _cbRet == 0)
			{
				DisconnectAndReconnect(client);
				return false;
			}
			client->cbRead = _cbRet;
			client->dwState = WRITING_STATE;
			break;

			// Pending write operation 
		case WRITING_STATE:
			if (!success || _cbRet != client->cbToWrite)
			{
				DisconnectAndReconnect(client);
				return false;
			}
			client->dwState = READING_STATE;
			break;

		default:
		{
			printf("Invalid pipe state.\n");
		}
		}
	}
	return true;
}

VOID NamedPipeServerWindows::GetAnswerToRequest(LPPIPEINST pipe)
{
	if (!_onRequest)
		return;

	unsigned char msg[BUFSIZE];
	size_t num_of_byte = _onRequest(pipe->chRequest, msg, pipe->cbRead);
	msg[num_of_byte] = 0;

	memcpy(pipe->chReply, msg, num_of_byte);
	pipe->cbToWrite = num_of_byte * sizeof(unsigned char);
}

bool NamedPipeServerWindows::generatePipes()
{
	for (int i = 0; i < INSTANCES; i++)
	{
		_hEvents[i] = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 

		if (_hEvents[i] == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
			return 0;
		}

		_pipe[i].oOverlap.hEvent = _hEvents[i];

		_pipe[i].hPipeInst = CreateNamedPipe(
			_pipe_name,            // pipe name 
			PIPE_ACCESS_DUPLEX |     // read/write access 
			FILE_FLAG_OVERLAPPED,    // overlapped mode 
			PIPE_TYPE_BYTE |      // message-type pipe 
			PIPE_READMODE_BYTE |  // message-read mode 
			PIPE_WAIT,               // blocking mode 
			INSTANCES,               // number of instances 
			BUFSIZE * sizeof(unsigned char),   // output buffer size 
			BUFSIZE * sizeof(unsigned char),   // input buffer size 
			PIPE_TIMEOUT,            // client time-out 
			NULL);                   // default security attributes 

		if (_pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed with %d %d.\n", GetLastError(), i);
			return 0;
		}

		// Call the subroutine to connect to the new client

		_pipe[i].fPendingIO = ConnectToNewClient(
			_pipe[i].hPipeInst,
			&_pipe[i].oOverlap);

		_pipe[i].dwState = _pipe[i].fPendingIO ?
			CONNECTING_STATE : // still connecting 
			READING_STATE;     // ready to read 
	}
}

void NamedPipeServerWindows::start()
{
	generatePipes();

	_running = true;
	std::thread run_thread(&NamedPipeServerWindows::run, this);
	run_thread.detach();
}

void NamedPipeServerWindows::run()
{
	while (_running)
	{
		BOOL success = FALSE;
		// Wait for the event object to be signaled, indicating 
		// completion of an overlapped read, write, or 
		// connect operation. 

		DWORD wait = WaitForMultipleObjects(
			INSTANCES,    // number of event objects 
			_hEvents,      // array of event objects 
			FALSE,        // does not wait for all 
			INFINITE);    // waits indefinitely 

						  // wait shows which pipe completed the operation. 

		int i = wait - WAIT_OBJECT_0;  // determines which pipe 
		
		if (i < 0 || i >(INSTANCES - 1))
		{
			printf("Index out of range %d.\n");
			continue;
		}

		// Get the result if the operation was pending. 

		if (!checkPendingIO(&_pipe[i]))
		{
			printf("Pipe Borked: %d \n", i);
			continue;
		}

		// The pipe state determines which operation to do next. 

		switch (_pipe[i].dwState)
		{
		case READING_STATE:

			readFromClient(&_pipe[i]);
			break;

		case WRITING_STATE:
			
			GetAnswerToRequest(&_pipe[i]);
			writeToClient(&_pipe[i]);
			break;

		default:
			printf("Invalid pipe state.\n");
		}
	}
}
