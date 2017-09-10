#ifndef _UDP_SOCKET_CLIENT
#define _UDP_SOCKET_CLIENT

#include "CommunicationClient.h"

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

#define DEFAULT_BUFFER_SIZE 4096
#define SOCKET_READ_TIMEOUT_SEC 1
#ifndef INVALID_SOCKET
#   define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#   define SOCKET_ERROR (-1)
#endif

/**
 * UDP socket client
 *
 * Refer to the following link for Microsoft's Server C++ code:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx
 */
class UDPSocketClient : public CommunicationClient {
private:
    /** The address to connect to */
    std::string address;

    /** The port to connect to */
    int port;

    /** Associated socket id */
    SOCKET socketId;

    /** Bound socket address */
    SOCKADDR_IN addr;

    /** Whether or not the server is listening */
    int connected;

public:
    /**
     * Initialize a new UDP socket client.
     *
     * @param address   The address to connect to
     * @param port      The port to connect to
     */
    UDPSocketClient(std::string inAddress, int inPort) :
            CommunicationClient(), address(inAddress), port(inPort), connected(false) {
        if ((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
            printf("UDPSocketClient: Could not create socket.\n");
            return;
        }

        addr.sin_family = AF_INET;
        // TODO Support IPv6, use InetPton (inet_addr is deprecated)
        addr.sin_addr.s_addr = inet_addr(address.c_str());
        addr.sin_port = htons(port);

        connect();
    };

    /**
     * Deconstructor
     */
    ~UDPSocketClient() {};

    void connect() {
        if (!connected) {
            if (::connect(socketId, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                printf("UDPSocketClient: Error binding to the socket address.\n");
                return;
            }

            connected = true;
        }
    }

    /**
     * Read bytes of data from the port.
     * TODO Provide an asynchronous version of the function
     */
    int read(char * outBuffer, unsigned int buffer_len) {
        unsigned int len;

        len = recv(socketId, outBuffer, buffer_len, 0);

        return len;
    };

    /**
     * Write bytes of data to the port.
     */
    int write(const char * buffer, unsigned int len) {
        if (send(socketId, buffer, len, 0) < 0) {
            return 0;
        }
        // TODO Handle error codes and reconnection

        return 1;
    };

    /**
     * Close the connection.
     */
    void close() {
        if (connected) {
#ifdef _WIN32
            closesocket(socketId);
#else
            ::close(socketId);
#endif
        }
        CommunicationClient::close();
    };
};

#endif