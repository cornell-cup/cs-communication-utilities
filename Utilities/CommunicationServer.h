#ifndef _COMMUNICATION_SERVER
#define _COMMUNICATION_SERVER

#include <functional>
#include <memory>
#include <thread>

template<class T>
class CommunicationServer {
protected:
    /** Internal message to close */
    volatile int closeMessage;

    /** Whether or not the server is listening */
    int listening;

    /** Handler thread */
    std::shared_ptr<std::thread> handlerThread;
    std::function<T> handler;

    /**
     * Continuously handle received data (done in a new thread)
     *
     * @param handler   The function to handle data
     *                  Note, the first parameter passed to the handler
     *                  will be available only within the call to the
     *                  handler, otherwise it may be deleted at any time.
     */
    virtual void handle(std::function<T> handler) {
        handlerThread = nullptr;
        closeMessage = 0;
    };
public:
    /** Constructor */
    CommunicationServer() : closeMessage(0), listening(0) {};

    /** Destructor */
    virtual ~CommunicationServer() {
        close();
    };

    /** @return Whether or not the server is listening */
    virtual int isListening() {
        return listening;
    };

    /*
    * Specify how the Server handles data from clients
    *
    * @param handler   The function to handle data from clients
    */
    void setDataHandler(std::function<T> handler)
    {
        this->handler = handler;
    }

    /**
     * Start the server and begin listening.
     *
     * @return  Whether the server successfully begins listening. If a the
     *          server is already listening then it will return false.
     */
    int listen(){
        if (handlerThread == nullptr) {
            handlerThread = std::make_shared<std::thread>(&CommunicationServer::handle, this, handler);
            handlerThread->detach();
            closeMessage = 0;
            listening = 1;
            return true;
        }
        return false;
    };

    /** Close the server and stop listening. */
    virtual void close() {
        handlerThread = nullptr;
        closeMessage = 1;
        listening = 0;
    };
};

#endif
