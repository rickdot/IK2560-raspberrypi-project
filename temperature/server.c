#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

// #define NUM_OF_SLAVES 2
#define NUM_OF_SLAVES 1

int connections[NUM_OF_SLAVES];
char slaves[NUM_OF_SLAVES][18] = {
    "B8:27:EB:3F:E3:D4" // pi1
    // "64:79:F0:CA:99:5E"  // computer
};

int socket_creator(char arr[])
{

    int status = 0;
    int connection_socket = 0;
    // int bind_status = 0;
    // int bytes_read = 0;
    struct sockaddr_rc addr = {0};

    // struct timeval tv;												//Timeout for socket options for read() to be nonblocking
    // tv.tv_sec = 0;
    // tv.tv_usec = 100000;

    while (1)
    {
        connection_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM); // Initialize socket
        if (-1 == connection_socket)
        {
            perror("connection_socket");
        }

        // bind_status = bind(connection_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));            // Binds socket
        // if (-1 == bind_status) {
        // 	perror("bind_status");
        // }

        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = (uint8_t)1;
        str2ba(arr, &addr.rc_bdaddr);

        status = connect(connection_socket, (struct sockaddr *)&addr, sizeof(addr));

        // setsockopt(connection_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);		// Set a timeout, so read() is nonblocking

        // str2ba(arr, &rem_addr.l2_bdaddr);													// Converts BT address string to BT address

        // status = connect(connection_socket, (struct sockaddr *)&rem_addr, sizeof(rem_addr));	// Connect to client
        if (-1 == status)
        {
            printf("-----Pi %s failed to connect-----\n", arr);
            perror("status");
            close(connection_socket);
        }
        if (0 == status)
        {
            printf("Connection socket value: %d\n", connection_socket);
            printf("Pi %s connected\n", arr);
            break;
        }
    }
    return connection_socket;
}

void *sending(void *pthread_data)
{
    char input[60];
    while (1)
    {
        memset(input, 0, sizeof(input));
        printf("Enter message (format: [BL address]message): \n");
        scanf("%s", input);

        // get address and message
        if (input[0] != '[' || input[18] != ']')
        {
            printf("address incorrect\n");
            continue;
        }
        char address[18] = {""};
        strncpy(address, input + 1, 17);

        char msg[40] = {""};
        strncpy(msg, input + 19, 40);

        bool flag = false;
        // send to specific client
        for (int i = 0; i < NUM_OF_SLAVES; i++)
        {
            printf("%s\n", slaves[i]);
            printf("%s\n", address);

            if (strcmp(slaves[i], address) == 0)
            {
                write(connections[i], msg, 40);
                printf("Sent out!\n");
                flag = true;
                break;
            }
        }
        if (flag == false)
        {
            printf("Couldn't find specific dedvice\n");
        }
    }
    return 0;
}

void *receiving(void *pthread_data)
{
    int bytes_read;
    char buf[1024] = {0};
    while (1)
    {
        bytes_read = read(connections[0], buf, sizeof(buf));
        if (bytes_read > 0)
        {
            printf("server received [%s]\n", buf);
        }
        // check if the message is for specific client
        if (strlen(buf) < 19 || buf[0] != '[' || buf[18] != ']')
        {
            printf("No need to forward\n");
        }
        else
        {
            char address[18] = {""};
            strncpy(address, buf + 1, 17);

            char msg[40] = {""};
            strncpy(msg, buf + 19, 40);

            bool flag = false;
            // send to specific client
            for (int i = 0; i < NUM_OF_SLAVES; i++)
            {
                printf("%s\n", slaves[i]);
                printf("%s\n", address);

                if (strcmp(slaves[i], address) == 0)
                {
                    write(connections[i], msg, 40);
                    printf("Forward to %s!\n", address);
                    flag = true;
                    break;
                }
            }
            if (flag == false)
            {
                printf("Couldn't find specific dedvice, forward failed!\n");
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct sockaddr_rc addr = {0};
    int s, status;
    // char dest[18] = "B8:27:EB:3F:E3:D4";

    for (int i = 0; i < NUM_OF_SLAVES; i++)
    {
        printf("Attempting to connect with [%s]\n", slaves[i]);
        connections[i] = socket_creator(slaves[i]);
    }

    // allocate a socket
    // s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    // set the connection parameters (who to connect to)
    // addr.rc_family = AF_BLUETOOTH;
    // addr.rc_channel = (uint8_t) 1;
    // str2ba( dest, &addr.rc_bdaddr );
    // connect to client
    // status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send message (loop)

    pthread_t receiver;
    int rid = 1;
    pthread_t sender;
    int sid = 1;
    int ret;

    // receiving process
    printf("creating reveive thread\n");
    ret = pthread_create(&receiver, NULL, receiving, NULL);
    if (ret != 0)
    {
        printf("creating receiver error!\n");
        return 0;
    }
    printf("Receiver created!\n");

    // sending process
    printf("creating send thread\n");
    ret = pthread_create(&sender, NULL, sending, NULL);
    if (ret != 0)
    {
        printf("creating sender error!\n");
        return 0;
    }
    printf("sender created!\n");

    pthread_exit(NULL);
    close(s);
    return 0;
}
