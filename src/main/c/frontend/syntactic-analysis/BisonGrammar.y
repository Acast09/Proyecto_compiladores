%{

#include "BisonActions.h"
#include "AbstractSyntaxTree.h"

#include <stdio.h>

static void yyerror(
    YYLTYPE * location,
    const char * message
);

%}

%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed

%locations

%union {
    signed int integer;
    TokenLabel token;

    char * string;

    Network * network;
    Interface * interface;
    Route * route;
    Device * device;
    Endpoint * endpoint;
    Connection * connection;
    Topology * topology;
    Program * program;
}

/* KEYWORDS */

%token <token> TOPOLOGY
%token <token> NETWORK
%token <token> ROUTER
%token <token> SWITCH
%token <token> HOST
%token <token> INTERFACE
%token <token> CONNECT
%token <token> ROUTE
%token <token> VIA

/* VALUES */

%token <string> IDENTIFIER
%token <string> IP_ADDRESS
%token <string> IP_CIDR

/* PUNCTUATION */

%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> DOT

/* COMMENT / OTHER TOKENS */

%token <token> OPEN_COMMENT
%token <token> CLOSE_COMMENT
%token <token> IGNORED
%token <token> UNKNOWN

/* NONTERMINALS */

%type <program> program

%type <topology>
    topology
    topology_elements
    topology_element

%type <network>
    network_definition

%type <device>
    device_definition
    router_definition
    switch_definition
    host_definition
    router_elements
    router_element

%type <interface>
    interface_definition
    interface_definitions

%type <route>
    route_definition

%type <endpoint>
    endpoint

%type <connection>
    connection_definition

%type <string>
    identifier
    ip_address
    ip_cidr

/* DESTRUCTORS */

%destructor {
    destroyTopology($$);
} <topology>

%destructor {
    destroyNetwork($$);
} <network>

%destructor {
    destroyInterface($$);
} <interface>

%destructor {
    destroyRoute($$);
} <route>

%destructor {
    destroyDevice($$);
} <device>

%destructor {
    destroyEndpoint($$);
} <endpoint>

%destructor {
    destroyConnection($$);
} <connection>

%destructor {
    free($$);
} <string>

/* START */

%start program

%%

/*
 * PROGRAM
 */

program
    : topology
        {
            $$ = TopologyProgramSemanticAction($1);
        }
    ;

/*
 * TOPOLOGY
 */

topology
    : TOPOLOGY identifier OPEN_BRACE topology_elements CLOSE_BRACE
        {
            $$ = TopologySemanticAction(
                $2,
                $4
            );
        }
    ;

topology_elements
    : %empty
        {
            $$ = EmptyTopologySemanticAction();
        }

    | topology_elements topology_element
        {
            $$ = MergeTopologySemanticAction(
                $1,
                $2
            );
        }
    ;

topology_element
    : network_definition
        {
            $$ = NetworkTopologyElementSemanticAction($1);
        }

    | device_definition
        {
            $$ = DeviceTopologyElementSemanticAction($1);
        }

    | connection_definition
        {
            $$ = ConnectionTopologyElementSemanticAction($1);
        }
    ;

/*
 * NETWORK
 */

network_definition
    : NETWORK identifier ip_cidr
        {
            $$ = NetworkSemanticAction(
                $2,
                $3
            );
        }
    ;

/*
 * DEVICES
 */

device_definition
    : router_definition
        {
            $$ = $1;
        }

    | switch_definition
        {
            $$ = $1;
        }

    | host_definition
        {
            $$ = $1;
        }
    ;

/*
 * ROUTER
 */

router_definition
    : ROUTER identifier OPEN_BRACE router_elements CLOSE_BRACE
        {
            $$ = RouterSemanticAction(
                $2,
                $4
            );
        }
    ;

router_elements
    : %empty
        {
            $$ = EmptyDeviceSemanticAction();
        }

    | router_elements router_element
        {
            $$ = MergeDeviceSemanticAction(
                $1,
                $2
            );
        }
    ;

router_element
    : interface_definition
        {
            $$ = InterfaceDeviceElementSemanticAction($1);
        }

    | route_definition
        {
            $$ = RouteDeviceElementSemanticAction($1);
        }
    ;

/*
 * SWITCH
 */

switch_definition
    : SWITCH identifier OPEN_BRACE interface_definitions CLOSE_BRACE
        {
            $$ = SwitchSemanticAction(
                $2,
                $4
            );
        }
    ;

/*
 * HOST
 */

host_definition
    : HOST identifier OPEN_BRACE interface_definitions CLOSE_BRACE
        {
            $$ = HostSemanticAction(
                $2,
                $4
            );
        }
    ;

/*
 * INTERFACES
 */

interface_definitions
    : %empty
        {
            $$ = NULL;
        }

    | interface_definitions interface_definition
        {
            $$ = InterfaceListSemanticAction(
                $1,
                $2
            );
        }
    ;

interface_definition
    : INTERFACE identifier ip_cidr NETWORK identifier
        {
            $$ = InterfaceWithCidrSemanticAction(
                $2,
                $3,
                $5
            );
        }

    | INTERFACE identifier NETWORK identifier
        {
            $$ = InterfaceWithoutCidrSemanticAction(
                $2,
                $4
            );
        }
    ;

/*
 * ROUTES
 */

route_definition
    : ROUTE ip_cidr VIA ip_address
        {
            $$ = RouteSemanticAction(
                $2,
                $4
            );
        }
    ;

/*
 * CONNECTION
 */

connection_definition
    : CONNECT endpoint endpoint
        {
            $$ = ConnectionSemanticAction(
                $2,
                $3
            );
        }
    ;

endpoint
    : identifier DOT identifier
        {
            $$ = EndpointSemanticAction(
                $1,
                $3
            );
        }
    ;

/*
 * LEXICAL VALUES
 */

identifier
    : IDENTIFIER
        {
            $$ = StringSemanticAction(
                $1
            );
        }
    ;

ip_address
    : IP_ADDRESS
        {
            $$ = StringSemanticAction(
                $1
            );
        }
    ;

ip_cidr
    : IP_CIDR
        {
            $$ = StringSemanticAction(
                $1
            );
        }
    ;

%%

static void yyerror(
    YYLTYPE * location,
    const char * message
) {
    if (location != NULL) {
        fprintf(
            stderr,
            "Syntax error at line %d, column %d: %s\n",
            location->first_line,
            location->first_column,
            message
        );
    }
    else {
        fprintf(
            stderr,
            "Syntax error: %s\n",
            message
        );
    }
}