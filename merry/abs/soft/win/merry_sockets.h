// #ifndef _MERRY_SOCKETS_
// #define _MERRY_SOCKETS_

// #if defined(_WIN64)
// #include "..\utils\merry_config.h"
// #include "..\utils\merry_types.h"
// #else
// #include "../utils/merry_config.h"
// #include "../utils/merry_types.h"
// #endif

// #if defined(_MERRY_HOST_OS_LINUX_)
// #define _USE_SLINUX_
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>

// typedef struct {
//     int sd;
// } MerrySocket;

// #define INVALID_SOCKET -1

// #elif defined(_MERRY_HOST_OS_WINDOWS_)
// #define _USE_WLINUX_
// // for Windows
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "Ws2_32.lib")

// typedef struct {
//     SOCKET sd;
// } MerrySocket;
// #endif

// #define _MERRY_SOCK_STREAM_ 0

// #define _MERRY_FAMILY_IPV4_ 0
// #define _MERRY_FAMILY_IPV6_ 1


// #if defined(_USE_WLINUX_)

// static int merry_initialize_windows_sockets() {
//     WSADATA wsaData;
//     return WSAStartup(MAKEWORD(2, 2), &wsaData);
// }

// static void merry_cleanup_windows_sockets() {
//     WSACleanup();
// }

// #endif

// mret_t merry_get_socket(msize_t family, MerrySocket *sock) {
// #if defined(_USE_WLINUX_)
//     if (merry_initialize_windows_sockets() != 0) {
//         return RET_FAILURE;
//     }
// #endif

//     int domain;
//     if (family == _MERRY_FAMILY_IPV4_) {
//         domain = AF_INET;
//     } else if (family == _MERRY_FAMILY_IPV6_) {
//         domain = AF_INET6;
//     } else {
// #if defined(_USE_WLINUX_)
//         merry_cleanup_windows_sockets();
// #endif
//         return RET_FAILURE;
//     }

//     sock->sd = socket(domain, SOCK_STREAM, 0);
//     if (sock->sd == INVALID_SOCKET) {
// #if defined(_USE_WLINUX_)
//         merry_cleanup_windows_sockets();
// #endif
//         return RET_FAILURE;
//     }
//     return RET_SUCCESS;
// }

// mret_t merry_bind_socket(MerrySocket *sock, const char *address, mword_t port) {
//     struct sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(port);
//     addr.sin_addr.s_addr = inet_addr(address);

//     if (bind(sock->sd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }

//     return RET_SUCCESS;
// }

// mret_t merry_listen_socket(MerrySocket *sock, int backlog) {
//     if (listen(sock->sd, backlog) == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }
//     return RET_SUCCESS;
// }

// mret_t merry_accept_socket(MerrySocket *server_sock, MerrySocket *client_sock) {
//     SOCKET s = accept(server_sock->sd, NULL, NULL);
//     if (s == INVALID_SOCKET) {
//         return RET_FAILURE;
//     }
//     client_sock->sd = s;
//     return RET_SUCCESS;
// }

// mret_t merry_recv_socket(MerrySocket *sock, void *buffer, msize_t len, msize_t *received_len) {
//     int ret = recv(sock->sd, buffer, len, 0);
//     if (ret == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }
//     *received_len = ret;
//     return RET_SUCCESS;
// }

// mret_t merry_send_socket(MerrySocket *sock, const void *buffer, msize_t len, msize_t *sent_len) {
//     int ret = send(sock->sd, buffer, len, 0);
//     if (ret == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }
//     *sent_len = ret;
//     return RET_SUCCESS;
// }

// mret_t merry_shutdown_socket(MerrySocket *sock, int how) {
//     if (shutdown(sock->sd, how) == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }
//     return RET_SUCCESS;
// }

// mret_t merry_close_socket(MerrySocket *sock) {
// #if defined(_USE_WLINUX_)
//     if (closesocket(sock->sd) == SOCKET_ERROR) {
//         return RET_FAILURE;
//     }
//     merry_cleanup_windows_sockets();
// #else
//     if (close(sock->sd) == -1) {
//         return RET_FAILURE;
//     }
// #endif
//     return RET_SUCCESS;
// }

// #endif
