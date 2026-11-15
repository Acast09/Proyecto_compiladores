
#include "SemanticAnalyzer.h"
#include "IPv4Validator.h"

#include "../../support/logging/Logger.h"

#include <stdbool.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;
static unsigned int _errors = 0;

/* PRIVATE FUNCTIONS */

static void _reportError(const char * message) {
    ++_errors;
    logError(_logger, "%s", message);
}

static void _reportWarning(const char * message) {
    logWarning(_logger, "%s", message);
}

static bool _hasDuplicateNetworkName(
    const Network * network
) {
    for (
        const Network * current = network;
        current != NULL;
        current = current->next
    ) {
        for (
            const Network * other = current->next;
            other != NULL;
            other = other->next
        ) {
            if (strcmp(current->name, other->name) == 0) {
                return true;
            }
        }
    }

    return false;
}

static bool _hasDuplicateDeviceName(
    const Device * device
) {
    for (
        const Device * current = device;
        current != NULL;
        current = current->next
    ) {
        for (
            const Device * other = current->next;
            other != NULL;
            other = other->next
        ) {
            if (strcmp(current->name, other->name) == 0) {
                return true;
            }
        }
    }

    return false;
}

static bool _hasDuplicateInterfaceName(
    const Interface * interface
) {
    for (
        const Interface * current = interface;
        current != NULL;
        current = current->next
    ) {
        for (
            const Interface * other = current->next;
            other != NULL;
            other = other->next
        ) {
            if (strcmp(current->name, other->name) == 0) {
                return true;
            }
        }
    }

    return false;
}

static unsigned int _countConnections(
    const Topology * topology,
    const char * deviceName,
    const char * interfaceName
) {
    unsigned int count = 0;

    for (
        const Connection * connection = topology->connections;
        connection != NULL;
        connection = connection->next
    ) {
        if (
            connection->source != NULL &&
            strcmp(connection->source->device, deviceName) == 0 &&
            strcmp(connection->source->interface, interfaceName) == 0
        ) {
            ++count;
        }

        if (
            connection->target != NULL &&
            strcmp(connection->target->device, deviceName) == 0 &&
            strcmp(connection->target->interface, interfaceName) == 0
        ) {
            ++count;
        }
    }

    return count;
}

static bool _deviceHasConnections(
    const Topology * topology,
    const char * deviceName
) {
    for (
        const Connection * connection = topology->connections;
        connection != NULL;
        connection = connection->next
    ) {
        if (
            (connection->source != NULL &&
             strcmp(connection->source->device, deviceName) == 0) ||
            (connection->target != NULL &&
             strcmp(connection->target->device, deviceName) == 0)
        ) {
            return true;
        }
    }

    return false;
}

static bool _networkHasInterfaces(
    const Topology * topology,
    const char * networkName
) {
    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        for (
            const Interface * interface = device->interfaces;
            interface != NULL;
            interface = interface->next
        ) {
            if (
                interface->network != NULL &&
                strcmp(interface->network, networkName) == 0
            ) {
                return true;
            }
        }
    }

    return false;
}

/* MODULE LIFECYCLE */

