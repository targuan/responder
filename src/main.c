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

/*
 * 
 */

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

int main(int argc, char** argv) {
    int rawsock;
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int len, sll_size;
    void* packet_buffer;

    int family;

    struct ether_header *eth;
    struct iphdr *ip;
    struct ip6_hdr *ip6;
    struct udphdr *udp;

    packet_buffer = calloc(2048,1);


    if ((rawsock = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("Error creating raw socket: ");
        exit(-1);
    }

    bzero(&sll, sizeof (sll));
    bzero(&ifr, sizeof (ifr));

    strncpy((char *) ifr.ifr_name, "eth2", IFNAMSIZ);
    if ((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        printf("Error getting Interface index !\n");
        exit(-1);
    }

    /* Bind our raw socket to this interface */

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETHERTYPE_IP);


    if ((bind(rawsock, (struct sockaddr *) &sll, sizeof (sll))) == -1) {
        perror("Error binding raw socket to interface\n");
        exit(-1);
    }

    while (1) {
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
                printf("Received %d\n", ntohs(udp->dest));
                phex(packet_buffer, len);
                break;
            }
        }
    }

    return (EXIT_SUCCESS);
}

