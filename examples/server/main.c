/******************************** INCLUDE FILES *******************************/
#include <stdlib.h>
#include <argp.h>
#include <stdbool.h>
#include <unistd.h>

#include "tcp_server.h"  /* tcp server classes */

/******************************** LOCAL DEFINES *******************************/
#define MODULE_NAME     "main"

/* Default values */
#define TCP_SERVER_DFLT_IPV4     "127.0.0.1"
#define TCP_SERVER_DFLT_PORT      9080
#define TCP_SERVER_DFLT_WORKERS_N 5

#define TCP_PORT_MIN 1024
#define TCP_PORT_MAX 49151
/******************************* LOCAL TYPEDEFS ******************************/
typedef struct _serverArgs_t
{
    /* Host network interface */
    const char *ipv4;
    /* server port */
    int port;
    /* Server workers */
    int workers;
} serverArgs_t;

/********************************* LOCAL DATA *********************************/
/* Input args table */
static struct argp_option options[] = {
    {"ipv4", 'i', "ipv4", 0, "Host IPv4 address to which the server will bind"},
    {"port", 'p', "port", 0, "TCP server port"},
    {"workers", 'w', "workers", 0, "Number of preforked TCP server workers"},
    { 0 }
};

/******************************* LOCAL FUNCTIONS ******************************/
static
void usage(const char *exec_name)
{
    fprintf(stderr, "%s -i [ipv4] -p [port] -w [workers] ", ( char *)exec_name);
    fprintf(stderr, "Run '%s --help' for more information", ( char *)exec_name);

    return;
}

/*********************** INPUT ARGS PARSE CHECK FUNCS *************************/
static
error_t parse_option( int key, char *arg, struct argp_state *state )
{
     serverArgs_t *arguments = state->input;

    switch (key) {
        case 'i':
            arguments->ipv4 = arg;
            break;
        case 'p':
            arguments->port = atoi(arg);
            break;
        case 'w':
            arguments->workers = atoi(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static struct argp argp = {options, parse_option};

/*************************** SERVER CONNECTION CB *****************************/
static void my_connection_callback(int conn)
{
    int rx_bytes = 0;
    char rx_buffer[1500];

    rx_bytes = read(conn, rx_buffer, 1500);
    /* Just loopback */
    write(conn, rx_buffer, rx_bytes);
    close(conn);
}

/********************************** MAIN **************************************/
int main(int argc, char *argv[])
{
    int i = 0;
    int ret = 0;
    serverArgs_t args = { TCP_SERVER_DFLT_IPV4, TCP_SERVER_DFLT_PORT, 
                    TCP_SERVER_DFLT_WORKERS_N };
    tcp_server_t *tcp_server = NULL;

    if(0 != argp_parse(&argp, argc, argv, 0, 0, &args))
    {
        usage(argv[0]);
        return -1;
    }

    /* TCP server params */
    fprintf(stderr, "Server bind IPv4 address: %s\n", args.ipv4);
    fprintf(stderr, "Server port: %d\n", args.port);
    fprintf(stderr, "Workers: %d\n", args.workers);

    /* Create new instance of a server */
    tcp_server = tcp_server_new(args.ipv4, args.port, args.workers, my_connection_callback);
    if (tcp_server == NULL)
    {
        fprintf(stderr, "Failed to initialize TCP server\n");
        return -1;
    }
    tcp_server_run(tcp_server);

    /* Destroy server */
    tcp_server_destroy(&tcp_server);

    return ret;
}
