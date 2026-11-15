
#include "Generator.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* MODULE STATE */

static Logger * _logger = NULL;

/* PRIVATE FUNCTIONS */

/**
 * Destroys the generator module and its resources.
 */
static void _destroyGeneratorModule(void) {
    if (_logger != NULL) {
        destroyLogger(_logger);
        _logger = NULL;
    }
}

/**
 * Writes a string escaped for use inside a quoted DOT label.
 */
static void _writeEscapedString(FILE * output, const char * value) {
    if (value == NULL) {
        return;
    }

    for (const char * current = value; *current != '\0'; ++current) {
        switch (*current) {
            case '"':
                fputs("\\\"", output);
                break;

            case '\\':
                fputs("\\\\", output);
                break;

            case '\n':
                fputs("\\n", output);
                break;

            case '\r':
                fputs("\\r", output);
                break;

            case '\t':
                fputs("\\t", output);
                break;

            default:
                fputc(*current, output);
                break;
        }
    }
}

/**
 * Returns the DOT shape associated with a device type.
 */
static const char * _getDeviceShape(DeviceType type) {
    switch (type) {
        case DEVICE_ROUTER:
            return "ellipse";

        case DEVICE_SWITCH:
            return "box";

        case DEVICE_HOST:
            return "component";

        default:
            return "box";
    }
}

/**
 * Returns the textual name of a device type.
 */
static const char * _getDeviceTypeName(DeviceType type) {
    switch (type) {
        case DEVICE_ROUTER:
            return "Router";

        case DEVICE_SWITCH:
            return "Switch";

        case DEVICE_HOST:
            return "Host";

        default:
            return "Unknown";
    }
}

/**
 * Finds the position of a device in the topology device list.
 */
static bool _findDeviceIndex(
    const Topology * topology,
    const char * deviceName,
    size_t * index
) {
    size_t currentIndex = 0;

    if (
        topology == NULL ||
        deviceName == NULL ||
        index == NULL
    ) {
        return false;
    }

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        if (
            device->name != NULL &&
            strcmp(device->name, deviceName) == 0
        ) {
            *index = currentIndex;
            return true;
        }

        ++currentIndex;
    }

    return false;
}

/**
 * Writes a device and its interfaces and static routes.
 */
static void _writeDeviceNode(
    FILE * output,
    const Device * device,
    size_t index
) {
    fprintf(
        output,
        "    device_%zu [shape=%s, label=\"",
        index,
        _getDeviceShape(device->type)
    );

    _writeEscapedString(output, device->name);

    fprintf(
        output,
        "\\n%s",
        _getDeviceTypeName(device->type)
    );

    if (device->interfaces != NULL) {
        fputs("\\n\\nInterfaces:", output);

        for (
            const Interface * interface = device->interfaces;
            interface != NULL;
            interface = interface->next
        ) {
            fputs("\\n", output);
            _writeEscapedString(output, interface->name);

            if (interface->cidr != NULL) {
                fputs(": ", output);
                _writeEscapedString(output, interface->cidr);
            }

            if (interface->network != NULL) {
                fputs(" [", output);
                _writeEscapedString(output, interface->network);
                fputs("]", output);
            }
        }
    }

    if (device->routes != NULL) {
        fputs("\\n\\nStatic routes:", output);

        for (
            const Route * route = device->routes;
            route != NULL;
            route = route->next
        ) {
            fputs("\\n", output);
            _writeEscapedString(output, route->destination);
            fputs(" via ", output);
            _writeEscapedString(output, route->nextHop);
        }
    }

    fputs("\"];\n", output);
}

/**
 * Writes all network nodes.
 */
static void _writeNetworkNodes(
    FILE * output,
    const Topology * topology
) {
    for (
        const Network * network = topology->networks;
        network != NULL;
        network = network->next
    ) {
        size_t index = 0;

        for (
            const Network * current = topology->networks;
            current != network;
            current = current->next
        ) {
            ++index;
        }

        fprintf(
            output,
            "    network_%zu [shape=folder, style=filled, "
            "fillcolor=lightblue, label=\"",
            index
        );

        _writeEscapedString(output, network->name);

        fputs("\\n", output);
        _writeEscapedString(output, network->cidr);

        fputs("\"];\n", output);
    }
}

/**
 * Writes the logical associations between interfaces and networks.
 */
static void _writeNetworkAssociations(
    FILE * output,
    const Topology * topology
) {
    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        size_t deviceIndex = 0;

        _findDeviceIndex(topology, device->name, &deviceIndex);

        for (
            const Interface * interface = device->interfaces;
            interface != NULL;
            interface = interface->next
        ) {
            if (interface->network == NULL) {
                continue;
            }

            size_t networkIndex = 0;
            bool networkFound = false;

            for (
                const Network * network = topology->networks;
                network != NULL;
                network = network->next
            ) {
                if (
                    network->name != NULL &&
                    strcmp(network->name, interface->network) == 0
                ) {
                    networkFound = true;
                    break;
                }

                ++networkIndex;
            }

            if (networkFound) {
                fprintf(
                    output,
                    "    device_%zu -> network_%zu "
                    "[style=dashed, label=\"",
                    deviceIndex,
                    networkIndex
                );

                _writeEscapedString(output, interface->name);

                fputs("\"];\n", output);
            }
        }
    }
}

