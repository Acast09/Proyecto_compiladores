#include "BisonActions.h"

#include <string.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

void _shutdownBisonActionsModule() {
    if (_logger != NULL) {
        logDebugging(
            _logger,
            "Destroying module: BisonActions..."
        );

        destroyLogger(_logger);
        _logger = NULL;
    }

    _compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(
    CompilerState * compilerState
) {
    _compilerState = compilerState;
    _logger = createLogger("BisonActions");

    return _shutdownBisonActionsModule;
}

/* LOGGING */

static void _logSyntacticAnalyzerAction(
    const char * actionName
) {
    logDebugging(
        _logger,
        "Executing semantic action: %s",
        actionName
    );
}

/* STRING */

char * StringSemanticAction(
    const char * value
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (value == NULL) {
        return NULL;
    }

    size_t length = strlen(value);

    char * string =
        calloc(
            length + 1,
            sizeof(char)
        );

    if (string != NULL) {
        strcpy(string, value);
    }

    return string;
}

/* APPEND HELPERS */

static Network * _appendNetwork(
    Network * list,
    Network * network
) {
    if (network == NULL) {
        return list;
    }

    if (list == NULL) {
        return network;
    }

    Network * current = list;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = network;

    return list;
}

static Interface * _appendInterface(
    Interface * list,
    Interface * interface
) {
    if (interface == NULL) {
        return list;
    }

    if (list == NULL) {
        return interface;
    }

    Interface * current = list;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = interface;

    return list;
}

static Route * _appendRoute(
    Route * list,
    Route * route
) {
    if (route == NULL) {
        return list;
    }

    if (list == NULL) {
        return route;
    }

    Route * current = list;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = route;

    return list;
}

static Device * _appendDevice(
    Device * list,
    Device * device
) {
    if (device == NULL) {
        return list;
    }

    if (list == NULL) {
        return device;
    }

    Device * current = list;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = device;

    return list;
}

static Connection * _appendConnection(
    Connection * list,
    Connection * connection
) {
    if (connection == NULL) {
        return list;
    }

    if (list == NULL) {
        return connection;
    }

    Connection * current = list;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = connection;

    return list;
}

/* NETWORK */

Network * NetworkSemanticAction(
    char * name,
    char * cidr
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Network * network =
        calloc(
            1,
            sizeof(Network)
        );

    if (network != NULL) {
        network->name = name;
        network->cidr = cidr;
        network->next = NULL;
    }

    return network;
}

/* INTERFACE */

Interface * InterfaceWithCidrSemanticAction(
    char * name,
    char * cidr,
    char * network
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Interface * interface =
        calloc(
            1,
            sizeof(Interface)
        );

    if (interface != NULL) {
        interface->name = name;
        interface->cidr = cidr;
        interface->network = network;
        interface->next = NULL;
    }

    return interface;
}

Interface * InterfaceWithoutCidrSemanticAction(
    char * name,
    char * network
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Interface * interface =
        calloc(
            1,
            sizeof(Interface)
        );

    if (interface != NULL) {
        interface->name = name;
        interface->cidr = NULL;
        interface->network = network;
        interface->next = NULL;
    }

    return interface;
}

Interface * InterfaceListSemanticAction(
    Interface * list,
    Interface * interface
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    return _appendInterface(
        list,
        interface
    );
}

/* ROUTE */

Route * RouteSemanticAction(
    char * destination,
    char * nextHop
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Route * route =
        calloc(
            1,
            sizeof(Route)
        );

    if (route != NULL) {
        route->destination = destination;
        route->nextHop = nextHop;
        route->next = NULL;
    }

    return route;
}

/* DEVICE ELEMENT ACCUMULATOR */

Device * EmptyDeviceSemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    return calloc(
        1,
        sizeof(Device)
    );
}

Device * InterfaceDeviceElementSemanticAction(
    Interface * interface
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Device * device =
        EmptyDeviceSemanticAction();

    if (device != NULL) {
        device->interfaces = interface;
    }

    return device;
}

Device * RouteDeviceElementSemanticAction(
    Route * route
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Device * device =
        EmptyDeviceSemanticAction();

    if (device != NULL) {
        device->routes = route;
    }

    return device;
}

