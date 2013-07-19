/* 
 * File:   udputils.h
 * Author: cqdc5422
 *
 * Created on 19 juillet 2013, 09:49
 */

#ifndef UDPUTILS_H
#define	UDPUTILS_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <netinet/udp.h>
void inverse_udp(struct udphdr *u);
void gen_random(char *s, const int len);

#ifdef	__cplusplus
}
#endif

#endif	/* UDPUTILS_H */

