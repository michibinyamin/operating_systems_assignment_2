#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int create_server(int port) {
    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t len = sizeof(cli_addr);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(sockfd, 10) == -1) {
        perror("Listen failed");
        close(sockfd);
        return -1;
    }

    // Accept connection from client
    if ((connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len)) == -1) {
        perror("Accept failed");
        close(sockfd);
        return -1;
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

    // Close the original listening socket as it's no longer needed
    close(sockfd);

    return connfd;
}



int create_client(char* ip, int port) {
    if (strcmp(ip, "localhost") == 0) {
        ip = "127.0.0.1";
    }
    
    int sockfd;
    struct sockaddr_in serverAddr;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // Fill in server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd; // Return socket file descriptor
}








void handle(int sockfd_in,int sockfd_out, int sockfd_both, char *program, char **args, int input, int output, int both){

    // Open pipes for input and output
    int input_pipefd[2];
    int output_pipefd[2];
    
    if (pipe(input_pipefd) == -1 || pipe(output_pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork for handeling client and program communication
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(input_pipefd[1]);  // Close unused write end of the input pipe
        close(output_pipefd[0]);  // Close unused read end of the output pipe

        if (input || both)
        {
            dup2(input_pipefd[0], STDIN_FILENO);  // Redirect stdin to the read end of the input pipe (set read here from the pipe)
        }
        close(input_pipefd[0]);
        

        if (output || both)
        {
            dup2(output_pipefd[1], STDOUT_FILENO);  // Redirect stdout to the write end of the output pipe
            dup2(output_pipefd[1], STDERR_FILENO);  // Redirect stderr to the write end of the output pipe
        }
        

        close(output_pipefd[1]);    // Close unused write end of the output pipe
        // Execute the desired program
        if (execvp(program, args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }else{
        // Parent process (acts as the client)
        close(input_pipefd[0]);  // Close unused read end of the input pipe
        close(output_pipefd[1]);  // Close unused write end of the output pipe

        char buffer[BUFFER_SIZE];
        ssize_t n;

        if (both){
            // Fork for handeling send and receive
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // Read data from the client and write it to the input pipe
                while ((n = read(sockfd_both, buffer, BUFFER_SIZE)) > 0) {   // Write to the buffer the client input
                    if (write(input_pipefd[1], buffer, n) != n) {   // Write to the pipe the buffer
                        perror("write to input pipe");
                        break;
                    }
                }
                close(input_pipefd[1]);  // Close write end of the input pipe
                exit(EXIT_SUCCESS); 
            }else{
                // Read the output from the output pipe and send it to the client
                while ((n = read(output_pipefd[0], buffer, BUFFER_SIZE)) > 0) {  // read to the buffer the output of the program
                    if (write(sockfd_both, buffer, n) != n) {
                        perror("write to client");
                        break;
                    }
                }
                close(output_pipefd[0]);  // Close read end of the output pipe
                wait(NULL);     // Wait for the child process to finish
            }

            close(output_pipefd[0]);  // Close read end of the output pipe
            close(input_pipefd[1]);  // Close write end of the input pipe

        }
        else if (input && output)
        {
            // Fork for handeling two terminals
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // Read data from the client and write it to the input pipe
                while ((n = read(sockfd_in, buffer, BUFFER_SIZE)) > 0) {   // Write to the buffer the client input
                    if (write(input_pipefd[1], buffer, n) != n) {   // Write to the pipe the buffer
                        perror("write to input pipe");
                        break;
                    }
                }
                close(input_pipefd[1]);  // Close write end of the input pipe
                exit(EXIT_SUCCESS); 
            }else{
                // Read the output from the output pipe and send it to the client
                while ((n = read(output_pipefd[0], buffer, BUFFER_SIZE)) > 0) {  // read to the buffer the output of the program
                    if (write(sockfd_out, buffer, n) != n) {
                        perror("write to client");
                        break;
                    }
                }
                close(output_pipefd[0]);  // Close read end of the output pipe
                wait(NULL);     // Wait for the child process to finish
            }

            close(output_pipefd[0]);  // Close read end of the output pipe
            close(input_pipefd[1]);  // Close write end of the input pipe

        }
        else{
            if (input)
            {
                // Read data from the client and write it to the input pipe
                while ((n = read(sockfd_in, buffer, BUFFER_SIZE)) > 0) {   // Write to the buffer the client input
                    if (write(input_pipefd[1], buffer, n) != n) {   // Write to the pipe the buffer
                        perror("write to input pipe");
                        break;
                    }
                }
                close(input_pipefd[1]);  // Close write end of the input pipe
            }
            if (output)
            {
                // Read the output from the output pipe and send it to the client
                while ((n = read(output_pipefd[0], buffer, BUFFER_SIZE)) > 0) {  // read to the buffer the output of the program
                    if (write(sockfd_out, buffer, n) != n) {
                        perror("write to client");
                        break;
                    }
                }
                close(output_pipefd[0]);  // Close read end of the output pipe
            }
        }
        close(sockfd_in);  // Close the client socket
        close(sockfd_out);
        wait(NULL);     // Wait for the child process to finish
    }
}




void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
    exit(1);
}





