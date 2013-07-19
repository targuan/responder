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

// fork
#include <sys/types.h>
#include <unistd.h>


#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>

#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>
#include <net/ethernet.h>

#include <netinet/ip.h> // struct iphdr
#include <netinet/ip6.h> // struct ip6_hdr
#include <netinet/udp.h> // struct udphdr *udp;

#include "iputils.h"
#include "udputils.h"
#include "ethutils.h"
#include "dnsutils.h"

/*
 *
 */





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
            dns[2] = 0x81;
            dns[3] = 0x83;
            dns[11] = 0x01;
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
            packet_buffer[len+11] = 1+((extra_data_len)&0xff);
            packet_buffer[len+12] = (extra_data_len)&0xff;
            gen_random(&(packet_buffer[len+13]), extra_data_len);
            
            udp->len = htons(ntohs(udp->len)+12+extra_data_len);
            //extra_data_len = 0;
            if (family == ETHERTYPE_IP) {
                inverse_ip(ip);
                udp->check = 0;

            } else if (family == ETHERTYPE_IPV6) {
                inverse_ip6(ip6);
                ps_hdr.src = ip6->ip6_src;
                ps_hdr.dst = ip6->ip6_dst;
                ps_hdr.len = udp->len;
                ps_hdr.nh = ntohs(17);

                ck = inet_cksum((unsigned short *)&ps_hdr, sizeof (ps_hdr), 0);
                udp->check = inet_cksum((unsigned short *)udp, ntohs(udp->len), (~ck)&0xffff);
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

