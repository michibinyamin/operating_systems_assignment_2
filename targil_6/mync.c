#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

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

// Function to create and bind a UDP socket
int create_udp_server(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Configure server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;            // IPv4
    server_addr.sin_port = htons(port);         // Port number
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Bind to any IP address

    // Bind the socket to the address and port
    if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }
    return server_socket; // Return the socket descriptor
}

// Function to create a UDP socket for a client and connect to the server
int create_udp_client() {
    int client_socket;
    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }
    //char *message = "Hello, server!";
    //sendto(client_socket, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    return client_socket; // Return the socket descriptor
}

// Function to return the server
struct sockaddr_in get_server(char* ip, int port){
    if (strcmp(ip, "localhost") == 0) {
        ip = "127.0.0.1";
    }
    struct sockaddr_in server_addr;
    // Configure server address struct
    server_addr.sin_family = AF_INET;            // IPv4
    server_addr.sin_port = htons(port);         // Port number
    server_addr.sin_addr.s_addr = inet_addr(ip);
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
    return server_addr;
}

// Function to return the server
struct sockaddr_un get_unix_server(char* path){
    struct sockaddr_un server_addr;
    // Configure server address struct
    server_addr.sun_family = AF_UNIX;            // IPv4
    strcpy(server_addr.sun_path, path);         // Port number
    
    return server_addr;
}

int create_unix_server(char* path){
    int server_fd, client_fd;
    struct sockaddr_un addr;
    
    // Create a UNIX domain socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a path
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Accept a connection
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    return client_fd;
}

