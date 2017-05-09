#include "NamedPipeClient.h"
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <thread>
#include <intrin.h>


class NamedPipeClientWindows :
	public NamedPipeClient
{
	friend NamedPipeClient;
public:
	~NamedPipeClientWindows();

	// Inherited via NamedPipeClient
	virtual void send(unsigned char *, size_t size) override;
	virtual void close() override;
	virtual bool open() override;
	virtual void onRecieve(void(*returnFunct)(unsigned char *, size_t size)) override;
	virtual void errorCallback(void(*errorFunct)(std::string msg, unsigned int ec)) override;	

protected:
	void sendError(std::string msg, unsigned int ec);
	void sendRecieve(unsigned char * data, size_t size);
	bool setPipeState(DWORD state);

private:
	NamedPipeClientWindows(LPTSTR name);

	HANDLE _pipe;
	unsigned char _readBuffer[BUF_SIZE];
	BOOL   _fSuccess = FALSE;
	DWORD  _cbRead, _cbToWrite, _cbWritten;
	LPTSTR _pipeName;
	void(*_onRecieveCallBack)(unsigned char *, size_t size);
	void(*_onErrorCallback)(std::string msg, unsigned int ec);
	void startReadPolling();
	void pollServer();
	bool _reading = false;


	// Inherited via NamedPipeClient
};

NamedPipeClient* NamedPipeClient::create(const char * name)
{
	return new NamedPipeClientWindows(const_cast<LPTSTR>(name));
}
