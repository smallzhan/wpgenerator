#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "wpsocket.h"

#ifdef WIN32
#include <winsock.h>
typedef int socklen_t;
typedef char raw_type;

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
typedef void raw_type;
#endif

namespace WaterPrinter
{
#ifdef WIN32
static bool s_initialized = false;
#endif

static void fillAddress(const std::string &addr,
                        unsigned short port,
                        sockaddr_in &addr_in)
{
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;

    hostent *host;
    if ((host = gethostbyname(addr.c_str())) != NULL)
    {
        addr_in.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
        // FIXME: remove 127.0.0.1
        addr_in.sin_port = htons(port);
    }
}

////////////////////////////////////////////////////////////////
//////////////// Socket
////////////////////////////////////////////////////////////////

Socket::Socket(int type, int protocol)
{
#ifdef WIN32
    if (!s_initialized)
    {
        WORD version_req;
        WSDATA wsa_data;

        version_req = MAKEWORD(2, 0);
        if (WSAStartup(version_req, &wsa_data) = 0)
        {
            s_initialized = true;
        }
    }
#endif
    socket_ = socket(AF_INET, type, protocol);

    if (socket_ < 0)
    {
        printf("socket init failed!\n");
    }

}


Socket::Socket(int socket)
{
    this->socket_ = socket;
}


Socket::~Socket()
{
#ifdef WIN32
    ::closesocket(socket_);
#else
    ::close(socket_);
#endif
    socket_ = -1;
}

std::string Socket::getLocalAddress()
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getsockname(socket_, (sockaddr*)&addr, (socklen_t *) &addr_len) >= 0)
    {
        return inet_ntoa(addr.sin_addr);
    }
}

unsigned short Socket::getLocalPort()
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getsockname(socket_, (sockaddr*)&addr, (socklen_t *) &addr_len) >= 0)
    {
        return ntohs(addr.sin_port);
    }
}

void Socket::setLocalPort(unsigned short local_port)
{
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(local_port);

    if (bind(socket_, (sockaddr*)&local_addr, sizeof(sockaddr_in)) < 0)
    {
        printf("bind socket failed\n");
    }
}

void Socket::setLocalAddressAndPort(const std::string &local_addr,
                                    unsigned short local_port)
{
    sockaddr_in local_addr_in;
    fillAddress(local_addr, local_port, local_addr_in);

    if (bind(socket_, (sockaddr*)&local_addr, sizeof(sockaddr_in)) < 0)
    {
        printf("bind socket failed\n");
    }
}

void Socket::cleanUp()
{
#ifdef WIN32
    if (WSACleanup() != 0)
    {
        printf("WSACLeanup failed on windows\n");
    }
#endif
}

unsigned short Socket::resolveService(const std::string &service,
                                      const std::string &protocol)
{
    struct servent *serv;

    if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
    {
        return atoi(service.c_str());
    }
    else
    {
        return ntohs(serv->s_port);
    }
}

int Socket::setSockUnblock(int unblock)
//inline int Socket::setSockUnblock(int unblock)
{
#ifdef _WIN32
    if(ioctlsocket(socket_, FIONBIO, (unsigned long *)&unblock) == 0)
        return 0;
#else
    int flags;
    flags = fcntl(socket_, F_GETFL, 0);
    if (flags == -1) {
        perror("GET SOCKET OPTION\n");
        return -1;
    }
    if (fcntl(socket_, F_SETFL, flags|O_NONBLOCK) == 0) return 0;
#endif
    return -1;
}

int Socket::setSockNodelay(int nodelay)
{
    if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) == 0)
        return 0;
    return -1;
}

inline int Socket::setSockTimeout(int timeout)
{
    if(setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == 0)
        return 0;
    return -1;
}
////////////////////////////////////////////////////////////////
//////////////// WPSocket
////////////////////////////////////////////////////////////////

WPSocket::WPSocket(int type, int protocol)
    : Socket(type, protocol)
{}

WPSocket::WPSocket(int new_conn) : Socket(new_conn)
{}

void WPSocket::connect(const std::string &peer_addr,
                       unsigned short peer_port)
{
    sockaddr_in dest_addr;
    fillAddress(peer_addr, peer_port, dest_addr);

    if (::connect(socket_, (sockaddr*) &dest_addr, sizeof(dest_addr)) < 0)
    {
        printf("connect to peer failed\n");
    }
}

void WPSocket::send(const void *buffer, int buf_len)
{
    if (::send(socket_, (raw_type*) buffer, buf_len, 0) < 0)
    {
        printf("send failed\n");
    }
}

int WPSocket::recv(void *buffer, int buf_len)
{
    int recv_size = -1;
    //printf("before recv\n");
    if ((recv_size = ::recv(socket_, (raw_type*)buffer, buf_len, 0)) < 0)
    {
    // non block recv nothing.
    //printf("recv failed\n");
    }
    //printf("after recv\n");

    return recv_size;
}

std::string WPSocket::getPeerAddress()
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getpeername(socket_, (sockaddr*) &addr, (socklen_t*) &addr_len) < 0)
    {
        printf("get peer name failed\n");
    }
    return inet_ntoa(addr.sin_addr);
}

