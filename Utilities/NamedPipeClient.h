#ifndef NAMED_PIPE_CLIENT_H
#define NAMED_PIPE_CLAINT_H

#include <string>
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define BUF_SIZE 8

class NamedPipeClient
{
public:
	NamedPipeClient();
	~NamedPipeClient();
	virtual void send(unsigned char *, size_t size) = 0;
	virtual void close() = 0;
	virtual bool open() = 0;
	virtual void onRecieve(void(*returnFunct)(unsigned char*,size_t size)) = 0;
	virtual void errorCallback(void (*errorFunct)(std::string msg, unsigned int ec)) = 0;
	
	static NamedPipeClient* create(const char * name);

};

#endif // ! NAMED_PIPE_CLIENT_H
