#ifndef _UDP_SOCKET_SERVER
#define _UDP_SOCKET_SERVER

#include "CommunicationServer.h"

#ifdef _WIN32
#   include <WinSock2.h>
#   include <WS2tcpip.h>
#   include <Windows.h>
#else
#   include <arpa/inet.h>
#   include <string.h>
#   include <sys/socket.h>
#   include <unistd.h>
#   define Sleep(x) usleep(x*1000)
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
#endif

#include <thread>

#define DEFAULT_BUFFER_SIZE 4096
#define SOCKET_READ_TIMEOUT_SEC 1
#ifndef INVALID_SOCKET
#   define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#   define SOCKET_ERROR (-1)
#endif

/**
 * Asynchronous UDP socket server
 *
 * Refer to the following link for Microsoft's Server C++ code:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx
 */
class UDPSocketServer : public CommunicationServer<void(char *, unsigned int)> {
private:
    /** The address to bind to */
    std::string address;

    /** The port to listen on */
    int port;

    /** Associated socket id */
    SOCKET socketId;

    /** Bound socket address */
    SOCKADDR_IN addr;

    /** Is Socket Alive **/
    bool socketLive = false;

protected:
    void handle(std::function<void(char *, unsigned int)> handler) {
        char * buffer = new char[DEFAULT_BUFFER_SIZE];
        unsigned int buffer_len = DEFAULT_BUFFER_SIZE;
        unsigned int len = 0;

        memset(buffer, 0, buffer_len);
        while (!closeMessage) {
            len = recv(socketId, buffer, buffer_len, 0);
            if (len > 0) {
                handler(buffer, len);
                memset(buffer, 0, buffer_len);
            }
            Sleep(10);
        }

        delete buffer;
        CommunicationServer::handle(handler);
    };

    static int createSocket(SOCKET& sockId)
    {   
        if ((sockId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
            printf("UDPSocketServer: Could not create socket.\n");
            return 0;
        } 
        return 1;
    };

public:
    /**
     * Initialize a new UDP socket server.
     */
    UDPSocketServer() : CommunicationServer(){
        socketLive = createSocket(socketId);
    };


    /**
    * Binds the UDP socket to provided address and port.
    * Returns PORT_BIND_ERROR if binding failes.
    *
    * @param address    The address to bind to
    * @param port        The port to listen on    
    **/
    int bindSocket(const std::string inAddress, int inPort)
    {
        if(!socketLive && !(socketLive = createSocket(socketId)))
            return 0;

        this->address = inAddress;
        this->port = inPort;

        addr.sin_family = AF_INET;
        // TODO Support IPv6, use InetPton (inet_addr is deprecated)
        addr.sin_addr.s_addr = inet_addr(address.c_str());
        addr.sin_port = htons(port);   

        if (bind(socketId, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("UDPSocketServer: Error binding to the socket address.\n");
            return 0;
        }
        else
            return 1;
    }

    ~UDPSocketServer() {};

    void close() {
        CommunicationServer::close();
        socketLive = false;
#ifdef _WIN32
        closesocket(socketId);
#else
        ::close(socketId);
#endif
    };
};

#endif