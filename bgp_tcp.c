// bgp_client_packets.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BGP_PORT 179
#define BGP_MARKER_SIZE 16

void send_bgp_open(int sock) {
    unsigned char packet[29] = {0};
    memset(packet, 0xFF, BGP_MARKER_SIZE);      // Marker: all 1s
    packet[16] = 0x00;
    packet[17] = 29;                            // Length = 29 bytes
    packet[18] = 1;                             // Type = OPEN
    packet[19] = 4;                             // Version
    packet[20] = 0xFD; packet[21] = 0xE9;       // My AS = 65001
    packet[22] = 90;                            // Hold Time (90 sec)
    packet[23] = 1;  packet[24] = 1;  packet[25] = 1;  packet[26] = 1; // BGP ID: 1.1.1.1
    packet[27] = 0;                             // Opt Param Len = 0

    send(sock, packet, 29, 0);
    printf("→ Sent BGP OPEN\n");
}

void send_bgp_keepalive(int sock) {
    unsigned char packet[19] = {0};
    memset(packet, 0xFF, BGP_MARKER_SIZE);
    packet[16] = 0x00;
    packet[17] = 19;                            // Length
    packet[18] = 4;                             // Type = KEEPALIVE

    send(sock, packet, 19, 0);
    printf("→ Sent BGP KEEPALIVE\n");
}

void send_bgp_notification(int sock) {
    unsigned char packet[21] = {0};
    memset(packet, 0xFF, BGP_MARKER_SIZE);
    packet[16] = 0x00;
    packet[17] = 21;                            // Length
    packet[18] = 3;                             // Type = NOTIFICATION
    packet[19] = 6;                             // Error Code (Cease)
    packet[20] = 0;                             // Subcode

    send(sock, packet, 21, 0);
    printf("→ Sent BGP NOTIFICATION (Cease)\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <BGP Server IP>\n", argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(BGP_PORT);
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    printf("Connecting to %s:5000...\n", argv[1]);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("✓ Connected to BGP server.\n");

    send_bgp_open(sock);
    sleep(1);

    send_bgp_keepalive(sock);
    sleep(1);

    send_bgp_notification(sock);  // End session politely
    sleep(1);

    close(sock);
    return 0;
}

