#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pthread.h>

int s, master;

void *sending(void *pthread_data)
{
    char input[60];
    while (1)
    {
        printf("Enter message(If you want to forward, please use [] include Bluetooth address): \n");
        scanf("%s", input);
        write(master, input, 60);
    }
    return 0;
}

void *receiving(void *pthread_data)
{
    int bytes_read;
    char bufRead[1024];
    while (1)
    {
        bytes_read = read(master, bufRead, sizeof(bufRead));
        if (bytes_read > 0)
        {
            printf("client received [%s]\n", bufRead);
        }
    }
    return 0;
}
// client: listen for connection
int main(int argc, char **argv)
{
    struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
    char buf[1024] = {0};
    socklen_t opt = sizeof(rem_addr);

    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t)1;

    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    master = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str(&rem_addr.rc_bdaddr, buf);
    fprintf(stderr, "accepted connection from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    pthread_t receiver;
    pthread_t sender;
    int ret;

    // Receive thread
    printf("creating receive thread\n");
    ret = pthread_create(&receiver, NULL, receiving, NULL);
    if (ret != 0)
    {
        printf("creating receiver error!\n");
        return 0;
    }
    printf("Receiver created!\n");

    // Send thread
    printf("creating send thread\n");
    ret = pthread_create(&sender, NULL, sending, NULL);
    if (ret != 0)
    {
        printf("creating sender error!\n");
        return 0;
    }
    printf("sender created!\n");

    // close threads
    pthread_exit(NULL);
    // close connection
    close(master);
    close(s);
    return 0;
}
