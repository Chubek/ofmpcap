#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pcre2posix.h>
#include <linux/if_link.h>

static struct 
ifaddrs* ifaddr;

static int 
family, s;

static char 
host[NI_MAXHOST];

static inline 
void
get_interface_addrs(void)
{ 
	if (getifaddrs(&ifaddr) < 0)
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
}

static inline 
struct ifaddrs*
select_interface_by_name(const char* ifa_name_pattern)
{
	struct ifaddrs* ifa;
	regex_t* preg;
	int cmp_res;
	char errbuff[ERRBUFF_SIZE + 1] = {0};

	if ((cmp_res = pcre2posix(preg, ifa_name_pattern, REG_NOSUB)) <  0)
	{
		pcre2_regerror(cmp_res, preg, &errbuff[0], ERRBUFF_SIZE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->next)
	{
		if (pcre2_regexec(preg, ifa->ifa_name, 0, NULL, 0))
			break;
	}

	regfree(preg);
	return ifa;
}

int main(int argc, char* argv[]) {

    for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        printf("%-8s %s (%d)\n",
               ifa->ifa_name,
               (family == AF_PACKET) ? "AF_PACKET" :
               (family == AF_INET) ? "AF_INET" :
               (family == AF_INET6) ? "AF_INET6" : "???",
               family);

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST,
                            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\t\taddress: <%s>\n", host);
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats* stats = ifa->ifa_data;

            printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                   "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);
}