int main(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "-e") != 0) {
        print_usage(argv[0]);
    }

    char *command = argv[2];
    char *input_param = NULL;
    char *output_param = NULL;
    char *both_param = NULL;


    // Get the ports number for TCPS
    int input_port;
    int output_port;
    int both_port;

    // Get the ports and ip numbers for TCPC
    int C_input_port;
    int C_output_port;
    int C_both_port;

    char* input_ip;
    char* output_ip;
    char* both_ip;

    // Mark which and what to create. 0-nothing, 1-TCPS, 2-TCPC
    int got_input = 0;
    int got_output = 0;
    int got_both = 0;

    // Find all of the relevent flags and store them
    for (int i = 3; i < argc; i += 2) {
        if (i + 1 >= argc) {
            print_usage(argv[0]);
        }
        if (strcmp(argv[i], "-i") == 0) {
            input_param = argv[i + 1];
            int result = sscanf(input_param, "TCPS%d", &input_port);
            got_input = 1;
            if (result == 0) // If TCPC
            {
                input_ip = strtok(input_param+4, ",");  // Skip "TCPC"
                char* token = strtok(NULL, ",");
                C_input_port = atoi(token);    // Convert token to integer
                got_input = 2;
            }
    
        } else if (strcmp(argv[i], "-o") == 0) {
            output_param = argv[i + 1];
            int result = sscanf(output_param, "TCPS%d", &output_port);   
            got_output = 1;
            if (result == 0) // If TCPC
            {
                output_ip = strtok(output_param+4, ",");  // Skip "TCPC"
                char* token = strtok(NULL, ",");
                C_output_port = atoi(token);    // Convert token to integer
                got_output = 2;
            }   
        } else if (strcmp(argv[i], "-b") == 0) {
            both_param = argv[i + 1];
            int result = sscanf(both_param, "TCPS%d", &both_port);   
            got_both = 1;
            if (result == 0) // If TCPC
            {
                both_ip = strtok(both_param+4, ",");  // Skip "TCPC"
                sscanf(both_param, ",%d", &C_both_port); 
                char* token = strtok(NULL, ",");
                C_both_port = atoi(token);    // Convert token to integer
                got_both = 2;
            }
        } else {
            print_usage(argv[0]);
        }
    }

    // Devide the command into an array
    char *args[256];
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < 255) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    int sockfd_input;
    int sockfd_output;
    int sockfd_both;

    if (both_param)
    {
       if (got_both == 1)
       {
            sockfd_both = create_server(both_port); // One of the ports, they are the same. this is for avoiding to create two servers
       }
       else{    // == 2
            sockfd_both = create_client(both_ip,C_both_port);
       }
       
    }else{
        if (input_param)
        {
            if(got_input == 1){
                // Create a server and wait for client connection
                sockfd_input = create_server(input_port);
            }else{  // == 2
                
                sockfd_input = create_client(input_ip,C_input_port);
            }
            
        }
        if (output_param)
        {
            if(got_output == 1){
                // Create a server and wait for client connection
                sockfd_output = create_server(output_port);
            }else{  // == 2
                sockfd_output = create_client(output_ip,C_output_port);
            }
        }
    }
    // this function will handle the rest
    handle(sockfd_input,sockfd_output,sockfd_both, args[0], args, got_input, got_output, got_both);

    // Close the sockets
    if (close(sockfd_input) == -1) {
        perror("Error while closing socket");
        exit(EXIT_FAILURE);
    }
    if (close(sockfd_output) == -1) {
        perror("Error while closing socket");
        exit(EXIT_FAILURE);
    }
    if (close(sockfd_both) == -1) {
        perror("Error while closing socket");
        exit(EXIT_FAILURE);
    }

    return 0;
}