static void _shutdownSemanticAnalyzerModule(void) {
    if (_logger != NULL) {
        logDebugging(
            _logger,
            "Destroying module: SemanticAnalyzer..."
        );

        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeSemanticAnalyzerModule(void) {
    _logger = createLogger("SemanticAnalyzer");

    return _shutdownSemanticAnalyzerModule;
}

/* PUBLIC FUNCTIONS */

CompilationStatus analyzeSemantics(
    const Topology * topology,
    const SymbolTable * symbolTable
) {
    if (topology == NULL || symbolTable == NULL) {
        logError(
            _logger,
            "Cannot perform semantic analysis: invalid input."
        );

        return FAILED;
    }

    _errors = 0;

    logInformation(
        _logger,
        "Starting semantic analysis for topology \"%s\".",
        topology->name
    );

    /* 1. Network identifiers */

    if (_hasDuplicateNetworkName(topology->networks)) {
        _reportError(
            "Duplicate network identifier found."
        );
    }

    /* 1.1. Network IPv4 configurations */

    for (
        const Network * network = topology->networks;
        network != NULL;
        network = network->next
    ) {
        IPv4NetworkInfo networkInfo;

        if (!parseIPv4Cidr(network->cidr, &networkInfo)) {
            logError(
                _logger,
                "Network \"%s\" has an invalid IPv4 configuration: \"%s\".",
                network->name,
                network->cidr != NULL ? network->cidr : "(null)"
            );

            ++_errors;
        }
    }

    /* 2. Device identifiers */

    if (_hasDuplicateDeviceName(topology->devices)) {
        _reportError(
            "Duplicate device identifier found."
        );
    }

    /* 3. Interface identifiers and network references */

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        if (_hasDuplicateInterfaceName(device->interfaces)) {
            logError(
                _logger,
                "Device \"%s\" contains duplicate interface identifiers.",
                device->name
            );
            ++_errors;
        }

        for (
            const Interface * interface = device->interfaces;
            interface != NULL;
            interface = interface->next
        ) {
            const Network * network = NULL;

    if (interface->network != NULL) {
        network = findNetwork(
            symbolTable,
            interface->network
        );

        if (network == NULL) {
            logError(
                _logger,
                "Interface \"%s\" of device \"%s\" references "
                "undefined network \"%s\".",
                interface->name,
                device->name,
                interface->network
            );

            ++_errors;
        }
    }

    if (interface->cidr != NULL) {
        IPv4NetworkInfo interfaceInfo;

        if (!parseIPv4Cidr(interface->cidr, &interfaceInfo)) {
            logError(
                _logger,
                "Interface \"%s\" of device \"%s\" has an invalid "
                "IPv4 configuration: \"%s\".",
                interface->name,
                device->name,
                interface->cidr
            );

            ++_errors;
        }
        else if (network != NULL) {
            IPv4NetworkInfo networkInfo;

            if (parseIPv4Cidr(network->cidr, &networkInfo)) {

                /* Validate that both configurations use the same subnet mask. */

                if (interfaceInfo.mask != networkInfo.mask) {
                    logError(
                        _logger,
                        "Interface \"%s\" on device \"%s\" uses a subnet mask "
                        "that does not match network \"%s\".",
                        interface->name,
                        device->name,
                        network->name
                    );

                    ++_errors;
                }

                /* Validate that the interface IP belongs to the declared network. */

                if (
                    (interfaceInfo.address & networkInfo.mask) !=
                    (networkInfo.address & networkInfo.mask)
                ) {
                    logError(
                        _logger,
                        "IP address of interface \"%s\" on device \"%s\" "
                        "does not belong to network \"%s\".",
                        interface->name,
                        device->name,
                        network->name
                    );

                    ++_errors;
                }
            }
        }
    }
    else if (
        device->type == DEVICE_ROUTER ||
        device->type == DEVICE_HOST
    ) {
        logWarning(
            _logger,
            "Interface \"%s\" of device \"%s\" has no IP address.",
            interface->name,
            device->name
        );
    }
        }
}

    /* 4. Static routes */

for (
    const Device * device = topology->devices;
    device != NULL;
    device = device->next
) {
    if (
        device->type != DEVICE_ROUTER &&
        device->routes != NULL
    ) {
        logError(
            _logger,
            "Device \"%s\" is not a router but defines static routes.",
            device->name
        );

        ++_errors;
    }

    if (
        device->type == DEVICE_ROUTER &&
        device->routes == NULL
    ) {
        logWarning(
            _logger,
            "Router \"%s\" has no static routes configured.",
            device->name
        );
    }

    /* Validate static routes */

    for (
        const Route * route = device->routes;
        route != NULL;
        route = route->next
    ) {
        IPv4NetworkInfo destinationInfo;

        if (!parseIPv4Cidr(route->destination, &destinationInfo)) {
            logError(
                _logger,
                "Router \"%s\" has an invalid route destination: \"%s\".",
                device->name,
                route->destination != NULL
                    ? route->destination
                    : "(null)"
            );

            ++_errors;
        }

        if (!isValidIPv4Address(route->nextHop)) {
            logError(
                _logger,
                "Router \"%s\" has an invalid next hop: \"%s\".",
                device->name,
                route->nextHop != NULL
                    ? route->nextHop
                    : "(null)"
            );

            ++_errors;
        }
    }
}

    /* 5. Physical connections */

    for (
        const Connection * connection = topology->connections;
        connection != NULL;
        connection = connection->next
    ) {
        if (
            connection->source == NULL ||
            connection->target == NULL
        ) {
            _reportError(
                "A connection contains an incomplete endpoint."
            );
            continue;
        }

        const Endpoint * endpoints[] = {
            connection->source,
            connection->target
        };

        for (int index = 0; index < 2; ++index) {
            const Endpoint * endpoint = endpoints[index];

            const Device * device =
                findDevice(symbolTable, endpoint->device);

            if (device == NULL) {
                logError(
                    _logger,
                    "Connection references undefined device \"%s\".",
                    endpoint->device
                );
                ++_errors;
                continue;
            }

            if (
                findInterface(
                    symbolTable,
                    endpoint->device,
                    endpoint->interface
                ) == NULL
            ) {
                logError(
                    _logger,
                    "Connection references undefined interface \"%s\" "
                    "on device \"%s\".",
                    endpoint->interface,
                    endpoint->device
                );
                ++_errors;
                continue;
            }

            if (
                _countConnections(
                    topology,
                    endpoint->device,
                    endpoint->interface
                ) > 1
            ) {
                logError(
                    _logger,
                    "Interface \"%s\" of device \"%s\" has more than "
                    "one physical connection.",
                    endpoint->interface,
                    endpoint->device
                );
                ++_errors;
            }
        }

        if (
            strcmp(
                connection->source->device,
                connection->target->device
            ) == 0 &&
            strcmp(
                connection->source->interface,
                connection->target->interface
            ) == 0
        ) {
            _reportError(
                "A connection cannot link an interface to itself."
            );
        }
    }

    /* 6. Warnings for unused elements */

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        if (!_deviceHasConnections(topology, device->name)) {
            logWarning(
                _logger,
                "Device \"%s\" has no physical connections.",
                device->name
            );
        }
    }

    for (
        const Network * network = topology->networks;
        network != NULL;
        network = network->next
    ) {
        if (!_networkHasInterfaces(topology, network->name)) {
            logWarning(
                _logger,
                "Network \"%s\" has no associated interfaces.",
                network->name
            );
        }
    }

    if (_errors > 0) {
        logError(
            _logger,
            "Semantic analysis failed with %u error(s).",
            _errors
        );

        return FAILED;
    }

    logInformation(
        _logger,
        "Semantic analysis completed successfully."
    );

    return SUCCEEDED;
}