#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>

#define REQUEST_PORT 4048
#define SIGNAL_PORT 4144

// Request codes
#define _GET_CORE_COUNT_ 0x00
#define _GET_OS_ID_ 0x01
#define _GET_DATA_MEM_PAGE_COUNT_ 0x02
#define _GET_INST_MEM_PAGE_COUNT_ 0x03
#define _ADD_BREAKPOINT_ 0x04
#define _INST_AT_ 0x05
#define _DATA_AT_ 0x06
#define _SP_OF_ 0x07
#define _BP_OF_ 0x08
#define _PC_OF_ 0x09
#define _REGR_OF_ 0x0A
#define _CONTINUE_CORE_ 0x0B

// Signal codes
#define _NEW_CORE_ 0x00
#define _NEW_OS_ 0x01
#define _ERROR_ENCOUNTERED_ 0x02
#define _TERMINATING_ 0x03
#define _REPLY_ 0x04
#define _HIT_BP_ 0x05
#define _CORE_TERMINATING_ 0x06
#define _ADDED_MEM_ 0x07

static int exi = 0;

int create_socket_and_connect(int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = (port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Attempt to connect to server
    while (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection to port %d failed, retrying in 1 second...\n", port);
        sleep(1);
    }

    printf("Connected to port %d.\n", port);
    return sockfd;
}

void send_request(int sockfd, unsigned char request_code, unsigned char *args, size_t args_len)
{
    unsigned char request[16] = {0};
    request[0] = request_code;
    if (args_len > 0 && args != NULL)
    {
        memcpy(request + 10, args, args_len);
    }
    send(sockfd, request, sizeof(request), 0);
}

void handle_signal(unsigned char *signal)
{
    unsigned char signal_code = signal[0];
    switch (signal_code)
    {
    case _NEW_CORE_:
        printf("New core created with ID: %d\n", signal[15]);
        break;
    case _NEW_OS_:
        printf("New OS created\n");
        break;
    case _ERROR_ENCOUNTERED_:
        printf("Error encountered, code: %d\n", signal[15]);
        break;
    case _TERMINATING_:
        printf("VM is terminating, OS ID: %d\n", signal[15]);
        break;
    case _REPLY_:
        printf("Reply received for request\n");
        break;
    case _HIT_BP_:
        printf("Core hit breakpoint, core ID: %d\n", signal[15]);
        break;
    case _CORE_TERMINATING_:
        printf("Core is terminating, core ID: %d\n", signal[15]);
        break;
    case _ADDED_MEM_:
        printf("New memory page allocated\n");
        break;
    default:
        printf("Unknown signal code: %d\n", signal_code);
        break;
    }
}

void *signal_listener_thread(void *arg)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = (SIGNAL_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, 5);
    socklen_t addrlen = sizeof(server_addr);
    int t = accept(sockfd, (struct sockaddr *)&server_addr, &addrlen);
    close(sockfd);
    unsigned char buffer[16];

    while (1)
    {
        int bytes_received = recv(t, buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
        {
            handle_signal(buffer);
            if (buffer[0] == _TERMINATING_)
            {
                atomic_store(&exi, 1);
                printf("Listener Exiting.\n");
                break;
            }
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by server on signal port\n");
            break;
        }
        else
        {
            perror("Receive failed on signal port");
            break;
        }
    }

    close(t);
    return NULL;
}

void *request_sender_thread(void *arg)
{
    int sockfd = create_socket_and_connect(REQUEST_PORT);
    sleep(1); // Wait for the signal listener to be ready

    // Example requests
    send_request(sockfd, _GET_OS_ID_, NULL, 0);

    // Example to add a breakpoint at address 0x123456
    // unsigned char bp_address[6] = {0x00, 0x00, 0x00, 0x12, 0x34, 0x56};
    // send_request(sockfd, _ADD_BREAKPOINT_, bp_address, 6);

    // // Example to get instruction at address 0x123456
    // send_request(sockfd, _INST_AT_, bp_address, 6);

        send_request(sockfd, _GET_CORE_COUNT_, NULL, 0);
    while (1)
    {
        // Keep sending requests as needed
        // Here we just wait indefinitely, you can modify this to send periodic requests if needed
        if (exi == 1)
        {
            printf("Prep to exit\n");
            send_request(sockfd, 0, NULL, 0);
            printf("Sender Exiting.\n");
            break;
        }
        sleep(5);
    }

    close(sockfd);
    return NULL;
}

int main()
{
    pthread_t signal_thread, request_thread;

    // Create threads for sending requests and listening to signals
    pthread_create(&signal_thread, NULL, signal_listener_thread, NULL);
    pthread_create(&request_thread, NULL, request_sender_thread, NULL);

    // Wait for both threads to complete
    pthread_join(signal_thread, NULL);
    pthread_join(request_thread, NULL);

    return 0;
}
