#include <stdio.h>
#include <pcap.h>

void packet_handler(unsigned char *user, const struct pcap_pkthdr *pkthdr, const unsigned char *packet) {
    printf("Packet captured. Length: %d\n", pkthdr->len);
}

int main(int argc, char *argv[]) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Open the network interface for packet capture
    handle = pcap_open_live("en0", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device: %s\n", errbuf);
        return 1;
    }

    // Set a filter to capture only TCP packets (you can customize this filter)
    struct bpf_program fp;
    char filter_exp[] = "tcp";
    bpf_u_int32 mask;
    bpf_u_int32 net;

    if (pcap_lookupnet("en0", &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Could not get netmask for device: %s\n", errbuf);
        net = 0;
        mask = 0;
    }

    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Could not parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Could not install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    // Start capturing packets
    pcap_loop(handle, 0, packet_handler, NULL);

    // Close the handle when done
    pcap_close(handle);

    return 0;
}

