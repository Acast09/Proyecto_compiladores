    
#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

#include "SymbolTable.h"

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/ModuleDestructor.h"

/**
 * Initializes the semantic analyzer module.
 */
ModuleDestructor initializeSemanticAnalyzerModule();

/**
 * Validates the topology using its symbol table.
 *
 * Returns SUCCEEDED if no semantic errors are found.
 * Warnings do not prevent compilation.
 */
CompilationStatus analyzeSemantics(
    const Topology * topology,
    const SymbolTable * symbolTable
);

#endif