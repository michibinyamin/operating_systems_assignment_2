#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
    exit(1);
}

int setup_tcp_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int new_socket;
    if ((new_socket = accept(server_fd, NULL, NULL)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return new_socket;
}

int setup_tcp_client(const char *hostname, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *he;

    if ((he = gethostbyname(hostname)) == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "-e") != 0) {
        print_usage(argv[0]);
    }

    char *command = argv[2];
    char *input_param = NULL;
    char *output_param = NULL;
    char *both_param = NULL;

    for (int i = 3; i < argc; i += 2) {
        if (i + 1 >= argc) {
            print_usage(argv[0]);
        }
        if (strcmp(argv[i], "-i") == 0) {
            input_param = argv[i + 1];
        } else if (strcmp(argv[i], "-o") == 0) {
            output_param = argv[i + 1];
        } else if (strcmp(argv[i], "-b") == 0) {
            both_param = argv[i + 1];
        } else {
            print_usage(argv[0]);
        }
    }

    char *args[256];
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < 255) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        int input_fd = -1, output_fd = -1;

        if (input_param) {
            if (strncmp(input_param, "TCPS", 4) == 0) {
                int port = atoi(input_param + 4);
                input_fd = setup_tcp_server(port);
            } else if (strncmp(input_param, "TCPC", 4) == 0) {
                char *ip = strtok(input_param + 4, ",");
                int port = atoi(strtok(NULL, ","));
                input_fd = setup_tcp_client(ip, port);
            }
        }

        if (output_param) {
            if (strncmp(output_param, "TCPS", 4) == 0) {
                int port = atoi(output_param + 4);
                output_fd = setup_tcp_server(port);
            } else if (strncmp(output_param, "TCPC", 4) == 0) {
                char *ip = strtok(output_param + 4, ",");
                int port = atoi(strtok(NULL, ","));
                output_fd = setup_tcp_client(ip, port);
            }
        }

        if (both_param) {
            if (strncmp(both_param, "TCPS", 4) == 0) {
                int port = atoi(both_param + 4);
                input_fd = output_fd = setup_tcp_server(port);
            } else if (strncmp(both_param, "TCPC", 4) == 0) {
                char *ip = strtok(both_param + 4, ",");
                int port = atoi(strtok(NULL, ","));
                input_fd = output_fd = setup_tcp_client(ip, port);
            }
        }

        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(1);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
