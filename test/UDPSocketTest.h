#include "UDPSocketClient.h"
#include "UDPSocketServer.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>

class UDPSocketTest : public testing::Test
{
  protected:
    int port;
    UDPSocketServer server;
    UDPSocketClient client;

    UDPSocketTest() : port((std::rand() % 10000) + 50000), client("127.0.0.1", port) {}

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        server.close();
        client.close();
    }

    static void genRandomString(char *s, int length)
    {
        const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < length - 1; ++i)
        {
            s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        s[length - 1] = 0;
    }
};

TEST_F(UDPSocketTest, SendReceive)
{
    char data[5] = "test";
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();

    client.write(data, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data, result);
    delete result;
}

TEST_F(UDPSocketTest, SendReceive2)
{
    char data1[6] = "test1";
    char data2[6] = "test2";
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();

    client.write(data1, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data1, result);

    client.write(data2, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data2, result);
    delete result;
}

TEST_F(UDPSocketTest, SendReciveMulti)
{
    char data1[6] = "test1";
    char data2[6] = "test2";
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();

    for (int i = 0; i < 10; i++)
    {
        int msg_length = (std::rand() % DEFAULT_BUFFER_SIZE + 1);
        char msg[msg_length] = {};

        genRandomString(msg, msg_length);

        client.write(msg, msg_length);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_STREQ(msg, result);
    }

    delete result;
}

TEST_F(UDPSocketTest, TestBindToUsedPortFailServer)
{
    UDPSocketServer server2;
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();

    EXPECT_EQ(0, server2.bindSocket("127.0.0.1", port));

    server2.close();
}

TEST_F(UDPSocketTest, TestCloseRebindAndListen)
{
    char data1[6] = "test1";
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();
    server.close();

    server.listen();

    ASSERT_EQ(1, server.bindSocket("127.0.0.1", port));

    client.write(data1, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data1, result);

    delete result;
}

TEST_F(UDPSocketTest, TestClientSendToUnknownPort)
{
    char data[5] = "test";
    ASSERT_TRUE(client.write(data, 5));
}

TEST_F(UDPSocketTest, TestClientCloseConnectSend)
{
    char data[5] = "test";
    char *result = new char[4096];

    server.setDataHandler([result](char *buffer, unsigned int buffer_len) {
        std::strncpy(result, buffer, buffer_len);
    });
    server.bindSocket("127.0.0.1", port);
    server.listen();

    client.close();
    ASSERT_TRUE(client.connect());

    ASSERT_TRUE(client.write(data,5));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_STREQ(data, result);
    
    delete result;
}