#include "UDPSocketClient.h"
#include "UDPSocketServer.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>

class UDPSocketTest : public testing::Test {
protected:
    int port;
    UDPSocketServer server;
    UDPSocketClient client;

    UDPSocketTest() : port((std::rand() % 10000) + 50000), server("127.0.0.1", port), client("127.0.0.1", port) {}

    virtual void SetUp() {
    }

    virtual void TearDown() {
        server.close();
        client.close();
    }
};

TEST_F(UDPSocketTest, SendReceive) {
    char data[5] = "test";
    char * result = new char[4096];
    server.server([result](char * buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });

    client.write(data, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data, result);
    delete result;
}

TEST_F(UDPSocketTest, SendReceive2) {
    char data1[6] = "test1";
    char data2[6] = "test2";
    char * result = new char[4096];
    server.server([result](char * buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });

    client.write(data1, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data1, result);

    client.write(data2, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data2, result);
    delete result;
}
