#include <stdlib.h>
#include <unistd.h>
#include "kvs_ev.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct kvs_ev_kqueue_t {
    int kq;
    struct kevent *events;
} kvs_ev_kqueue_t;

void *kvs_ev_kqueue_new(int size) {
    kvs_ev_kqueue_t *ev;
    ev = malloc(sizeof(kvs_ev_kqueue_t));
    if (ev == NULL) {
        return NULL;
    }

    ev->kq = kqueue();
    if (ev->kq == -1) {
        goto fail;
    }
    
    return ev;
fail:
    free(ev);
    return NULL;
}

int kvs_ev_kqueue_resize(kvs_ev_t *e, int size) {
    kvs_ev_kqueue_t *ev = (kvs_ev_kqueue_t *)e->ev;
    struct kevent    *new_events = NULL;

    if ((new_events = realloc((void *)ev->events, size)) == NULL) {
        return -1;
    }
    ev->events = new_events;
    return 0;
}

int kvs_ev_kqueue_add(kvs_ev_t *e, int fd, int mask) {
    struct kevent event;
    kvs_ev_kqueue_t *ev = (kvs_ev_kqueue_t *)e->ev;

    if (mask & KVS_EV_READ) {
        EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
        if (kevent(ev->kq, &event, 1, NULL, 0, NULL) == -1) {
            return -1;
        }
    }

    if (mask & KVS_EV_WRITE) {
        EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
        if (kevent(ev->kq, &event, 1, NULL, 0, NULL) == -1) {
            return -1;
        }
    }
    return 0;
}

int kvs_ev_kqueue_del(kvs_ev_t *e, int fd, int mask) {
    struct kevent event;
    kvs_ev_kqueue_t *ev = (kvs_ev_kqueue_t *)e->ev;

    if (mask & KVS_EV_READ) {
        EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        if (kevent(ev->kq, &event, 1, NULL, 0, NULL) == -1) {
            return -1;
        }
    }

    if (mask & KVS_EV_WRITE) {
        EV_SET(&event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        if (kevent(ev->kq, &event, 1, NULL, 0, NULL) == -1) {
            return -1;
        }
    }
    return 0;
}

int kvs_ev_kqueue_cycle(kvs_ev_t *e, struct timeval *tv) {
    ;
    return 0;
}

void kvs_ev_kqueue_free(kvs_ev_t *e) {
    kvs_ev_kqueue_t *ev = (kvs_ev_kqueue_t *)e->ev;
    free(ev->events);
    close(ev->kq);
    free(ev);
}

const kvs_ev_vtable_t kvs_ev_kqueue = {
    kvs_ev_kqueue_new,
    kvs_ev_kqueue_add,
    kvs_ev_kqueue_del,
    kvs_ev_kqueue_resize,
    kvs_ev_kqueue_cycle,
    kvs_ev_kqueue_free,
};
