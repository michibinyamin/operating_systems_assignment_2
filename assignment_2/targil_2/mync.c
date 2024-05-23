#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // Include for pid_t
#include <sys/wait.h> // Include for waitpid

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command>\n", prog_name);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-e") != 0) {
        print_usage(argv[0]);
    }

    // Command to execute
    char *command = argv[2];

    // Split command into executable and arguments
    char *args[256];
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < 255) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Null-terminate the arguments array

    // Fork the process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // In the child process, execute the command
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(1);
        }
    } else {
        // Wait for the child to finish in the parent process
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}