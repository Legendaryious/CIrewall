#ifndef CIREWALL_H
#define CIREWALL_H

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "cli_options.h"

// Structure to manage a TCP socket
typedef struct {
    int socket_fd;               // Socket file descriptor
    struct sockaddr_in address;   // Socket address
} SocketManager;

// Structure to manage multiple sockets
typedef struct {
    SocketManager *sockets; // Array of SocketManagers
    int port_count;                   // Number of ports being managed
} MultiSocketManager;

#define DEFAULT_PORTS_LENGTH 17

static const int DEFAULT_PORTS[DEFAULT_PORTS_LENGTH]={
        80,    // HTTP
        443,   // HTTPS
        21,    // FTP
        22,    // SSH
        23,    // Telnet
        25,    // SMTP
        53,    // DNS
        110,   // POP3
        143,   // IMAP
        3389,  // RDP
        3306,  // MySQL
        5432,  // PostgreSQL
        1433,  // MSSQL
        1521,  // Oracle
        161,   // SNMP
        389,   // LDAP
        123    // NTP
}

// Function declarations
MultiSocketManager* create_multi_socket_manager(const char *interfaces[], int *ports, int port_count);
MultiSocketManager* create_multi_socket_manager_with_default(int *ports, int port_count);
int add_port(
int bind_socket(int sockfd, const struct sockaddr_in *addr);
int listen_socket(int sockfd, int backlog);
int accept_connection(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen);
int connect_socket(int sockfd, const struct sockaddr_in *addr);
ssize_t send_data(int sockfd, const void *buf, size_t len, int flags);
ssize_t receive_data(int sockfd, void *buf, size_t len, int flags);
void close_socket(int sockfd);

#endif // SOCKETLIB_H
