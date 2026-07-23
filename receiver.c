#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <stdint.h>

#define LISTEN_PORT 47002
#define PLAYER_PORT 47020
#define PLAYER_IP "127.0.0.1"
#define PACKET_SIZE 164
#define MAX_SEQ_NUMBERS 1048576
#define BUFFER_SIZE (1024 * 1024) // 1MB socket buffer

static uint8_t seen_bitset[MAX_SEQ_NUMBERS / 8];

static inline int is_seen(uint32_t seq) {
    if (seq >= MAX_SEQ_NUMBERS) return 0;
    return (seen_bitset[seq / 8] & (1 << (seq % 8))) != 0;
}

static inline void mark_seen(uint32_t seq) {
    if (seq < MAX_SEQ_NUMBERS) {
        seen_bitset[seq / 8] |= (1 << (seq % 8));
    }
}

int main(void) {
    memset(seen_bitset, 0, sizeof(seen_bitset));

    int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_fd < 0) {
        perror("socket listen");
        exit(EXIT_FAILURE);
    }

    int send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_fd < 0) {
        perror("socket send");
        exit(EXIT_FAILURE);
    }

    // Set low-latency traffic class
    int tos = IPTOS_LOWDELAY;
    setsockopt(send_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));

    // Increase socket buffers to prevent kernel drops
    int buf_size = BUFFER_SIZE;
    setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
    setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(LISTEN_PORT);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("bind listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in player_addr;
    memset(&player_addr, 0, sizeof(player_addr));
    player_addr.sin_family = AF_INET;
    player_addr.sin_port = htons(PLAYER_PORT);
    inet_pton(AF_INET, PLAYER_IP, &player_addr.sin_addr);

    unsigned char buffer[PACKET_SIZE];

    while (1) {
        ssize_t bytes_read = recv(listen_fd, buffer, sizeof(buffer), 0);
        if (bytes_read < 4) {
            continue;
        }

        uint32_t seq = ((uint32_t)buffer[0] << 24) |
                       ((uint32_t)buffer[1] << 16) |
                       ((uint32_t)buffer[2] << 8)  |
                       ((uint32_t)buffer[3]);

        if (!is_seen(seq)) {
            mark_seen(seq);
            sendto(send_fd, buffer, bytes_read, 0, (struct sockaddr *)&player_addr, sizeof(player_addr));
        }
    }

    close(listen_fd);
    close(send_fd);
    return 0;
}