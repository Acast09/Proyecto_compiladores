#ifndef IPV4_VALIDATOR_HEADER
#define IPV4_VALIDATOR_HEADER

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t address;
    uint32_t mask;
    unsigned int prefix;
} IPv4NetworkInfo;

/* Validates a dotted-decimal IPv4 address. */
bool isValidIPv4Address(const char * address);

/*
 * Accepts both:
 *   192.168.1.0/24
 *   192.168.1.0/255.255.255.0
 */
bool parseIPv4Cidr(
    const char * cidr,
    IPv4NetworkInfo * result
);

/* Checks whether an address belongs to a network. */
bool isIPv4AddressInNetwork(
    const char * address,
    const char * networkCidr
);

/* Checks whether two CIDR configurations use the same mask. */
bool haveSameIPv4Mask(
    const char * firstCidr,
    const char * secondCidr
);

#endif