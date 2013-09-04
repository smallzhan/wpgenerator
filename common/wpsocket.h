#ifndef _WPRINTER_WPSOCKET_H_

#define _WPRINTER_WPSOCKET_H_

#include <string>

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
    TypeName(const TypeName&);                  \
    void operator=(const TypeName&)

namespace WaterPrinter
{
class Socket
{
public:
    ~Socket();
    std::string getLocalAddress();
    unsigned short getLocalPort();
    void setLocalPort(unsigned short local_port);
    void setLocalAddressAndPort(const std::string& local_addr,
                                unsigned short local_port = 0);
    static void cleanUp();
    static unsigned short resolveService(const std::string &service,
                                         const std::string &protocol = "tcp");

    int setSockUnblock(int unblock = 1);
    int setSockNodelay(int nodelay = 1);
    int setSockTimeout(int timeout);

protected:
    int socket_;
    Socket(int type, int protocol);
    explicit Socket(int socket);

private:
    DISALLOW_COPY_AND_ASSIGN(Socket);
};

class WPSocket : public Socket
{
public:
    void connect(const std::string &peer_addr, unsigned short peer_port);
    void send(const void *buffer, int buf_len);
    int recv(void *buffer, int buf_len);
    std::string getPeerAddress();
    unsigned short getPeerPort();
    void getPeerInfo(std::string &peer_addr, unsigned short &peer_port);

protected:
    WPSocket(int type, int protocol);
    explicit WPSocket(int new_conn);
};

class TCPSocket : public WPSocket
{
public:
    TCPSocket();
    TCPSocket(const std::string &peer_addr, unsigned short peer_port);

private:
    friend class TCPServerSocket;
    explicit TCPSocket(int new_conn);
};

class TCPServerSocket : public Socket
{
public:
    TCPServerSocket(unsigned short local_port, int queue_len = 3);
    TCPServerSocket(const std::string &local_addr, unsigned short local_port,
                    int queue_len = 5);
    TCPSocket *accept();

private:
    void setListen(int queue_len);
};

class UDPSocket : public WPSocket
{
public:
    UDPSocket();
    explicit UDPSocket(unsigned short local_port);
    UDPSocket(const std::string &local_addr, unsigned short local_port);
    void disconnect();
    void sendTo(const void *buffer, int buf_len, const std::string &peer_addr,
                unsigned short peer_port);
    void recvFrom(void *buffer, int buf_len, std::string &src_addr,
                  unsigned short &src_port);
    void setMulticastTTL(unsigned char multicast_ttl);
    void joinGroup(const std::string &multicast_group);
    void leaveGroup(const std::string &multicast_group);

private:
    void setBroadcast();
};
}
#endif  // _WPRINTER_WPSOCKET_H_
