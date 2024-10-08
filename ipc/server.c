#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
 
typedef struct {
    int sock;
    char operation;
    double operand;
    int id;
} threadArgs;

double calculate(char operation, double operand, int id);

void* calculate_t(void* arg);

void my_sleep(int seconds) {
    struct timespec req;
    req.tv_sec = seconds;          
    req.tv_nsec = 0;               

    nanosleep(&req, NULL);        
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int THREADS_COUNT = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    char buffer[1024] = {0};
    char operation;
    double operand;

    while(1) {
        read(new_socket, buffer, 1024);
        sscanf(buffer, "%c %lf", &operation, &operand);
        
        THREADS_COUNT++;
        
	    pthread_t th;
        threadArgs *args = (threadArgs *)malloc(sizeof(threadArgs));
        args->sock = new_socket;
        args->operation = operation;
        args->operand = operand;
        args->id = THREADS_COUNT;
        
	    if (pthread_create(&th, NULL, calculate_t, args) != 0) {
        	perror("pthread_create failed");
        	exit(EXIT_FAILURE);
    	}

        pthread_detach(th);
    }
 
    return 0;
}
 
void* calculate_t(void* arg) {
    char buffer[1024] = {0};
    threadArgs* args = (threadArgs*) arg;
    
    double result = calculate(args->operation, args->operand, args->id);
    sprintf(buffer, "%lf", result);
    send(args->sock, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    return NULL;
}

double calculate(char operation, double operand, int id) {
    my_sleep(5);
    printf("\n[%d] Calculating %c %lf\n", id, operation, operand);
    double result = 0;
    switch(operation) {
        case '+':
            result = operand + operand;
            break;
        case '*':
            result = operand * operand;
            break;
        case 's':
            result = sqrt(operand);
            break;
        case '^':
            result = exp(operand);
            break;
        case '!':
            result = 1;
            for(int i = 1; i <= operand; i++) {
                result *= i;
            }
            break;
        default:
            printf("Invalid operation\n");
    }
    return result;
}
 
