/* 
 * File:   ethutils.h
 * Author: cqdc5422
 *
 * Created on 19 juillet 2013, 10:03
 */

#ifndef ETHUTILS_H
#define	ETHUTILS_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <net/ethernet.h>
void inverse_eth(struct ether_header *e);


#ifdef	__cplusplus
}
#endif

#endif	/* ETHUTILS_H */

