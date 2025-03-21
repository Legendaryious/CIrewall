#include "CIrewall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 80
#define IPC_SIZE 10
#define SHM_SIZE (sizeof(int) * IPC_SIZE)

MultiSocketManager* create_multi_socket_manager(int *ports, int port_count) {
  const char *default_interfaces[MAX_PORTS];
  for (int i = 0; i < port_count; i++) {
        default_interfaces[i] = INADDR_ANY; // INADDR_ANY
    }
  return create_multi_socket_manager_with_interfaces(default_interfaces, ports, port_count)
}

MultiSocketManager* create_multi_socket_manager_with_interfaces(const char *interfaces[], int *ports, int port_count) {
    MultiSocketManager *manager = (MultiSocketManager *)malloc(sizeof(MultiSocketManager));
    if (!manager) {
        perror("Failed to allocate memory for MultiSocketManager");
        return NULL;
    }
    manager->sockets = malloc(port_count * sizeof(SocketManager)); // Start with space for 2 sockets
    if (manager->sockets == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for sockets.\n");
        free(manager);
        return NULL;
    }
    manager->port_count = port_count;

    for (int i = 0; i < port_count; i++) {
        // Create socket
        manager->sockets[i].socket_fd = create_raw_socket();
        if (manager->sockets[i].socket_fd < 0) {
            perror("Failed to create socket");
            free(manager);
            return NULL;
        }

        // Set up the address structure
        memset(&manager->sockets[i].address, 0, sizeof(manager->sockets[i].address));
        manager->sockets[i].address.sin_family = AF_INET;
        manager->sockets[i].address.sin_addr.s_addr = interfaces[i]; // Bind to all interfaces
        manager->sockets[i].address.sin_port = htons(ports[i]); // Convert port to network byte order
    }

    return manager;
}

int add_socket(MultiSocketManager *manager, int socket_fd, const char *interface, int port) {
    // Allocate memory for one more SocketManager
    SocketManager *new_sockets = realloc(manager->sockets, (manager->port_count + 1) * sizeof(SocketManager));
    if (new_sockets == NULL) {
        fprintf(stderr, "Error: Memory allocation failed during reallocation.\n");
        return -1; // Indicate failure
    }
    manager->sockets = new_sockets;

    // Initialize the new SocketManager
    SocketManager new_socket;
    new_socket.socket_fd = socket_fd;
    // Set up the address structure
    memset(&new_socket.address, 0, sizeof(new_socket.address));
    new_socket.address.sin_family = AF_INET;
    new_socket.address.sin_addr.s_addr = interface; // Bind to all interfaces
    new_socket.address.sin_port = htons(port); // Convert port to network byte order
    new_socket.address = interface;

    // Add the new SocketManager to the MultiSocketManager
    manager->sockets[manager->port_count] = new_socket;
    manager->port_count++; // Increment the count of managed ports

    return 0; // Indicate success
}

int add_port(MultiSocketManager *manager, const char *interface, int port) {
  return add_socket(manager,create_raw_socket(),interface,port);
}

// Function to clean up the MultiSocketManager
void free_manager(MultiSocketManager *manager) {
    if (manager != NULL) {
        free(manager->sockets); // Free the dynamically allocated array
        free(manager);          // Free the manager itself
    }
}

// Bind all sockets to their respective addresses
void bind_sockets(MultiSocketManager *manager) {
    for (int i = 0; i < manager->port_count; i++) {
        if (bind(manager->sockets[i].socket_fd, (struct sockaddr *)&manager->sockets[i].address, sizeof(manager->sockets[i].address)) < 0) {
            perror("Failed to bind socket");
            close_sockets(manager);
            exit(EXIT_FAILURE);
        }
    }
}

