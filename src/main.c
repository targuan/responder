/*
 * File:   main.c
 * Author: targuan
 *
 * Created on 11 juillet 2013, 21:33
 */

#include <stdio.h>
#include <stdlib.h>
#include<sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>

#include <strings.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>
#include <net/ethernet.h>

#include <netinet/ip.h> // struct iphdr
#include <netinet/ip6.h> // struct ip6_hdr
#include <netinet/udp.h> // struct udphdr *udp;

#define IP_HDR_LEN      (ip->ihl * 4)
#define IP6_HDR_LEN     (sizeof(struct ip6_hdr))
#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6 0x86dd
#endif

struct ip6_pseudo_hdr {
    struct in6_addr src;
    struct in6_addr dst;
    uint16_t len;
    uint16_t nh;
};

/*
 *
 */
void gen_random(char *s, const int len) {
    int i;
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    for (i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof (alphanum) - 1)];
    }

    s[len] = 0;
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

void phex(unsigned char *p, int len) {
    int i = 0;
    while (len--) {
        printf("%.2X ", *p);
        p++;

        if (!((++i) % 16)) {
            printf("\n");
        }
    }
    if ((i % 16)) {
        printf("\n");
    }

}

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

void inverse_eth(struct ether_header *e) {
    struct ether_header t;


    memcpy(&t, e, sizeof (struct ether_header));

    memcpy(e->ether_shost, t.ether_dhost, sizeof (t.ether_dhost));
    memcpy(e->ether_dhost, t.ether_shost, sizeof (t.ether_dhost));
    e->ether_dhost[0] = 0x00;
    e->ether_dhost[1] = 0x00;
    e->ether_dhost[2] = 0x5E;
    e->ether_dhost[3] = 0x00;
    e->ether_dhost[4] = 0x01;
    e->ether_dhost[5] = 0x04;

}

void inverse_udp(struct udphdr *u) {
    int src;
    int len, sll_size;


    src = u->source;
    u->source = u->dest;
    u->dest = src;
}



struct sockaddr_ll sll;
int sll_size;
void read_from(int rawsock) {
    int extra_data_len;
    struct ether_header *eth;
    struct iphdr *ip;
    struct ip6_hdr *ip6;
    struct udphdr *udp;
    static char* packet_buffer = NULL;
    char *dns;
    struct ip6_pseudo_hdr ps_hdr;
    int family;
    int len;
    uint32_t ck;
    
    if(packet_buffer == NULL) packet_buffer = calloc(2048, 1);

    if ((len = recvfrom(rawsock, packet_buffer, 2048, 0, NULL, NULL)) == -1) {
        perror("Recv from returned -1: ");
        exit(-1);
    } else {
        eth = (struct ether_header *) packet_buffer;
        ip = (struct iphdr *) (packet_buffer + ETHER_HDR_LEN);
        ip6 = (struct ip6_hdr *) (packet_buffer + ETHER_HDR_LEN);
        family = ntohs(eth->ether_type);

        if (family == ETHERTYPE_IP) {
            udp = (struct udphdr *) (((void *) ip) + IP_HDR_LEN);
        } else if (family == ETHERTYPE_IPV6) {
            udp = (struct udphdr *) (((void *) ip6) + IP6_HDR_LEN);
        }

        if (ntohs(udp->dest) == 53) {
            dns = (((void *) udp) + sizeof (struct udphdr));

            inverse_udp(udp);
            udp->check = 0;
            dns[2] = dns[2] | 0x80;
            dns[3] = 0x80;
            extra_data_len = rand()%0xff;
            packet_buffer[2] = 0x81;
            packet_buffer[3] = 0x83;
            packet_buffer[11] = 0x01;
            packet_buffer[len] = 0xC0;
            packet_buffer[len+1] = 0x0C;
            packet_buffer[len+2] = 0x00;
            packet_buffer[len+3] = 0x10;
            packet_buffer[len+4] = 0x00;
            packet_buffer[len+5] = 0x01;
            packet_buffer[len+6] = 0x00;
            packet_buffer[len+7] = 0x00;
            packet_buffer[len+8] = 0x00;
            packet_buffer[len+9] = 0xc1;
            packet_buffer[len+10] = 0x00;
            packet_buffer[len+11] = 1+(extra_data_len)&0xff;
            packet_buffer[len+12] = (extra_data_len)&0xff;
            gen_random(&(packet_buffer[len+13]), extra_data_len);
            
            //udp->len = htons(ntohs(udp->len)+12+extra_data_len);
            extra_data_len = 0;
            if (family == ETHERTYPE_IP) {
                inverse_ip(ip);
                udp->check = 0;

            } else if (family == ETHERTYPE_IPV6) {
                inverse_ip6(ip6);
                ps_hdr.src = ip6->ip6_src;
                ps_hdr.dst = ip6->ip6_dst;
                ps_hdr.len = udp->len;
                ps_hdr.nh = ntohs(17);

                ck = inet_cksum(&ps_hdr, sizeof (ps_hdr), 0);
                udp->check = inet_cksum(udp, ntohs(udp->len), (~ck)&0xffff);
            }
            inverse_eth(eth);

            if (sendto(rawsock, packet_buffer, len+12+extra_data_len, 0, (struct sockaddr*) &sll, sizeof (struct sockaddr_ll)) < 0)
                perror("Send failed\n");
        }
    }
}


int main(int argc, char** argv) {
    int rawsock4, rawsock6;
    struct ifreq ifr;
    fd_set readfs;
    int pid;
    int cpu=1;

    if(fork() != 0) {
        exit(0);
    }

    if ((rawsock4 = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("Error creating raw socket 4: ");
        exit(-1);
    }
    if ((rawsock6 = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("Error creating raw socket 6: ");
        exit(-1);
    }

    cpu = atoi(argv[2]);

    bzero(&sll, sizeof (sll));
    bzero(&ifr, sizeof (ifr));

    strncpy((char *) ifr.ifr_name, argv[1], IFNAMSIZ);
    if ((ioctl(rawsock4, SIOCGIFINDEX, &ifr)) == -1) {
        printf("Error getting Interface index !\n");
        exit(-1);
    }

    /* Bind our raw socket to this interface */

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_halen = ETH_ALEN;

    sll.sll_protocol = htons(ETHERTYPE_IP);
    if ((bind(rawsock4, (struct sockaddr *) &sll, sizeof (sll))) == -1) {
        perror("Error binding raw socket to interface\n");
        exit(-1);
    }

    sll.sll_protocol = htons(ETHERTYPE_IPV6);
    if ((bind(rawsock6, (struct sockaddr *) &sll, sizeof (sll))) == -1) {
        perror("Error binding raw socket to interface\n");
        exit(-1);
    }

    while((--cpu)>0) {
	pid = fork();
	if(pid == 0) break;
    }

    FD_ZERO(&readfs);
    
    FD_SET(rawsock4, &readfs);
    FD_SET(rawsock6, &readfs);

    while (1) {
	FD_ZERO(&readfs);

        FD_SET(rawsock4, &readfs);
        FD_SET(rawsock6, &readfs);

        if (select(FD_SETSIZE, &readfs, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(rawsock4, &readfs)) {
            read_from(rawsock4);
        }
        if (FD_ISSET(rawsock6, &readfs)) {
            read_from(rawsock6);
        }
    }

    return (EXIT_SUCCESS);
}

