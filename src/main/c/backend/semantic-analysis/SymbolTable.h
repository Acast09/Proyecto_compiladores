
#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/**
 * A symbol that references a network declared in the topology.
 */
typedef struct NetworkSymbol NetworkSymbol;

struct NetworkSymbol {
    const Network * network;
    NetworkSymbol * next;
};

/**
 * A symbol that references a device declared in the topology.
 */
typedef struct DeviceSymbol DeviceSymbol;

struct DeviceSymbol {
    const Device * device;
    DeviceSymbol * next;
};

/**
 * A symbol that references an interface and its owning device.
 */
typedef struct InterfaceSymbol InterfaceSymbol;

struct InterfaceSymbol {
    const Device * device;
    const Interface * interface;
    InterfaceSymbol * next;
};

/**
 * The symbol table contains references to the elements of the AST.
 *
 * The AST owns the actual elements. The symbol table only owns its
 * auxiliary symbol nodes.
 */
typedef struct {
    NetworkSymbol * networks;
    DeviceSymbol * devices;
    InterfaceSymbol * interfaces;
} SymbolTable;

/**
 * Creates a symbol table from the specified topology.
 */
SymbolTable * createSymbolTable(const Topology * topology);

/**
 * Releases the symbol table without destroying the AST elements.
 */
void destroySymbolTable(SymbolTable * symbolTable);

/**
 * Finds a network by its identifier.
 */
const Network * findNetwork(
    const SymbolTable * symbolTable,
    const char * name
);

/**
 * Finds a device by its identifier.
 */
const Device * findDevice(
    const SymbolTable * symbolTable,
    const char * name
);

/**
 * Finds an interface by its name and owning device.
 */
const Interface * findInterface(
    const SymbolTable * symbolTable,
    const char * deviceName,
    const char * interfaceName
);

#endif