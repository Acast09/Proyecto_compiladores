#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"

#include <stdlib.h>

ModuleDestructor initializeAbstractSyntaxTreeModule();

typedef enum DeviceType DeviceType;

typedef struct Network Network;
typedef struct Interface Interface;
typedef struct Route Route;
typedef struct Device Device;
typedef struct Endpoint Endpoint;
typedef struct Connection Connection;
typedef struct Topology Topology;
typedef struct Program Program;

enum DeviceType {
    DEVICE_ROUTER,
    DEVICE_SWITCH,
    DEVICE_HOST
};

struct Network {
    char * name;
    char * cidr;
    Network * next;
};

struct Interface {
    char * name;
    char * cidr;
    char * network;
    Interface * next;
};

struct Route {
    char * destination;
    char * nextHop;
    Route * next;
};

struct Device {
    DeviceType type;
    char * name;

    Interface * interfaces;
    Route * routes;

    Device * next;
};

struct Endpoint {
    char * device;
    char * interface;
};

struct Connection {
    Endpoint * source;
    Endpoint * target;

    Connection * next;
};

struct Topology {
    char * name;

    Network * networks;
    Device * devices;
    Connection * connections;
};

struct Program {
    Topology * topology;
};

void destroyNetwork(Network * network);

void destroyInterface(Interface * interface);

void destroyRoute(Route * route);

void destroyDevice(Device * device);

void destroyEndpoint(Endpoint * endpoint);

void destroyConnection(Connection * connection);

void destroyTopology(Topology * topology);

void destroyProgram(Program * program);

#endif