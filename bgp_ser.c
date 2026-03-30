// bgp_server_packets.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 179
#define BGP_MARKER_SIZE 16
#define MAX_PACKET_SIZE 4096

void handle_bgp_packet(unsigned char *packet, int len) {
    if (len < 19) {
        printf("✘ Packet too short to be a valid BGP message.\n");
        return;
    }

    // Check marker
    for (int i = 0; i < BGP_MARKER_SIZE; i++) {
        if (packet[i] != 0xFF) {
            printf("✘ Invalid BGP marker.\n");
            return;
        }
    }

    // Extract BGP header
    int length = (packet[16] << 8) | packet[17];
    int type = packet[18];

    printf("✔ BGP packet received: Type = %d, Length = %d\n", type, length);

    switch (type) {
        case 1:
            printf("→ BGP OPEN received\n");
            break;
        case 2:
            printf("→ BGP UPDATE received\n");
            break;
        case 3:
            printf("→ BGP NOTIFICATION received\n");
            break;
        case 4:
            printf("→ BGP KEEPALIVE received\n");
            break;
        default:
            printf("→ Unknown BGP message type: %d\n", type);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    unsigned char buffer[MAX_PACKET_SIZE] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("✅ BGP server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) {
        perror("Accept failed");
        exit(1);
    }

    printf("🚦 Client connected. Waiting for BGP packets...\n");

    while (1) {
        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            printf("❌ Client disconnected or error occurred.\n");
            break;
        }
        handle_bgp_packet(buffer, bytes);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

