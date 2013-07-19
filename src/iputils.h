/* 
 * File:   iputils.h
 * Author: cqdc5422
 *
 * Created on 19 juillet 2013, 09:50
 */

#ifndef IPUTILS_H
#define	IPUTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <netinet/ip.h> // struct iphdr
#include <netinet/ip6.h> // struct ip6_hdr
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
void inverse_ip(struct iphdr *i);
void inverse_ip6(struct ip6_hdr *ip6);
unsigned short int inet_cksum(unsigned short int *addr, size_t len, int init);


#ifdef	__cplusplus
}
#endif

#endif	/* IPUTILS_H */

