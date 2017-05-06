#include <string>

#define CONNECTING_STATE 0 
#define READING_LENGTH_STATE 1 
#define READING_DATA_STATE 2 
#define WRITING_STATE 3
#define INSTANCES 5 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096


class NamedPipeServer
{
public:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual bool isRunning() = 0;
	virtual void setOnRequestCallback(unsigned char *(*_onRequest)(unsigned char * data_in, unsigned int* data_out, size_t size_of_data_in)) = 0;
	virtual void setOnErrorCallback(void(*onError)(std::string msg, unsigned int ec)) = 0;

	static NamedPipeServer* create(const char * name);

};

