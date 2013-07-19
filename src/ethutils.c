#include "ethutils.h"
#include <unistd.h>
#include <string.h>


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
