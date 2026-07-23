#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define LISTEN_PORT 47010
#define RELAY_PORT 47001
#define RELAY_IP "127.0.0.1"
#define PACKET_SIZE 164

int main(void) {
    int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_fd < 0) {
        perror("socket listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(LISTEN_PORT);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("bind listen");
        exit(EXIT_FAILURE);
    }

    int send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_fd < 0) {
        perror("socket send");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in relay_addr;
    memset(&relay_addr, 0, sizeof(relay_addr));
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_port = htons(RELAY_PORT);
    inet_pton(AF_INET, RELAY_IP, &relay_addr.sin_addr);

    unsigned char buffer[PACKET_SIZE];
    unsigned long packet_count = 0;

    while (1) {
        ssize_t bytes_read = recv(listen_fd, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) {
            continue;
        }

        // Send primary packet
        sendto(send_fd, buffer, bytes_read, 0, (struct sockaddr *)&relay_addr, sizeof(relay_addr));

        // Duplicate 15 out of 16 packets to stay strictly under 2.00x overhead
        if ((packet_count % 16) != 0) {
            sendto(send_fd, buffer, bytes_read, 0, (struct sockaddr *)&relay_addr, sizeof(relay_addr));
        }

        packet_count++;
    }

    close(listen_fd);
    close(send_fd);
    return 0;
}