/**
 * Writes the physical connections between devices.
 */
static bool _writePhysicalConnections(
    FILE * output,
    const Topology * topology
) {
    for (
        const Connection * connection = topology->connections;
        connection != NULL;
        connection = connection->next
    ) {
        size_t sourceIndex = 0;
        size_t targetIndex = 0;

        if (
            connection->source == NULL ||
            connection->target == NULL ||
            !_findDeviceIndex(
                topology,
                connection->source->device,
                &sourceIndex
            ) ||
            !_findDeviceIndex(
                topology,
                connection->target->device,
                &targetIndex
            )
        ) {
            return false;
        }

        fprintf(
            output,
            "    device_%zu -> device_%zu [label=\"",
            sourceIndex,
            targetIndex
        );

        _writeEscapedString(
            output,
            connection->source->interface
        );

        fputs(" -- ", output);

        _writeEscapedString(
            output,
            connection->target->interface
        );

        fputs("\"];\n", output);
    }

    return true;
}

/* PUBLIC FUNCTIONS */

ModuleDestructor initializeGeneratorModule(void) {
    if (_logger == NULL) {
        _logger = createLogger("Generator");
    }

    if (_logger == NULL) {
        return NULL;
    }

    return _destroyGeneratorModule;
}

CompilationStatus executeGenerator(
    const Topology * topology,
    const char * outputPath
) {
    if (topology == NULL || outputPath == NULL) {
        if (_logger != NULL) {
            logError(
                _logger,
                "Cannot generate graph: invalid topology or output path."
            );
        }

        return FAILED;
    }

    FILE * output = fopen(outputPath, "w");

    if (output == NULL) {
        if (_logger != NULL) {
            logError(
                _logger,
                "Could not open output file \"%s\".",
                outputPath
            );
        }

        return FAILED;
    }

    fprintf(output, "digraph \"");
    _writeEscapedString(output, topology->name);
    fputs("\" {\n", output);

    fputs("    graph [rankdir=LR, splines=true];\n", output);
    fputs("    node [style=filled, fillcolor=white];\n", output);
    fputs("    edge [color=gray40];\n\n", output);

    /* Generate device nodes. */

    size_t deviceIndex = 0;

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        _writeDeviceNode(output, device, deviceIndex);
        ++deviceIndex;
    }

    fputc('\n', output);

    /* Generate network nodes. */

    _writeNetworkNodes(output, topology);

    fputc('\n', output);

    /* Generate logical network associations. */

    _writeNetworkAssociations(output, topology);

    fputc('\n', output);

    /* Generate physical connections. */

    bool connectionsValid =
        _writePhysicalConnections(output, topology);

    fputs("}\n", output);

    bool writeFailed = ferror(output) != 0;
    int closeStatus = fclose(output);

    if (!connectionsValid || writeFailed || closeStatus != 0) {
        if (_logger != NULL) {
            logError(
                _logger,
                "An error occurred while generating graph \"%s\".",
                outputPath
            );
        }

        return FAILED;
    }

    if (_logger != NULL) {
        logInformation(
            _logger,
            "Graph successfully generated at \"%s\".",
            outputPath
        );
    }

    return SUCCEEDED;
}

CompilationStatus executeGraphviz(
    const char *inputPath,
    const char *outputPath
) {
    if (inputPath == NULL || outputPath == NULL) {
        if (_logger != NULL) {
            logError(
                _logger,
                "Cannot execute Graphviz: invalid input or output path."
            );
        }

        return FAILED;
    }

    pid_t processId = fork();

    if (processId < 0) {
        if (_logger != NULL) {
            logError(
                _logger,
                "Could not create Graphviz process."
            );
        }

        return FAILED;
    }

    if (processId == 0) {
        execlp(
            "dot",
            "dot",
            "-Tsvg",
            inputPath,
            "-o",
            outputPath,
            (char *) NULL
        );

        _exit(127);
    }

    int processStatus;

    while (waitpid(processId, &processStatus, 0) < 0) {
        if (errno != EINTR) {
            if (_logger != NULL) {
                logError(
                    _logger,
                    "Could not wait for Graphviz process."
                );
            }

            return FAILED;
        }
    }

    if (
        !WIFEXITED(processStatus) ||
        WEXITSTATUS(processStatus) != 0
    ) {
        if (_logger != NULL) {
            logError(
                _logger,
                "Graphviz failed to generate SVG file \"%s\".",
                outputPath
            );
        }

        return FAILED;
    }

    if (_logger != NULL) {
        logInformation(
            _logger,
            "SVG successfully generated at \"%s\".",
            outputPath
        );
    }

    return SUCCEEDED;
}