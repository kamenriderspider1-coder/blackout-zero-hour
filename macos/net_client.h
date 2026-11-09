#ifndef BLACKOUT_NET_CLIENT_H
#define BLACKOUT_NET_CLIENT_H
typedef struct {
    int connected;
    int peer_present;
    int peer_id;
    float peer_x, peer_y, peer_angle;
    int peer_signals;
    char status[96];
} BlackoutNet;
int blackout_net_connect(BlackoutNet *net, const char *host, int port, const char *name, const char *room);
void blackout_net_update(BlackoutNet *net, float x, float y, float angle, int signals);
void blackout_net_close(BlackoutNet *net);
#endif
