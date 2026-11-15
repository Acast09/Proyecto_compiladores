
#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/ModuleDestructor.h"

/**
 * Initialize module's internal state.
 */
ModuleDestructor initializeGeneratorModule();

/**
 * Generates a DOT representation of the specified topology.
 *
 * @param topology The topology to represent.
 * @param outputPath The path of the output DOT file.
 * @return The compilation status.
 */
CompilationStatus executeGenerator(
    const Topology *topology,
    const char *outputPath
);

/**
 * Converts a DOT file into an SVG image using Graphviz.
 *
 * @param inputPath The path of the input DOT file.
 * @param outputPath The path of the output SVG file.
 * @return The compilation status.
 */
CompilationStatus executeGraphviz(
    const char *inputPath,
    const char *outputPath
);

#endif