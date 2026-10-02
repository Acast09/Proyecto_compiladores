#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownAbstractSyntaxTreeModule() {
    if (_logger != NULL) {
        logDebugging(
            _logger,
            "Destroying module: AbstractSyntaxTree..."
        );

        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
    _logger = createLogger("AbstractSyntaxTree");

    return _shutdownAbstractSyntaxTreeModule;
}

void destroyNetwork(Network * network) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    while (network != NULL) {
        Network * next = network->next;

        free(network->name);
        free(network->cidr);
        free(network);

        network = next;
    }
}

void destroyInterface(Interface * interface) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    while (interface != NULL) {
        Interface * next = interface->next;

        free(interface->name);
        free(interface->cidr);
        free(interface->network);
        free(interface);

        interface = next;
    }
}

void destroyRoute(Route * route) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    while (route != NULL) {
        Route * next = route->next;

        free(route->destination);
        free(route->nextHop);
        free(route);

        route = next;
    }
}

void destroyDevice(Device * device) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    while (device != NULL) {
        Device * next = device->next;

        free(device->name);

        destroyInterface(device->interfaces);
        destroyRoute(device->routes);

        free(device);

        device = next;
    }
}

void destroyEndpoint(Endpoint * endpoint) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    if (endpoint != NULL) {
        free(endpoint->device);
        free(endpoint->interface);
        free(endpoint);
    }
}

void destroyConnection(Connection * connection) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    while (connection != NULL) {
        Connection * next = connection->next;

        destroyEndpoint(connection->source);
        destroyEndpoint(connection->target);

        free(connection);

        connection = next;
    }
}

void destroyTopology(Topology * topology) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    if (topology != NULL) {
        free(topology->name);

        destroyNetwork(topology->networks);
        destroyDevice(topology->devices);
        destroyConnection(topology->connections);

        free(topology);
    }
}

void destroyProgram(Program * program) {
    logDebugging(
        _logger,
        "Executing destructor: %s",
        __FUNCTION__
    );

    if (program != NULL) {
        destroyTopology(program->topology);
        free(program);
    }
}