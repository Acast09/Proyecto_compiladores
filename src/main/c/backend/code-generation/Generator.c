
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
 * Returns the network name if all assigned interfaces of a device
 * belong to exactly one network.
 *
 * Returns NULL if the device has no assigned network or belongs
 * to more than one network.
 */
static const char * _getExclusiveNetworkName(const Device * device) {
    const char * networkName = NULL;

    if (device == NULL || device->type == DEVICE_ROUTER) {
        return NULL;
    }

    for (
        const Interface * interface = device->interfaces;
        interface != NULL;
        interface = interface->next
    ) {
        if (interface->network == NULL) {
            continue;
        }

        if (networkName == NULL) {
            networkName = interface->network;
        } else if (strcmp(networkName, interface->network) != 0) {
            return NULL;
        }
    }

    return networkName;
}

/**
 * Returns the fill color associated with a device type.
 */
static const char * _getDeviceFillColor(DeviceType type) {
    switch (type) {
        case DEVICE_ROUTER:
            return "#DCEBFA";

        case DEVICE_SWITCH:
            return "#DDF3EC";

        case DEVICE_HOST:
            return "#E9EDF2";

        default:
            return "#FFFFFF";
    }
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
        "    device_%zu [shape=%s, style=filled, "
        "fillcolor=\"%s\", color=\"#64748B\", "
        "fontname=\"Helvetica\", fontcolor=\"#243447\", "
        "penwidth=1.5, label=\"",
        index,
        _getDeviceShape(device->type),
        _getDeviceFillColor(device->type)
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
 * Writes network clusters and their exclusively assigned devices.
 */
static void _writeNetworkClusters(
    FILE * output,
    const Topology * topology
) {
    size_t networkIndex = 0;

    for (
        const Network * network = topology->networks;
        network != NULL;
        network = network->next
    ) {
        fprintf(
            output,
            "    subgraph cluster_network_%zu {\n",
            networkIndex
        );

        fputs(
            "        style=\"rounded,filled\";\n"
            "        color=\"#A9C7E5\";\n"
            "        fillcolor=\"#F2F7FC\";\n"
            "        penwidth=1.5;\n"
            "        fontname=\"Helvetica\";\n"
            "        fontcolor=\"#315477\";\n"
            "        fontsize=14;\n"
            "        label=\"",
            output
        );

        _writeEscapedString(output, network->name);

        fputs("\\n", output);
        _writeEscapedString(output, network->cidr);

        fputs("\";\n", output);

        /* Network node inside its cluster. */

        fprintf(
            output,
            "        network_%zu "
            "[shape=folder, style=filled, "
            "fillcolor=\"#FFFFFF\", color=\"#8BAECC\", "
            "fontname=\"Helvetica\", label=\"",
            networkIndex
        );

        _writeEscapedString(output, network->name);

        fputs("\"];\n", output);

        /* Devices belonging exclusively to this network. */

        size_t deviceIndex = 0;

        for (
            const Device * device = topology->devices;
            device != NULL;
            device = device->next
        ) {
            const char * assignedNetwork =
                _getExclusiveNetworkName(device);

            if (
                assignedNetwork != NULL &&
                network->name != NULL &&
                strcmp(assignedNetwork, network->name) == 0
            ) {
                _writeDeviceNode(output, device, deviceIndex);
            }

            ++deviceIndex;
        }

        fputs("    }\n\n", output);

        ++networkIndex;
    }
}

/**
 * Writes devices that must remain outside network clusters.
 * This includes routers and devices belonging to multiple networks.
 */
static void _writeExternalDeviceNodes(
    FILE * output,
    const Topology * topology
) {
    size_t deviceIndex = 0;

    for (
        const Device * device = topology->devices;
        device != NULL;
        device = device->next
    ) {
        if (_getExclusiveNetworkName(device) == NULL) {
            _writeDeviceNode(output, device, deviceIndex);
        }

        ++deviceIndex;
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
            "    device_%zu -> device_%zu "
            "[dir=none, color=\"#526273\", penwidth=1.6, "
            "fontname=\"Helvetica\", fontcolor=\"#475569\", label=\"",
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

        fputs(
        "    graph ["
        "rankdir=LR, "
        "splines=polyline, "
        "nodesep=0.65, "
        "ranksep=1.0, "
        "pad=0.35, "
        "compound=true, "
        "fontname=\"Helvetica\", "
        "bgcolor=\"#FFFFFF\""
        "];\n",
        output
    );

    fputs(
        "    node ["
        "style=filled, "
        "fontname=\"Helvetica\", "
        "fontsize=10"
        "];\n",
        output
    );

    fputs(
        "    edge ["
        "fontname=\"Helvetica\", "
        "fontsize=9, "
        "color=\"#526273\""
        "];\n\n",
        output
    );

    /* Generate network clusters and their devices. */

    _writeNetworkClusters(output, topology);

    /* Generate routers and devices outside network clusters. */

    _writeExternalDeviceNodes(output, topology);

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