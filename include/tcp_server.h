#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

/******************************** INCLUDE FILES *******************************/
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************** DEFINES **********************************/

/*********************************** TYPEDEFS *********************************/
typedef struct _tcp_server_t tcp_server_t;
typedef void (*conn_handler_t)(int conn_fd);

/************************** INTERFACE DATA DEFINITIONS ************************/

/************************* INTERFACE FUNCTION PROTOTYPES **********************/
tcp_server_t *tcp_server_new(const char *server_iface, int server_port, int workers_n, 
    conn_handler_t conn_handler);
void tcp_server_destroy(tcp_server_t **self_p);
void tcp_server_run(tcp_server_t *self);

#ifdef __cplusplus
}
#endif
#endif /* _TCP_SERVER_H_ CORE_*/
