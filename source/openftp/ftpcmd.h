// Functions specifically for socket/ftp use

#ifndef _openps3ftp_cmdfunc_
#define _openps3ftp_cmdfunc_

#define ssend(socket, str) send(socket, str, strlen(str), 0)

int slisten(int port, int backlog);
int sconnect(int *ret_s, const char ipaddr[16], int port);
void sclose(int *socket);

int recvfile(int socket, const char filename[512], u64 rest);
int sendfile(int socket, const char filename[512], u64 rest);

#endif /* _openps3ftp_cmdfunc_ */
