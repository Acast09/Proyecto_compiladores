
#include "SymbolTable.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


/* PRIVATE FUNCTIONS */

static bool _appendNetworkSymbol(
    SymbolTable * symbolTable,
    const Network * network
) {
    NetworkSymbol * symbol =
        calloc(1, sizeof(NetworkSymbol));

    if (symbol == NULL) {
        return false;
    }

    symbol->network = network;

    NetworkSymbol ** current =
        &symbolTable->networks;

    while (*current != NULL) {
        current = &(*current)->next;
    }

    *current = symbol;

    return true;
}

static bool _appendDeviceSymbol(
    SymbolTable * symbolTable,
    const Device * device
) {
    DeviceSymbol * symbol =
        calloc(1, sizeof(DeviceSymbol));

    if (symbol == NULL) {
        return false;
    }

    symbol->device = device;

    DeviceSymbol ** current =
        &symbolTable->devices;

    while (*current != NULL) {
        current = &(*current)->next;
    }

    *current = symbol;

    return true;
}

static bool _appendInterfaceSymbol(
    SymbolTable * symbolTable,
    const Device * device,
    const Interface * interface
) {
    InterfaceSymbol * symbol =
        calloc(1, sizeof(InterfaceSymbol));

    if (symbol == NULL) {
        return false;
    }

    symbol->device = device;
    symbol->interface = interface;

    InterfaceSymbol ** current =
        &symbolTable->interfaces;

    while (*current != NULL) {
        current = &(*current)->next;
    }

    *current = symbol;

    return true;
}

/* PUBLIC FUNCTIONS */

SymbolTable * createSymbolTable(
    const Topology * topology
) {
    if (topology == NULL) {
        return NULL;
    }

    SymbolTable * symbolTable =
        calloc(1, sizeof(SymbolTable));

    if (symbolTable == NULL) {
        return NULL;
    }

    for (
        const Network * network = topology->networks;
        network != NULL;
        network = network->next
    ) {
        if (!_appendNetworkSymbol(symbolTable, network)) {
            destroySymbolTable(symbolTable);
            return NULL;
        }
    }

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        if (!_appendDeviceSymbol(symbolTable, device)) {
            destroySymbolTable(symbolTable);
            return NULL;
        }

        for (
            const Interface * interface = device->interfaces;
            interface != NULL;
            interface = interface->next
        ) {
            if (!_appendInterfaceSymbol(
                symbolTable,
                device,
                interface
            )) {
                destroySymbolTable(symbolTable);
                return NULL;
            }
        }
    }

    return symbolTable;
}

void destroySymbolTable(SymbolTable * symbolTable) {
    if (symbolTable == NULL) {
        return;
    }

    while (symbolTable->networks != NULL) {
        NetworkSymbol * next =
            symbolTable->networks->next;

        free(symbolTable->networks);
        symbolTable->networks = next;
    }

    while (symbolTable->devices != NULL) {
        DeviceSymbol * next =
            symbolTable->devices->next;

        free(symbolTable->devices);
        symbolTable->devices = next;
    }

    while (symbolTable->interfaces != NULL) {
        InterfaceSymbol * next =
            symbolTable->interfaces->next;

        free(symbolTable->interfaces);
        symbolTable->interfaces = next;
    }

    free(symbolTable);
}

const Network * findNetwork(
    const SymbolTable * symbolTable,
    const char * name
) {
    if (symbolTable == NULL || name == NULL) {
        return NULL;
    }

    for (
        NetworkSymbol * symbol = symbolTable->networks;
        symbol != NULL;
        symbol = symbol->next
    ) {
        if (strcmp(symbol->network->name, name) == 0) {
            return symbol->network;
        }
    }

    return NULL;
}

const Device * findDevice(
    const SymbolTable * symbolTable,
    const char * name
) {
    if (symbolTable == NULL || name == NULL) {
        return NULL;
    }

    for (
        DeviceSymbol * symbol = symbolTable->devices;
        symbol != NULL;
        symbol = symbol->next
    ) {
        if (strcmp(symbol->device->name, name) == 0) {
            return symbol->device;
        }
    }

    return NULL;
}

const Interface * findInterface(
    const SymbolTable * symbolTable,
    const char * deviceName,
    const char * interfaceName
) {
    if (
        symbolTable == NULL ||
        deviceName == NULL ||
        interfaceName == NULL
    ) {
        return NULL;
    }

    for (
        InterfaceSymbol * symbol = symbolTable->interfaces;
        symbol != NULL;
        symbol = symbol->next
    ) {
        if (
            strcmp(symbol->device->name, deviceName) == 0 &&
            strcmp(symbol->interface->name, interfaceName) == 0
        ) {
            return symbol->interface;
        }
    }

    return NULL;
}