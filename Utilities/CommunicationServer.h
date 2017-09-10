#ifndef _COMMUNICATION_SERVER
#define _COMMUNICATION_SERVER

#include <functional>
#include <thread>

template<class T>
class CommunicationServer {
protected:
    /**
     * Whether or not the server is listening
     */
    int listening;

    /**
     * Internal message to close
     */
    volatile int closeMessage;

    /**
     * Continuously handle received data (done in a new thread)
     *
     * @param handler   The function to handle data
     *                  Note, the first parameter passed to the handler
     *                  will be available only within the call to the
     *                  handler, otherwise it may be deleted at any time.
     */
    virtual void handle(std::function<T> handler) {};
public:
    /** Constructor */
    CommunicationServer() {};

    /** Destructor */
    virtual ~CommunicationServer() {};

    /** @return Whether or not the server is listening */
    virtual bool isListening() {
        return false;
    };

    /**
     * Start the server and begin listening.
     *
     * @param handler   The function to handle data from clients
     */
    void server(std::function<T> handler) {
        std::thread(&CommunicationServer::handle, this, handler).detach();
        listening = 1;
    };

    /** Close the server and stop listening. */
    virtual void close() {
        closeMessage = 1;
    };
};

#endif
