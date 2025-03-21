#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_options.h"

// Function to display usage information
void display_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <main|client>\n", program_name);
    fprintf(stderr, "  main  - Run the program as main process to spawn and hold the wall.\n");
    fprintf(stderr, "  client  - Run the program as a client to access shared memory and blocked ips.\n");
}

// Function to handle command-line options
CLIMode handle_cli_options(int argc, char *argv[]) {
    CLIMode mode = MODE_NONE;

    if (argc != 2) {
        display_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "main") == 0) {
        mode |= MODE_MAIN; // Set main mode
    } else if (strcmp(argv[1], "client") == 0) {
        mode |= MODE_CLIENTt; // Set client mode
    } else {
        fprintf(stderr, "Invalid argument: %s\n", argv[1]);
        display_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    return mode;
}
