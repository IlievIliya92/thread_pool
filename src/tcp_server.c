/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>

#include <arpa/inet.h> 

#include "tcp_server.h"
#include "queue.h"
/******************************** LOCAL DEFINES *******************************/

#define TCP_SERVER_BACKLOG  10
/*********************************** TYPEDEFS *********************************/
//  Structure of our class

struct _tcp_server_t {
    int sock_fd;

    pthread_t *workers;
    int workers_n;

    conn_handler_t conn_handler;
    queue_t *conn_queue;
    pthread_mutex_t conn_mutex;
    pthread_cond_t  conn_cond;
};

/************************* LOCAL FUNCTIONS DEFINITIONS ************************/

/********************************* LOCAL DATA *********************************/
/* State (volatile since we are stopping this from keyboard isr)*/
static volatile bool SERVER_STOP = true;
/******************************* INTERFACE DATA *******************************/

/******************************* LOCAL FUNCTIONS ******************************/
static void tcp_server_sig_hndlr(int signum)
{
    SERVER_STOP = true;

    fprintf(stderr, "Server shuting down...\n");

}

static void *worker_thread(void *arg)
{
    tcp_server_t *self = (tcp_server_t *)arg;

    while (!SERVER_STOP)
    {
        /* Worker thread main loop */
        int conn;
        int ret = 0;

        pthread_mutex_lock(&self->conn_mutex);

        while (is_empty(self->conn_queue) && !SERVER_STOP) {
            pthread_cond_wait(&self->conn_cond, &self->conn_mutex);
        }
        if (SERVER_STOP) {
            pthread_mutex_unlock(&self->conn_mutex);
            break;
        }
        ret = dequeue(self->conn_queue, &conn);
        pthread_mutex_unlock(&self->conn_mutex);
        if (ret == -1)
        {
            continue;
        }
        fprintf(stderr, "Worker %lu handling connection %d\n", pthread_self(), conn);
        self->conn_handler(conn);
    }

    return NULL;
}
/***********************************  METHODS *********************************/

//  --------------------------------------------------------------------------
// Constructor
/**
 *
 * Create new tcp_server object.
 *
 * Returns:
 *     On success new tcp_server object, or NULL if the new tcp server could not be created.
 */
tcp_server_t *
tcp_server_new (const char *ipv4, int server_port, int workers_n, conn_handler_t conn_handler)
{
    tcp_server_t *self = (tcp_server_t *) malloc (sizeof (tcp_server_t));
    assert (self);

    fprintf(stderr, "TCP Server created [%p]\n", self);

    int ret = 0;
    struct sockaddr_in saddr;

    self->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (self->sock_fd == -1)
    {
        fprintf(stderr, "Failed to create TCP server socket! (%s)\n", strerror(errno));
        free(self);
        return NULL;
    }

    /* reuse sock */
    int optval = 1;
    ret = setsockopt(self->sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret == -1)
    {
        fprintf(stderr, "Failed to set sock option SO_REUSEADDR! (%s)\n", strerror(errno));
        close(self->sock_fd);
        free(self);
        return NULL;
    }

    /* Bind the socket */
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(ipv4);
    saddr.sin_port = htons(server_port);
    ret = bind(self->sock_fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret == -1) {
        fprintf(stderr, "bind failed! (%s)", strerror(errno));
        close(self->sock_fd);
        free(self);
        return NULL;
    }

    ret = listen(self->sock_fd, TCP_SERVER_BACKLOG);
    if (ret == -1)
    {
        fprintf(stderr, "listen failed! (%s)", strerror(errno));
        close(self->sock_fd);
        free(self);
        return NULL;
    }

    // Create thread pool 
    self->workers_n = workers_n;
    self->workers = (pthread_t *)malloc(sizeof(pthread_t) * workers_n);
    if (self->workers == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for worker threads! (%s)\n", strerror(errno));
        close(self->sock_fd);
        free(self);
        return NULL;
    }
    self->conn_queue = queue_new();
    pthread_mutex_init(&self->conn_mutex, NULL);
    pthread_cond_init(&self->conn_cond, NULL);
    self->conn_handler = conn_handler;

    return self;
}


//  --------------------------------------------------------------------------
// Destructor
/**
 *
 * Destroy tcp_server object. You must use this for any tcp server created via the
 * tcp_server_new method.
 *
 * Parameters:
 *      self_p (tcp_server_t **): pointer to tcp_server_t object reference,
 *                               so the destructor can nullify it
 *
 * Returns:
 *      None (void)
 */
void
tcp_server_destroy (tcp_server_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        tcp_server_t *self = *self_p;
        fprintf(stderr, "Destroying TCP Server [%p]\n", self);
        /*
         * Free class properties here
         */

        if (self->sock_fd)
        {
            close(self->sock_fd);
        }
        free(self->workers);
        queue_destroy(self->conn_queue);

        //  Free object itself
        free (self);

        *self_p = NULL;
    }
}

/**
 *
 * Server main processing loop.
 *
 * Parameters:
 *      self_p (tcp_server_t *): reference to a tcp_server_t object
 *
 * Returns:
 *      None (void)
 */
void
tcp_server_run(tcp_server_t *self)
{
    assert(self);

    int i = 0;
    int conn = -1;

    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    
    /* Register the sig handler */
    signal(SIGINT, tcp_server_sig_hndlr);

    for (i = 0; i < self->workers_n; i++)
    {
        pthread_create(&self->workers[i], NULL, worker_thread, self);
    }

    /* Start the server loop */
    SERVER_STOP = false;
    for (;;)
    {
        int ret = 0;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(self->sock_fd, &read_fds);

        if (SERVER_STOP)
        {
            break;
        }

        // select on self->sock_fd only
        ret = select(self->sock_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ret < 0) {
            fprintf(stderr, "Select failed!");
            continue;
        }

        conn = accept(self->sock_fd, (struct sockaddr *)&src_addr, &src_addr_len);
        if (conn == -1) {
            fprintf(stderr, "Failed to accept incoming connection! (%s)\n", strerror(errno));
            continue;
        }
        fprintf(stderr, "New connection from: %s:%d\n", inet_ntoa(src_addr.sin_addr),
                ntohs(src_addr.sin_port));

        pthread_mutex_lock(&self->conn_mutex);
        fprintf(stderr, "Enqueuing connection %d\n", conn);
        enqueue(self->conn_queue, conn);
        pthread_cond_signal(&self->conn_cond);
        pthread_mutex_unlock(&self->conn_mutex);

    }
}

//  --------------------------------------------------------------------------
