#include "net_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

static int fd = -1;
static char rx[4096];
static size_t used = 0;

static void status(BlackoutNet *n, const char *s) {
    snprintf(n->status, sizeof(n->status), "%s", s);
}

int blackout_net_connect(BlackoutNet *net, const char *host, int port, const char *name, const char *room) {
    if (!net || !host || !*host) return 0;
    memset(net, 0, sizeof(*net));
    struct addrinfo hints, *list = NULL, *it;
    char service[16];
    snprintf(service, sizeof(service), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    if (getaddrinfo(host, service, &hints, &list) != 0) {
        status(net, "HOST LOOKUP FAILED");
        return 0;
    }
    for (it = list; it; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0) break;
        close(fd); fd = -1;
    }
    freeaddrinfo(list);
    if (fd < 0) { status(net, "SERVER CONNECTION FAILED"); return 0; }
    char hello[512];
    snprintf(hello, sizeof(hello), "{\"type\":\"join\",\"name\":\"%.40s\",\"room\":\"%.12s\"}\n", name ? name : "Explorer", room ? room : "PUBLIC");
    if (send(fd, hello, strlen(hello), 0) < 0) {
        close(fd); fd = -1; status(net, "JOIN FAILED"); return 0;
    }
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    net->connected = 1;
    status(net, "ONLINE - WAITING FOR PLAYER");
    return 1;
}

void blackout_net_update(BlackoutNet *net, float x, float y, float angle, int signals) {
    if (!net || !net->connected || fd < 0) return;
    char out[256];
    int len = snprintf(out, sizeof(out), "{\"type\":\"state\",\"x\":%.3f,\"y\":%.3f,\"angle\":%.3f,\"signals\":%d}\n", x, y, angle, signals);
    if (send(fd, out, (size_t)len, 0) < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        net->connected = 0; status(net, "CONNECTION LOST"); close(fd); fd = -1; return;
    }
    for (;;) {
        ssize_t n = recv(fd, rx + used, sizeof(rx) - used - 1, 0);
        if (n > 0) {
            used += (size_t)n; rx[used] = 0;
            char *line;
            while ((line = strchr(rx, '\n')) != NULL) {
                *line = 0;
                int id, sig; float px, py, pa;
                if (sscanf(rx, "{\"type\":\"peer_state\",\"player_id\":%d,\"x\":%f,\"y\":%f,\"angle\":%f,\"signals\":%d}", &id, &px, &py, &pa, &sig) == 5) {
                    net->peer_present = 1; net->peer_id = id;
                    net->peer_x = px; net->peer_y = py; net->peer_angle = pa;
                    net->peer_signals = sig; status(net, "ONLINE - PLAYER CONNECTED");
                } else if (strstr(rx, "\"type\":\"peer_left\"")) {
                    net->peer_present = 0; status(net, "ONLINE - PLAYER LEFT");
                } else if (strstr(rx, "\"type\":\"error\"")) {
                    status(net, "SERVER REPORTED AN ERROR");
                }
                size_t consumed = (size_t)(line + 1 - rx);
                memmove(rx, rx + consumed, used - consumed);
                used -= consumed; rx[used] = 0;
            }
            if (used == sizeof(rx) - 1) used = 0;
        } else {
            if (n == 0) { net->connected = 0; status(net, "SERVER DISCONNECTED"); close(fd); fd = -1; }
            if (errno == EAGAIN || errno == EWOULDBLOCK || n < 0) break;
        }
    }
}

void blackout_net_close(BlackoutNet *net) {
    if (fd >= 0) { close(fd); fd = -1; }
    if (net) { net->connected = 0; net->peer_present = 0; status(net, "OFFLINE"); }
    used = 0;
}
