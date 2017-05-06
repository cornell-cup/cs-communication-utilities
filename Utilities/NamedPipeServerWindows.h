#ifndef NAMED_PIPE_SERVER_WINDOWS_H
#define NAMED_PIPE_SERVER_WINDOWS_H

#include "NamedPipeServer.h"

#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <cstring>
#include <thread>

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	unsigned char chRequest[BUFSIZE];
	DWORD cbRead;
	unsigned char chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;


class NamedPipeServerWindows :
	public NamedPipeServer
{
	friend NamedPipeServer;

public:
	void setOnRequestCallback(size_t(*_onRequest)(unsigned char * data_in, unsigned char * data_out, size_t size_of_data_in)) override;
	virtual void setOnErrorCallback(void(*onError)(std::string msg, unsigned int ec));
	virtual void start() override;
	virtual void stop() override;
	virtual bool isRunning();
	~NamedPipeServerWindows();

private:
	NamedPipeServerWindows(LPTSTR name);
	LPTSTR _pipe_name;
	PIPEINST _pipe[INSTANCES];
	HANDLE _hEvents[INSTANCES];
	bool generatePipes();
	DWORD _cbRet;
	size_t(*_onRequest)(unsigned char * data_in, unsigned char * data_out, size_t size_of_data_in);
	void(*_onError)(std::string msg, unsigned int ec);

	// Inherited via NamedPipeServer
	VOID DisconnectAndReconnect(LPPIPEINST);
	BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
	VOID GetAnswerToRequest(LPPIPEINST);
	void run();
	void sendError(std::string msg, unsigned int ec);
	void readFromClient(LPPIPEINST client);
	bool checkPendingIO(LPPIPEINST client);
	void writeToClient(LPPIPEINST client);

	bool _running = false;

};

NamedPipeServer* NamedPipeServer::create(const char * name)
{
	return new NamedPipeServerWindows(const_cast<LPTSTR>(name));
}

#endif // ! 
