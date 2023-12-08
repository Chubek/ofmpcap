#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#define ARP_REQUEST 1
#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IPV4 0x0800

void sendArpRequest(const char *interface, const char *targetIp) {
    int sockfd;
    struct sockaddr_ll saddr;
    struct ether_header *eth_header;
    struct ether_arp *arp_packet;
    char frame[1500];  // Maximum Ethernet frame size

    // Open a raw socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETHERNET_TYPE_ARP))) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Get the index of the network interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("Error getting interface index");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Construct Ethernet header
    eth_header = (struct ether_header *)frame;
    memset(eth_header->ether_dhost, 0xFF, 6);  // Broadcast MAC address
    memset(eth_header->ether_shost, 0x00, 6);  // Source MAC address (can be retrieved dynamically)
    eth_header->ether_type = htons(ETHERNET_TYPE_ARP);

    // Construct ARP request
    arp_packet = (struct ether_arp *)(frame + sizeof(struct ether_header));
    arp_packet->arp_hrd = htons(ARPHRD_ETHER);
    arp_packet->arp_pro = htons(ETHERNET_TYPE_IPV4);
    arp_packet->arp_hln = 6;  // Hardware address length (MAC)
    arp_packet->arp_pln = 4;  // Protocol address length (IPv4)
    arp_packet->arp_op = htons(ARP_REQUEST);

    // Set sender's MAC and IP address
    memset(arp_packet->arp_sha, 0x00, 6);  // Source MAC address (can be retrieved dynamically)
    inet_pton(AF_INET, "192.168.1.1", arp_packet->arp_spa);  // Source IP address

    // Set target's MAC and IP address
    memset(arp_packet->arp_tha, 0x00, 6);  // Target MAC address (unknown)
    inet_pton(AF_INET, targetIp, arp_packet->arp_tpa);  // Target IP address

    // Set the network interface index
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETHERNET_TYPE_ARP);
    saddr.sll_ifindex = ifr.ifr_ifindex;

    // Send the ARP request
    if (sendto(sockfd, frame, sizeof(struct ether_header) + sizeof(struct ether_arp), 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr_ll)) == -1) {
        perror("Error sending ARP request");
    } else {
        printf("ARP request sent successfully.\n");
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <target-ip>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sendArpRequest(argv[1], argv[2]);

    return EXIT_SUCCESS;
}

