#include "IPv4Validator.h"

#include <stddef.h>
#include <string.h>

/* Converts a dotted-decimal address into a 32-bit integer. */
static bool _parseIPv4Address(
    const char * address,
    uint32_t * result
) {
    if (address == NULL || result == NULL) {
        return false;
    }

    uint32_t value = 0;
    const char * current = address;

    for (int octet = 0; octet < 4; ++octet) {
        if (*current < '0' || *current > '9') {
            return false;
        }

        unsigned int number = 0;
        unsigned int digits = 0;

        while (*current >= '0' && *current <= '9') {
            number = number * 10 + (unsigned int)(*current - '0');

            if (number > 255 || ++digits > 3) {
                return false;
            }

            ++current;
        }

        value = (value << 8) | number;

        if (octet < 3) {
            if (*current != '.') {
                return false;
            }

            ++current;
        }
        else if (*current != '\0') {
            return false;
        }
    }

    *result = value;
    return true;
}

bool isValidIPv4Address(const char * address) {
    uint32_t value;
    return _parseIPv4Address(address, &value);
}

/* Converts a subnet mask into its prefix length. */
static bool _parseSubnetMask(
    uint32_t mask,
    unsigned int * prefix
) {
    bool foundZero = false;
    unsigned int count = 0;

    for (int bit = 31; bit >= 0; --bit) {
        bool currentBit = (mask & ((uint32_t)1 << bit)) != 0;

        if (currentBit) {
            if (foundZero) {
                return false;
            }

            ++count;
        }
        else {
            foundZero = true;
        }
    }

    *prefix = count;
    return true;
}

/* Converts a prefix length into a 32-bit subnet mask. */
static uint32_t _prefixToMask(unsigned int prefix) {
    if (prefix == 0) {
        return 0;
    }

    return UINT32_MAX << (32 - prefix);
}

bool parseIPv4Cidr(
    const char * cidr,
    IPv4NetworkInfo * result
) {
    if (cidr == NULL || result == NULL) {
        return false;
    }

    const char * separator = strchr(cidr, '/');

    if (separator == NULL || separator == cidr || separator[1] == '\0') {
        return false;
    }

    size_t addressLength = (size_t)(separator - cidr);

    char address[16];

    if (addressLength >= sizeof(address)) {
        return false;
    }

    memcpy(address, cidr, addressLength);
    address[addressLength] = '\0';

    uint32_t addressValue;

    if (!_parseIPv4Address(address, &addressValue)) {
        return false;
    }

    const char * suffix = separator + 1;

    unsigned int prefix = 0;
    uint32_t mask = 0;

    bool numericPrefix = true;

    for (const char * current = suffix; *current != '\0'; ++current) {
        if (*current < '0' || *current > '9') {
            numericPrefix = false;
            break;
        }
    }

    if (numericPrefix) {
        if (*suffix == '\0') {
            return false;
        }

        for (const char * current = suffix; *current != '\0'; ++current) {
            prefix = prefix * 10 + (unsigned int)(*current - '0');

            if (prefix > 32) {
                return false;
            }
        }

        mask = _prefixToMask(prefix);
    }
    else {
        if (!_parseIPv4Address(suffix, &mask)) {
            return false;
        }

        if (!_parseSubnetMask(mask, &prefix)) {
            return false;
        }
    }

    result->address = addressValue;
    result->mask = mask;
    result->prefix = prefix;

    return true;
}

bool isIPv4AddressInNetwork(
    const char * address,
    const char * networkCidr
) {
    uint32_t addressValue;
    IPv4NetworkInfo network;

    if (
        !_parseIPv4Address(address, &addressValue) ||
        !parseIPv4Cidr(networkCidr, &network)
    ) {
        return false;
    }

    return (addressValue & network.mask) ==
           (network.address & network.mask);
}

bool haveSameIPv4Mask(
    const char * firstCidr,
    const char * secondCidr
) {
    IPv4NetworkInfo first;
    IPv4NetworkInfo second;

    if (
        !parseIPv4Cidr(firstCidr, &first) ||
        !parseIPv4Cidr(secondCidr, &second)
    ) {
        return false;
    }

    return first.mask == second.mask;
}