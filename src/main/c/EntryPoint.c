#include "frontend/Frontend.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "frontend/syntactic-analysis/BisonActions.h"

#include "backend/semantic-analysis/SemanticAnalyzer.h"
#include "backend/semantic-analysis/SymbolTable.h"
#include "backend/code-generation/Generator.h"

#include "support/logging/Logger.h"
#include "support/type/CompilationStatus.h"
#include "support/type/CompilerState.h"
#include "support/type/ModuleDestructor.h"

/**
 * The main entry-point of the compiler.
 *
 */
const int main(
    const int length,
    const char ** arguments
) {
    LexicalAnalyzer * lexicalAnalyzer =
        createLexicalAnalyzer();

    Logger * logger =
        createLogger("EntryPoint");

    for (int k = 0; k < length; ++k) {
        logDebugging(
            logger,
            "Argument %d: \"%s\"",
            k,
            arguments[k]
        );
    }

    CompilerState compilerState = {
        .abstractSyntaxtTree = NULL,
        .value = 0
    };

    ModuleDestructor moduleDestructors[] = {
        initializeAbstractSyntaxTreeModule(),
        initializeFlexActionsModule(lexicalAnalyzer),
        initializeBisonActionsModule(&compilerState),
        initializeFrontendModule(lexicalAnalyzer),
        initializeSemanticAnalyzerModule(),
        initializeGeneratorModule()
    };

    /* 1. Syntactic analysis */

    CompilationStatus compilationStatus =
        executeSyntacticAnalysis();

    Program * program =
        (Program *) compilerState.abstractSyntaxtTree;

    if (compilationStatus == SUCCEEDED) {
        logDebugging(
            logger,
            "Syntactic-analysis phase completed successfully."
        );

        if (program == NULL || program->topology == NULL) {
            logError(
                logger,
                "The syntactic analysis did not produce a valid AST."
            );

            compilationStatus = FAILED;
        }
        else {
            logDebugging(
                logger,
                "AST successfully created for topology \"%s\".",
                program->topology->name
            );
        }
    }
    else {
        logError(
            logger,
            "The syntactic-analysis phase rejects the input program."
        );
    }

    /* 2. Semantic analysis */

    SymbolTable * symbolTable = NULL;

    if (compilationStatus == SUCCEEDED) {
        logInformation(
            logger,
            "Starting semantic-analysis phase..."
        );

        symbolTable =
            createSymbolTable(program->topology);

        if (symbolTable == NULL) {
            logError(
                logger,
                "Could not create the symbol table."
            );

            compilationStatus = OUT_OF_MEMORY;
        }
        else {
            compilationStatus =
                analyzeSemantics(
                    program->topology,
                    symbolTable
                );
        }
    }

    if (compilationStatus == SUCCEEDED) {
        logInformation(
            logger,
            "Semantic-analysis phase completed successfully."
        );
    }
    else {
        logError(
            logger,
            "The semantic-analysis phase rejects the input program."
        );
    }

    // 3. Code generation
    if (compilationStatus == SUCCEEDED) {
        logInformation(logger, "Starting code-generation phase...");

        // Generate the DOT file.
        compilationStatus = executeGenerator(
            program->topology,
            "topology.dot"
        );

        // Convert the DOT file into an SVG image.
        if (compilationStatus == SUCCEEDED) {
            compilationStatus = executeGraphviz(
                "topology.dot",
                "topology.svg"
            );
        }

        if (compilationStatus == SUCCEEDED) {
            logInformation(logger, "Code-generation phase completed successfully.");
        } else {
            logError(logger, "The code-generation phase failed.");
        }
    }

    /* 3. Release resources */

    logDebugging(
        logger,
        "Releasing symbol table resources..."
    );

    destroySymbolTable(symbolTable);

    logDebugging(
        logger,
        "Releasing AST resources..."
    );

    destroyProgram(program);

    for (
        int k =
            (sizeof(moduleDestructors) /
             sizeof(ModuleDestructor)) - 1;
        0 <= k;
        --k
    ) {
        if (moduleDestructors[k] != NULL) {
        moduleDestructors[k]();
        }
    }

    logDebugging(
        logger,
        "Compilation is done."
    );

    destroyLogger(logger);
    destroyLexicalAnalyzer(lexicalAnalyzer);

    return compilationStatus;
}