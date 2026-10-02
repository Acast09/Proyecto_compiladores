cmake_minimum_required(VERSION 3.28.3)

project(Flex-Bison-Compiler C)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(NOTICE "The C compiler is GCC.")

        add_custom_command(
                OUTPUT
                        ../src/main/c/frontend/syntactic-analysis/BisonParser.c
                        ../src/main/c/frontend/syntactic-analysis/BisonParser.h
                COMMAND ../src/main/bash/bison.sh
        )

        add_compile_options(-fsanitize=address)
        add_compile_options(-O3)
        add_compile_options(-static-libgcc)
        add_compile_options(-std=gnu99)
        add_link_options(-fsanitize=address)

        add_custom_command(
                OUTPUT
                        ../src/main/c/frontend/lexical-analysis/FlexScanner.c
                        ../src/main/c/frontend/lexical-analysis/FlexScanner.h
                COMMAND ../src/main/bash/flex.sh
                DEPENDS
                        ../src/main/c/frontend/syntactic-analysis/BisonParser.c
                        ../src/main/c/frontend/syntactic-analysis/BisonParser.h
        )

        add_executable(Flex-Bison-Compiler
                src/main/c/EntryPoint.c

                src/main/c/frontend/Frontend.c

                src/main/c/frontend/lexical-analysis/FlexActions.c
                src/main/c/frontend/lexical-analysis/FlexScanner.c

                src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.c
                src/main/c/frontend/syntactic-analysis/BisonActions.c
                src/main/c/frontend/syntactic-analysis/BisonParser.c

                src/main/c/support/configuration/Environment.c
                src/main/c/support/language/String.c
                src/main/c/support/logging/Logger.c
        )

        target_link_libraries(Flex-Bison-Compiler)
else ()
        message(NOTICE "The C compiler is unknown.")
endif ()