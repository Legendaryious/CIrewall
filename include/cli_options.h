#ifndef CLI_OPTIONS_H
#define CLI_OPTIONS_H

// Function to display usage information
void display_usage(const char *program_name);

// Function to handle command-line options
void handle_cli_options(int argc, char *argv[]);

// Function prototypes for writer and reader
void writer();
void reader();

#endif // CLI_OPTIONS_H