Device * MergeDeviceSemanticAction(
    Device * device,
    Device * element
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (device == NULL) {
        return element;
    }

    if (element == NULL) {
        return device;
    }

    device->interfaces =
        _appendInterface(
            device->interfaces,
            element->interfaces
        );

    device->routes =
        _appendRoute(
            device->routes,
            element->routes
        );

    element->interfaces = NULL;
    element->routes = NULL;

    free(element);

    return device;
}

/* DEVICES */

Device * RouterSemanticAction(
    char * name,
    Device * elements
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (elements == NULL) {
        elements = EmptyDeviceSemanticAction();
    }

    if (elements != NULL) {
        elements->type = DEVICE_ROUTER;
        elements->name = name;
        elements->next = NULL;
    }

    return elements;
}

Device * SwitchSemanticAction(
    char * name,
    Interface * interfaces
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Device * device =
        calloc(
            1,
            sizeof(Device)
        );

    if (device != NULL) {
        device->type = DEVICE_SWITCH;
        device->name = name;
        device->interfaces = interfaces;
        device->routes = NULL;
        device->next = NULL;
    }

    return device;
}

Device * HostSemanticAction(
    char * name,
    Interface * interfaces
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Device * device =
        calloc(
            1,
            sizeof(Device)
        );

    if (device != NULL) {
        device->type = DEVICE_HOST;
        device->name = name;
        device->interfaces = interfaces;
        device->routes = NULL;
        device->next = NULL;
    }

    return device;
}

/* ENDPOINT / CONNECTION */

Endpoint * EndpointSemanticAction(
    char * device,
    char * interface
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Endpoint * endpoint =
        calloc(
            1,
            sizeof(Endpoint)
        );

    if (endpoint != NULL) {
        endpoint->device = device;
        endpoint->interface = interface;
    }

    return endpoint;
}

Connection * ConnectionSemanticAction(
    Endpoint * source,
    Endpoint * target
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Connection * connection =
        calloc(
            1,
            sizeof(Connection)
        );

    if (connection != NULL) {
        connection->source = source;
        connection->target = target;
        connection->next = NULL;
    }

    return connection;
}

/* TOPOLOGY ACCUMULATOR */

Topology * EmptyTopologySemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    return calloc(
        1,
        sizeof(Topology)
    );
}

Topology * NetworkTopologyElementSemanticAction(
    Network * network
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Topology * topology =
        EmptyTopologySemanticAction();

    if (topology != NULL) {
        topology->networks = network;
    }

    return topology;
}

Topology * DeviceTopologyElementSemanticAction(
    Device * device
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Topology * topology =
        EmptyTopologySemanticAction();

    if (topology != NULL) {
        topology->devices = device;
    }

    return topology;
}

Topology * ConnectionTopologyElementSemanticAction(
    Connection * connection
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Topology * topology =
        EmptyTopologySemanticAction();

    if (topology != NULL) {
        topology->connections = connection;
    }

    return topology;
}

Topology * MergeTopologySemanticAction(
    Topology * topology,
    Topology * element
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (topology == NULL) {
        return element;
    }

    if (element == NULL) {
        return topology;
    }

    topology->networks =
        _appendNetwork(
            topology->networks,
            element->networks
        );

    topology->devices =
        _appendDevice(
            topology->devices,
            element->devices
        );

    topology->connections =
        _appendConnection(
            topology->connections,
            element->connections
        );

    element->networks = NULL;
    element->devices = NULL;
    element->connections = NULL;

    free(element);

    return topology;
}

/* TOPOLOGY */

Topology * TopologySemanticAction(
    char * name,
    Topology * elements
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (elements == NULL) {
        elements = EmptyTopologySemanticAction();
    }

    if (elements != NULL) {
        elements->name = name;
    }

    return elements;
}

/* PROGRAM */

Program * TopologyProgramSemanticAction(
    Topology * topology
) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Program * program =
        calloc(
            1,
            sizeof(Program)
        );

    if (program != NULL) {
        program->topology = topology;

        if (_compilerState != NULL) {
            _compilerState->abstractSyntaxtTree = program;
        }
    }

    return program;
}