void listen_sockets(int backlog){
    char buffer[4096];
    for (int i = 0; i < manager->port_count; i++) {
        if (listen(manager->sockets[i].socket_fd, backlog) < 0) {
            perror("Failed to listen on socket");
            close_sockets(manager);
            exit(EXIT_FAILURE);
        }
    }
  
    // Receive packets
    ssize_t packet_size = recv(sock, buffer, sizeof(buffer), 0);
    if (packet_size < 0) {
        perror("Receive failed");
        continue;
    }

    // Here you can analyze the packet if needed, but we will drop it
    struct iphdr *ip_header = (struct iphdr *)buffer;
    struct tcphdr *tcp_header = (struct tcphdr *)(buffer + (ip_header->ihl * 4));

    // Log the source of the packet (optional)
    printf("Dropped packet from %s:%d\n", inet_ntoa(ip_header->saddr), ntohs(tcp_header->source));
    
    // No response is sent, effectively dropping the packet
}

// Handle incoming packets and drop them
void handle_packets(MultiSocketManager *multi_manager) {
    char buffer[BUFFER_SIZE];

    printf("Listening on multiple ports and dropping all incoming packets...\n");

    while (1) {
        fd_set read_fds;
        int max_sd = -1;

        // Clear the socket set
        FD_ZERO(&read_fds);

        // Add each socket to the set
        for (int i = 0; i < multi_manager->count; i++) {
            int sd = multi_manager->sockets[i]->socket_fd;
            FD_SET(sd, &read_fds);
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Wait for activity on the sockets
        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            continue;
        }

        // Check each socket for incoming packets
        for (int i = 0; i < multi_manager->count; i++) {
            int sd = multi_manager->sockets[i]->socket_fd;

            if (FD_ISSET(sd, &read_fds)) {
                ssize_t packet_size = recv(sd, buffer, sizeof(buffer), 0);
                if (packet_size < 0) {
                    perror("Receive failed");
                    continue;
                }

                // Analyze the packet (optional)
                struct iphdr *ip_header = (struct iphdr *)buffer;
                struct tcphdr *tcp_header = (struct tcphdr *)(buffer + (ip_header->ihl * 4));

                // Log the source of the packet (optional)
                printf("Dropped packet from %s:%d\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr), ntohs(tcp_header->source));

                // No response is sent, effectively dropping the packet
            }
        }
    }
}


// Function to create a raw socket
int create_raw_socket() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int* attach_shared_memory(key_t key, int create) {
    int shmid;

    // Use IPC_CREAT only in the writer
    if (create) {
        shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    } else {
        shmid = shmget(key, SHM_SIZE, 0666); // No IPC_CREAT for reader
    }

    if (shmid < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    int *sharedArray = shmat(shmid, NULL, 0);
    if (sharedArray == (int *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    return sharedArray;
}


int main(int argc, char *argv[]) {
    int opts = handle_cli_options(argc, argv);
  
    key_t key = ftok("session.lock", 'R');
    int *sharedArray = attach_shared_memory(key, opts & MODE_MAIN); // Attach to shared memory with creation
    if(opts & MODE_MAIN){
      for (int i = 0; i < ARRAY_SIZE; i++) {
          sharedArray[i] = 0; // Initialize all elements to 0
      }
      MultiManager = create_multi_socket_manager((int *)DEFAULT_PORTS,DEFAULT_PORTS_LENGTH);
      int sock = create_raw_socket();
      int PORT
      struct sockaddr_in addr;
      char buffer[4096];
  
      // Set up the address structure
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = htons(PORT);
  
      // Bind the raw socket to the port
      if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
          perror("Bind failed");
          close(sock);
          exit(EXIT_FAILURE);
      }
  
      printf("Listening on port %d and dropping all incoming packets...\n", DEFAULT_PORT);
  
      while (1) {
          // Receive packets
          ssize_t packet_size = recv(sock, buffer, sizeof(buffer), 0);
          if (packet_size < 0) {
              perror("Receive failed");
              continue;
          }
  
          // Here you can analyze the packet if needed, but we will drop it
          struct iphdr *ip_header = (struct iphdr *)buffer;
          struct tcphdr *tcp_header = (struct tcphdr *)(buffer + (ip_header->ihl * 4));
  
          // Log the source of the packet (optional)
          printf("Dropped packet from %s:%d\n", inet_ntoa(ip_header->saddr), ntohs(tcp_header->source));
          
          // No response is sent, effectively dropping the packet
      }
  
      close(sock);
    }
    return 0;
}
