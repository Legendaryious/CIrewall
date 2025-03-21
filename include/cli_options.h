#ifndef CLI_OPTIONS_H
#define CLI_OPTIONS_H

// Enum for command-line modes
typedef enum {
    MODE_NONE = 0,
    MODE_MAIN = 1 << 0, // 1
    MODE_CLIENT = 1 << 1  // 2
} CLIMode;

// Function to display usage information
void display_usage(const char *program_name);

// Function to handle command-line options
CLIMode handle_cli_options(int argc, char *argv[]);

#endif // CLI_OPTIONS_H