int create_unix_client(char* path){
    int client_fd;
    struct sockaddr_un addr;

    // Create a UNIX domain socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Specify the server's address
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

int create_unix_data_server(char* path){
    int sockfd;
    struct sockaddr_un addr;
    // Step 1: Create a socket
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Step 2: Bind the socket
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int create_unix_data_client(){
    int sockfd;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}



// Handles redirections
void handle(int sockfd_in,int sockfd_out, int sockfd_both, int input, int output, int both){
    if (both)
    {
        dup2(sockfd_both, STDIN_FILENO);   // Redirect stdin to sockfd
        dup2(sockfd_both, STDOUT_FILENO);  // Redirect stdout to sockfd
        dup2(sockfd_both, STDERR_FILENO);  // Redirect stderr to sockfd
    }else{
    if (input)
        {
            dup2(sockfd_in, STDIN_FILENO);  // Redirect stdin to the read from sockfd_in
        }

        if (output)
        {
            dup2(sockfd_out, STDOUT_FILENO);  // Redirect stdout to sockfd  
            dup2(sockfd_out, STDERR_FILENO);  // Redirect stderr to sockfd
        }
    }
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
    exit(1);
}


void handle_alarm(int signo){
    if (signo == SIGALRM)
    {
        fprintf(stderr,"Time's up!");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    int isE = 3;
    if (argc < 2) {
        print_usage(argv[0]);
    }
    if (strcmp(argv[1], "-e") != 0)
    {
        isE--;
    }
    
    char *command = argv[isE-1];
    char *input_param = NULL;
    char *output_param = NULL;
    char *both_param = NULL;

    // Set time for alarm if needed
    int time;

    // Get the ports number for TCPS
    int input_port;
    int output_port;
    int both_port;
    char input_unix_port[256];
    char output_unix_port[256];
    char both_unix_port[256];
    
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
    for (int i = isE; i < argc; i += 2) {
        if (i + 1 >= argc) {
            print_usage(argv[0]);
        }   
        if (strcmp(argv[i], "-i") == 0) {
            input_param = argv[i + 1];
            int result = sscanf(input_param, "TCPS%d", &input_port);
            char *result2 = strstr(input_param, "TCPC");
            int result3 = sscanf(input_param, "UDPS%d", &input_port);
            int result4 = sscanf(input_param, "UDSSS%s", input_unix_port);
            int result5 = sscanf(input_param, "UDSCS%s", input_unix_port);
            int result6 = sscanf(input_param, "UDSSD%s", input_unix_port);
            if (result != 0)
            {
                got_input = 1;  // Set as TCPS
            }else if (result2 != NULL) // If TCPC
            {
                input_ip = strtok(input_param+4, ",");  // Skip "TCPC"
                char* token = strtok(NULL, ",");
                C_input_port = atoi(token);    // Convert token to integer
                got_input = 2;  
            }else if (result3 != 0)  
            {
                got_input = 3;  
            }else if (result4 != 0)  
            {
                got_input = 4;  
            }else if (result5 != 0)  
            {
                got_input = 5;  
            }else if (result6 != 0)  
            {
                got_input = 6;  
            }
            
    
        } else if (strcmp(argv[i], "-o") == 0) {
            output_param = argv[i + 1];
            int result = sscanf(output_param, "TCPS%d", &output_port); 
            char *result2 = strstr(output_param, "TCPC");
            char *result3 = strstr(output_param, "UDPC");
            int result4 = sscanf(output_param, "UDSSS%s", output_unix_port);
            int result5 = sscanf(output_param, "UDSCS%s", output_unix_port);
            int result6 = sscanf(output_param, "UDSCD%s", output_unix_port);
            if (result != 0)
            {
                got_output = 1;
            }else if (result2 != NULL) // If TCPC
            {
                output_ip = strtok(output_param+4, ",");  // Skip "TCPC"
                char* token = strtok(NULL, ",");
                C_output_port = atoi(token);    // Convert token to integer
                got_output = 2;
            }else if (result3 != NULL)
            {
                output_ip = strtok(output_param+4, ",");  // Skip "UDPC"
                char* token = strtok(NULL, ",");
                C_output_port = atoi(token);    // Convert token to integer
                got_output = 3;
            }else if (result4 != 0)  
            {
                got_output = 4;  
            }else if (result5 != 0)  
            {
                got_output = 5;
            }else if (result6 != 0)  
            {
                got_output = 6;
            }
            
        } else if (strcmp(argv[i], "-b") == 0) {
            both_param = argv[i + 1];
            int result = sscanf(both_param, "TCPS%d", &both_port); 
            char *result2 = strstr(both_param, "TCPC");
            int result3 = sscanf(both_param, "UDSSS%s", both_unix_port);
            int result4 = sscanf(both_param, "UDSCS%s", both_unix_port);
            if (result != 0) // If TCPC
            {
                got_both = 1;
            }else if (result2 != NULL){
                both_ip = strtok(both_param+4, ",");  // Skip "TCPC"
                sscanf(both_param, ",%d", &C_both_port); 
                char* token = strtok(NULL, ",");
                C_both_port = atoi(token);    // Convert token to integer
                got_both = 2;
            }else if(result3 != 0){
                
                got_both = 3;
            }else if(result4 != 0){
                
                got_both = 4;
            }
        
        } else if (strcmp(argv[i], "-t") == 0){
            signal(SIGALRM, handle_alarm); // Setting signal handler
            time = atoi(argv[i + 1]);
            alarm(time);    // Start alarm
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
       else if(got_both == 2){    
            sockfd_both = create_client(both_ip,C_both_port);
       }else if(got_both == 3){
            sockfd_both = create_unix_server(both_unix_port);
       }else if(got_both == 4){
            sockfd_both = create_unix_client(both_unix_port);
       }
       
    }else{
        if (input_param)
        {
            if(got_input == 1){
                // Create a server and wait for client connection
                sockfd_input = create_server(input_port);
            }else if(got_input == 2){  // == 2
                sockfd_input = create_client(input_ip,C_input_port);
            }else if(got_input == 3){
                sockfd_input = create_udp_server(input_port);
            }else if(got_input == 4){
                sockfd_input = create_unix_server(input_unix_port);
            }else if(got_input == 5){
                sockfd_input = create_unix_client(input_unix_port);
            }else if(got_input == 6){
                sockfd_input = create_unix_data_server(input_unix_port);
            }
            
        }
        if (output_param)
        {
            if(got_output == 1){
                // Create a server and wait for client connection
                sockfd_output = create_server(output_port);
            }else if(got_output == 2){
                sockfd_output = create_client(output_ip,C_output_port);
            }else if(got_output == 3){
                sockfd_output = create_udp_client(output_ip,C_output_port);
            }else if(got_output == 4){
                sockfd_output = create_unix_server(output_unix_port);
            }else if(got_output == 5){
                sockfd_output = create_unix_client(output_unix_port);
            }else if(got_output == 6){
                sockfd_output = create_unix_data_client();
            }
        }
    }
    
    // this function will handle directing the file descripters as needed
    handle(sockfd_input,sockfd_output,sockfd_both, got_input, got_output, got_both);

    if (got_output == 3 || got_output == 6)    // for UDP
    {
        // Redirect stdout to a file descriptor
        int fd[2]; // Array to hold the two file descriptors
        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            struct sockaddr_in server_addr;
            struct sockaddr_un server_addr2;
            close(fd[1]);
            if (got_output == 3)
            {
                server_addr = get_server(output_ip,C_output_port);
            }
            else{
                server_addr2 = get_unix_server(output_unix_port);
            }
            
            char buffer[1024];
            while (1)
            {
                int bytes_read = read(fd[0], buffer, sizeof(buffer));
                if (bytes_read == -1) {
                    perror("read");
                    return 1;
                }
                int len = strlen(buffer);
                if (len > 1) { // Data entered
                    // Send data to server
                    int bytes_sent;
                    if (got_output == 3)
                    {   // in server : nc -u -l port
                        bytes_sent = sendto(sockfd_output, buffer, bytes_read, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));
                        if (bytes_sent < 0) {
                            perror("sendto failed");
                            close(sockfd_output);
                            exit(EXIT_FAILURE);
                        }
                    }
                    else{   // in server : nc -uU -l -k path
                        bytes_sent = sendto(sockfd_output, buffer, bytes_read, 0,(struct sockaddr *)&server_addr2, sizeof(server_addr2));
                        if (bytes_sent < 0) {
                            perror("sendto failed");
                            close(sockfd_output);
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (bytes_sent < 0) {
                        perror("sendto failed");
                        close(sockfd_output);
                        exit(EXIT_FAILURE);
                    }
                }
            }
            close(sockfd_output);
            close(fd[0]);
        }else{
            close(fd[0]);
            dup2(fd[1],STDERR_FILENO);
            dup2(fd[1],STDOUT_FILENO);
            if (isE == 3)   // id -e run the program
            {
                if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            close(sockfd_output);
            close(fd[1]);
            }else{  // Chat (print the input on the screen so it can be taken)
                char buffer[1024]; // buffer to hold the input
                while(1){
                    if (fgets(buffer,sizeof(buffer), stdin) != NULL)
                    {
                        //printf("%s",buffer);
                        dprintf(STDERR_FILENO,"%s",buffer);
                    }
                }
            }
        }
    }

    // Tcp
    if(isE == 3)   // id -e run the program
    {
        // Execute the desired program
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    // In else case we choose to print the input, this way the communication works
    else{
        char buffer[1024]; // buffer to hold the input
        while(1){
            if (fgets(buffer,sizeof(buffer), stdin) != NULL)
            {
                //printf("%s",buffer);
                dprintf(STDERR_FILENO,"%s",buffer);
            }
            
        }
    }

    // Close the sockets
    // if (close(sockfd_input) == -1) {
    //     perror("Error while closing socket");
    //     exit(EXIT_FAILURE);
    // }
    // if (close(sockfd_output) == -1) {
    //     perror("Error while closing socket");
    //     exit(EXIT_FAILURE);
    // }
    // if (close(sockfd_both) == -1) {
    //     perror("Error while closing socket");
    //     exit(EXIT_FAILURE);
    // }
    return 0;
}

// Using netcat : 

// tcp server : nc -l port
// udp server : nc -u -l port
// unix domain stream server : nc -U -l path
// unix domain datagram server : nc -uU -l -k path

// tcp client : nc localhost port
// udp client : nc -u localhost port
// unix domain stream client : nc -U path
// unix domain datagram client : -uU path