unsigned short WPSocket::getPeerPort()
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getpeername(socket_, (sockaddr*) &addr, (socklen_t*) &addr_len) < 0)
    {
        printf("getpeername failed\n");
    }
    return ntohs(addr.sin_port);
}

void WPSocket::getPeerInfo(std::string & peer_addr, unsigned short & peer_port)
{
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    if (getpeername(socket_, (sockaddr*) &addr, (socklen_t*) &addr_len) < 0)
    {
        printf("getpeername failed\n");
    }

    peer_addr = inet_ntoa(addr.sin_addr);
    peer_port = ntohs(addr.sin_port);
}

////////////////////////////////////////////////////////////////
//////////////// TCPSocket
////////////////////////////////////////////////////////////////

TCPSocket::TCPSocket() : WPSocket(SOCK_STREAM, IPPROTO_TCP)
{}

TCPSocket::TCPSocket(const std::string &peer_addr, unsigned short peer_port)
    : WPSocket(SOCK_STREAM, IPPROTO_TCP)
{
    connect(peer_addr, peer_port);
}

TCPSocket::TCPSocket(int new_conn) : WPSocket(new_conn)
{}

////////////////////////////////////////////////////////////////
//////////////// TCPServerSocket
////////////////////////////////////////////////////////////////

TCPServerSocket::TCPServerSocket(unsigned short local_port, int queue_len)
    : Socket(SOCK_STREAM, IPPROTO_TCP)
{
    int yes = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    setLocalPort(local_port);

    setListen(queue_len);
}

TCPServerSocket::TCPServerSocket(const std::string& local_addr,
                                 unsigned short local_port,
                                 int queue_len)
    : Socket(SOCK_STREAM, IPPROTO_TCP)
{
    int yes = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    setLocalAddressAndPort(local_addr, local_port);

    setListen(queue_len);
}

TCPSocket* TCPServerSocket::accept()
{
    int new_conn;
    if ((new_conn = ::accept(socket_, NULL, 0)) < 0)
    {
    //printf("accept failed\n");
        return NULL;
    }

    return new TCPSocket(new_conn);
}

void TCPServerSocket::setListen(int queue_len)
{
    if (listen(socket_, queue_len) < 0)
    {
        printf("set listen failed\n");
    }
}

////////////////////////////////////////////////////////////////
//////////////// UDPSocket
////////////////////////////////////////////////////////////////

UDPSocket::UDPSocket() : WPSocket(SOCK_DGRAM, IPPROTO_UDP)
{
    setBroadcast();
}

UDPSocket::UDPSocket(unsigned short local_port)
    : WPSocket(SOCK_DGRAM, IPPROTO_UDP)
{
    setLocalPort(local_port);
    setBroadcast();
}

void UDPSocket::setBroadcast()
{
    int broadcast = 1;
    setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, (raw_type*) &broadcast, sizeof(broadcast));
}

void UDPSocket::disconnect()
{
    sockaddr_in null_addr;
    memset(&null_addr, 0, sizeof(null_addr));
    null_addr.sin_family = AF_UNSPEC;

    if (::connect(socket_, (sockaddr*) &null_addr, sizeof(null_addr)) < 0)
    {
#ifdef WIN32
        if (errno != WSAEAFNOSUPPORT)
#else
        if (errno != EAFNOSUPPORT)
#endif
        {
            printf("disconnect failed\n");
        }
    }
}

void UDPSocket::sendTo(const void *buffer, int buf_len,
                       const std::string &peer_addr,
                       unsigned short peer_port)
{
    sockaddr_in dest_addr;
    fillAddress(peer_addr, peer_port, dest_addr);

    if (sendto(socket_, (raw_type*)buffer, buf_len, 0,
               (sockaddr*) &dest_addr, sizeof(dest_addr)) != buf_len)
    {
        printf("sendto failed\n");
    }

}

void UDPSocket::recvFrom(void *buffer, int buf_len, std::string &src_addr,
                        unsigned short &src_port)
{
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int recv_size;
    if ((recv_size = recvfrom(socket_, (raw_type*) buffer, buf_len, 0,
                              (sockaddr*) &client_addr, (socklen_t*) &addr_len)) < 0)
    {
        printf("recvfrom failed\n");
    }
    src_addr = inet_ntoa(client_addr.sin_addr);
    src_port = ntohs(client_addr.sin_port);
}

void UDPSocket::setMulticastTTL(unsigned char multicast_ttl)
{
    if (setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_TTL,
                   (raw_type*) &multicast_ttl, sizeof(multicast_ttl)) < 0)
    {
        printf("set multicast TTL failed\n");
    }
}

void UDPSocket::joinGroup(const std::string &multicast_group)
{
    struct ip_mreq multicast_req;
    multicast_req.imr_multiaddr.s_addr = inet_addr(multicast_group.c_str());
    multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   (raw_type*) &multicast_req,
                   sizeof(multicast_req)) < 0)
    {
        printf("join multicast group failed\n");
    }
}

void UDPSocket::leaveGroup(const std::string &multicast_group)
{
    struct ip_mreq multicast_req;
    multicast_req.imr_multiaddr.s_addr = inet_addr(multicast_group.c_str());
    multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   (raw_type*) &multicast_req,
                   sizeof(multicast_req)) < 0)
    {
        printf("leave multicast group failed\n");
    }
}


}
