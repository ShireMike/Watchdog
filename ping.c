#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <webcheck.h>
//
int logmsg(char* message);
//
int ping(struct configparms * conf) {
    char message [200 ];
    struct timespec ts;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    // 1. Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr.s_addr = inet_addr(conf->szpingurl);
    //
    //
    // Set up timer
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ms = (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
    //  
    // 2. Start the connection
    int res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (res < 0) {
        if (errno == EINPROGRESS) {
            // Connection is underway; wait using select()
            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(sock, &write_fds);

            struct timeval tv;
            tv.tv_sec = 10;
            tv.tv_usec = 0;

            // 3. Monitor the socket for writability until timeout
            res = select(sock + 1, NULL, &write_fds, NULL, &tv);

            if (res > 0) {
                // Socket became writable; check if connection succeeded
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0) {
                    //
                    // Success - calculate elapsed time
                    //
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    uint64_t ms2 = (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;     
                    sprintf(message ,"ping to %s successful (%llu ms)", conf->szpingurl, ms2-ms) ;
                    logmsg (message) ;
                    //                  
                    res = 0; // Success!
                } else {
                    sprintf(message ,"ping to %s failed - connection error", conf->szpingurl) ;
                    logmsg (message) ;
                    res = -1; // Connection error occurred
                }
            } else {
                sprintf(message ,"ping to %s failed - timeout", conf->szpingurl) ;
                logmsg (message) ;                
                res = -1; // Timeout (res == 0) or select error (res < 0)
            }
        } else {
            sprintf(message ,"ping to %s failed - immediate connection error", conf->szpingurl) ;
            logmsg (message) ;                            
            res = -1; // Immediate connection failure
        }
    }
    // 4. Restore original blocking flags and close
    fcntl(sock, fcntl(sock, F_SETFL, flags), 0);
    close(sock);
    return res;
}
