#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 4144

/*
The VM so fast that it terminates before even an attempt is made.
Useful for big programs but not for small programs. gotta wait for connection first
*/

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

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    unsigned char buffer[16];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = (PORT);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    sleep(1);
    // Attempt to connect to server
    while (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed, retrying in 1 second...\n");
        usleep(5);
    }

    printf("Connected to VM server.\n");

    // Example requests
    // send_request(sockfd, _GET_OS_ID_, NULL, 0);

    // // Example to add a breakpoint at address 0x123456
    // unsigned char bp_address[6] = {0x00, 0x00, 0x00, 0x12, 0x34, 0x56};
    // send_request(sockfd, _ADD_BREAKPOINT_, bp_address, 6);

    // // Example to get instruction at address 0x123456
    // send_request(sockfd, _INST_AT_, bp_address, 6);

    // Receive and handle signals
    send_request(sockfd, _GET_CORE_COUNT_, NULL, 0);
    while (1)
    {
        int bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
        {
            handle_signal(buffer);
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by server\n");
            break;
        }
        else
        {
            perror("Receive failed");
            break;
        }
    }

    // Close socket
    close(sockfd);
    return 0;
}
