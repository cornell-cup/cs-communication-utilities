#include "stdafx.h"
#include "NamedPipeServerWindows.h"


NamedPipeServerWindows::NamedPipeServerWindows(LPTSTR pipe_name): _pipe_name(pipe_name)
{
	
}

void NamedPipeServerWindows::setOnRequestCallback(unsigned char *(*onRequest)(unsigned char * data_in, unsigned int * size_data_out, size_t size_data_in))
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
		client->dwState = READING_LENGTH_STATE;
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
		READING_LENGTH_STATE;     // ready to read 
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

void NamedPipeServerWindows::readLengthFromClient(LPPIPEINST client)
{
	printf("Reading Length from Client... \n");
	bool success = ReadFile(
		client->hPipeInst,
		&client->r2_data_size,
		4,
		&client->cbRead,
		&client->oOverlap);

	if (success && client->cbRead != 0)
	{
		client->fPendingIO = FALSE;
		client->dwState = READING_DATA_STATE;
	}

	// The read operation is still pending. 

	if (!success && (GetLastError() == ERROR_IO_PENDING))
	{
		client->fPendingIO = TRUE;
	}
}

void NamedPipeServerWindows::readDataFromClient(LPPIPEINST client)
{
	printf("Reading Data from Client... \n");
	unsigned int length_of_data = 0;

	client->r2_data_size = _byteswap_ushort(client->r2_data_size);
	printf("Length of data: %d", client->r2_data_size);

	bool success = ReadFile(
		client->hPipeInst,
		client->r2_data,
		client->r2_data_size,
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

	//DisconnectAndReconnect(client);
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
			printf("Checking connection pending... \n");
			if (!success)
			{
				printf("Error %d.\n", GetLastError());
				return false;
			}
			client->dwState = READING_LENGTH_STATE;
			break;

			// Pending read operation 
		case READING_LENGTH_STATE:
			printf("Checking read length pending... \n");
			if (_cbRet == 0)
			{
				return false;
			}
			printf("Size of r2 data: %d \n", client->r2_data_size);
			client->cbRead = _cbRet;
			client->r2_data = new unsigned char[client->r2_data_size];
			client->dwState = READING_DATA_STATE;
			break;

		case READING_DATA_STATE:
			printf("Checking read data pending... \n");
			if (_cbRet == 0)
			{
				return false;
			}
			client->cbRead = _cbRet;
			client->dwState = WRITING_STATE;
			// Pending write operation 
		case WRITING_STATE:
			printf("Checking write pending... \n");
			if (!success || _cbRet != client->cbToWrite)
			{
				return false;
			}

			delete client->r2_data;
			delete client->chReply;
			client->dwState = READING_LENGTH_STATE;
			break;

		default:
		{
			printf("Invalid pipe state. \n");
		}
		}
	}
	return true;
}

VOID NamedPipeServerWindows::GetAnswerToRequest(LPPIPEINST pipe)
{
	if (!_onRequest)
		return;
	unsigned int size_of_msg;
	unsigned char * msg = _onRequest(pipe->r2_data, &size_of_msg, pipe->r2_data_size);

	if (msg)
	{
		memcpy(pipe->chReply, msg, size_of_msg);
		pipe->cbToWrite = size_of_msg * sizeof(unsigned char);
	}
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
			16000000 * sizeof(unsigned char),   // output buffer size 
			16000000 * sizeof(unsigned char),   // input buffer size 
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
			READING_LENGTH_STATE;     // ready to read 
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
		case READING_LENGTH_STATE:
			readLengthFromClient(&_pipe[i]);
			break;

		case READING_DATA_STATE:
			readDataFromClient(&_pipe[i]);
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
