#include <string.h>
#include "iputils.h"


void inverse_ip(struct iphdr *i) {
    uint32_t t;

    t = i->daddr;
    i->daddr = i->saddr;
    i->saddr = t;
}

void inverse_ip6(struct ip6_hdr *ip6) {
    struct ip6_hdr t;

    memcpy(&t, ip6, sizeof (struct ip6_hdr));
    memcpy(&(ip6->ip6_src), &(t.ip6_dst), sizeof (struct in6_addr));
    memcpy(&(ip6->ip6_dst), &(t.ip6_src), sizeof (struct in6_addr));
}

unsigned short int inet_cksum(unsigned short int *addr, size_t len, int init) {
    register int nleft = (int) len;
    register unsigned short int *w = addr;
    unsigned short int answer = 0;
    register int sum = init;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16); /* add carry */
    answer = ~sum; /* truncate to 16 bits */
    return (answer);
}

