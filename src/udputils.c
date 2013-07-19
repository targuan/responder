#include "udputils.h"
#include <stdlib.h>

void inverse_udp(struct udphdr *u) {
    int src;


    src = u->source;
    u->source = u->dest;
    u->dest = src;
}

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
