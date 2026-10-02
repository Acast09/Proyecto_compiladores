#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"

#include "AbstractSyntaxTree.h"
#include "BisonParser.h"

#include <stdlib.h>

ModuleDestructor initializeBisonActionsModule(
    CompilerState * compilerState
);

/* STRING */

char * StringSemanticAction(
    const char * value
);

/* NETWORK */

Network * NetworkSemanticAction(
    char * name,
    char * cidr
);

/* INTERFACE */

Interface * InterfaceWithCidrSemanticAction(
    char * name,
    char * cidr,
    char * network
);

Interface * InterfaceWithoutCidrSemanticAction(
    char * name,
    char * network
);

Interface * InterfaceListSemanticAction(
    Interface * list,
    Interface * interface
);

/* ROUTE */

Route * RouteSemanticAction(
    char * destination,
    char * nextHop
);

/* DEVICE */

Device * RouterSemanticAction(
    char * name,
    Device * elements
);

Device * SwitchSemanticAction(
    char * name,
    Interface * interfaces
);

Device * HostSemanticAction(
    char * name,
    Interface * interfaces
);

/* DEVICE ELEMENT ACCUMULATOR */

Device * EmptyDeviceSemanticAction();

Device * MergeDeviceSemanticAction(
    Device * device,
    Device * element
);

Device * InterfaceDeviceElementSemanticAction(
    Interface * interface
);

Device * RouteDeviceElementSemanticAction(
    Route * route
);

/* ENDPOINT / CONNECTION */

Endpoint * EndpointSemanticAction(
    char * device,
    char * interface
);

Connection * ConnectionSemanticAction(
    Endpoint * source,
    Endpoint * target
);

/* TOPOLOGY ACCUMULATOR */

Topology * EmptyTopologySemanticAction();

Topology * MergeTopologySemanticAction(
    Topology * topology,
    Topology * element
);

Topology * NetworkTopologyElementSemanticAction(
    Network * network
);

Topology * DeviceTopologyElementSemanticAction(
    Device * device
);

Topology * ConnectionTopologyElementSemanticAction(
    Connection * connection
);

/* TOPOLOGY */

Topology * TopologySemanticAction(
    char * name,
    Topology * elements
);

Program * TopologyProgramSemanticAction(
    Topology * topology
);

#endif