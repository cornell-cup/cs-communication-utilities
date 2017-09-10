#ifndef _COMMUNICATION_CLIENT
#define _COMMUNICATION_CLIENT

class CommunicationClient {
protected:
    int connected;
public:
    /** Constructor */
    CommunicationClient() {};

    /** Destructor */
    virtual ~CommunicationClient() {
        close();
    };

    /** Connect or reconnect to the communication medium. */
    virtual void connect() {};

    /** @return    Whether or not the communication medium is connected. */
    virtual bool isConnected() {
        return connected;
    };

    /**
     * Read bytes of data from the communication medium. This call is non-blocking.
     *
     * @param buffer    The buffer to read data to
     * @param maxlen    The maximum number of bytes to read
     * @return          The number of bytes read
     */
    virtual int read(char * buffer, unsigned int len) {
        return 0;
    };

    /**
     * Write bytes of data through the communication medium. This call is non-blocking.
     * @param buffer    The buffer to write data from
     * @param len       The number of bytes to write
     * @return          Whether or not the write was successful
     */
    virtual int write(const char * buffer, unsigned int len) {
        return 0;
    };

    /** Close the connection */
    virtual void close() {
        connected = false;
    };
};

#endif
