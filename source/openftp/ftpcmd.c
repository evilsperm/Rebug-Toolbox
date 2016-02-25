//    This file is part of OpenPS3FTP.

//    OpenPS3FTP is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    OpenPS3FTP is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with OpenPS3FTP.  If not, see <http://www.gnu.org/licenses/>.

#include <netex/libnetctl.h>
#include <netex/errno.h>
#include <netex/net.h>

#include <sys/timer.h>
#include <sys/ppu_thread.h>
#include <cell/cell_fs.h>
#include <sysutil/sysutil_common.h>

#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <assert.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "ftp_filesystem.h"
#include "common_ftp.h"
extern u8 app_shutdown;

int slisten(int port, int backlog)
{
	int list_s = socket(AF_INET, SOCK_STREAM, 0);

	//int optval = 393216;
	//setsockopt(list_s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
	//setsockopt(list_s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));

	struct sockaddr_in sa;
	socklen_t sin_len = sizeof(sa);
	memset(&sa, 0, sin_len);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(list_s, (struct sockaddr *)&sa, sin_len);
	listen(list_s, backlog);

	return list_s;
}

int sconnect(int *ret_s, const char ipaddr[16], int port)
{
	*ret_s = socket(AF_INET, SOCK_STREAM, 0);

	//int optval = 393216;
	//setsockopt((*ret_s), SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
	//setsockopt((*ret_s), SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));

	struct sockaddr_in sa;
	socklen_t sin_len = sizeof(sa);
	memset(&sa, 0, sin_len);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = inet_addr(ipaddr);

	return connect(*ret_s, (struct sockaddr *)&sa, sin_len);
}

void sclose(int *socket_e)
{
	if(*socket_e != -1)
	{
		shutdown(*socket_e, SHUT_RDWR);
		socketclose(*socket_e);
		*socket_e = -1;
	}
}

static void callback_aio(CellFsAio *aio, CellFsErrno err, int id, uint64_t size)
{
	(void) id;
    if (err == CELL_FS_SUCCEEDED) {
        aio->offset+=size;
        aio->user_data=0;
	}
	else
	{
		if (err == CELL_FS_EBUSY)
			aio->user_data=4;
		else
			aio->user_data=err;
	}
}

int recvfile(int socket_e, const char filename[512], u64 rest)
{
	int ret = -1;
	Lv2FsFile fd;
	int idw0;
	u8 c_buf=0;

	unsigned char *buf = memalign(16, BUFFER_SIZE * 2);
	unsigned char *rbuf[2];
	rbuf[0]=buf;
	rbuf[1]=buf+(BUFFER_SIZE);

	if(buf != NULL && lv2FsOpen(filename, LV2_O_WRONLY | LV2_O_CREAT, &fd, 0644, NULL, 0) == 0)
	{
		u64 read_e = 0, pos; //, write_e

		if(rest > 0)
			lv2FsLSeek64(fd, (s64)rest, SEEK_SET, &pos);
		else
			lv2FsFtruncate(fd, 0);

		CellFsAio aiow0;

		aiow0.fd = fd;
		aiow0.offset = 0;
		aiow0.buf = rbuf[c_buf];
		aiow0.user_data = 0;

		ret = 0;
		int aioret=CELL_FS_SUCCEEDED;

		while(!app_shutdown)
		{
read_next:
			if((read_e = (u64)recv(socket_e, rbuf[c_buf], BUFFER_SIZE, MSG_WAITALL)) > 0)
			{
				if(read_e>BUFFER_SIZE) {ret=-1; break;}
				while(!app_shutdown)
				{
					//while(aiow0.user_data==1) {cellSysutilCheckCallback();}
					if(!aiow0.user_data) // can write - initiate aioWrite
					{
						//memcpy(buf2, buf, read_e);
						aiow0.size = read_e;
						aiow0.user_data=1;
						aiow0.buf = rbuf[c_buf];
aioagain:
						aioret=cellFsAioWrite(&aiow0, &idw0, callback_aio);
						if(aioret==CELL_FS_EBUSY) {sys_timer_usleep(1668);cellSysutilCheckCallback(); goto aioagain;}
						if(aioret!=CELL_FS_SUCCEEDED) {aiow0.user_data=aioret;break;}

						c_buf=1-c_buf;
//						sys_ppu_thread_yield();
						goto read_next;
					}
					else
					{
						if(aiow0.user_data==1)					// aioWrite still writing - check callbacks
						{
							cellSysutilCheckCallback();
						}
						else
							{ret = aiow0.user_data; break;}					// write failed - abort
					}
				}
			}
			else
				break;

			if(ret) break;
		}

		// wait for final chunk to be written
		while(aiow0.user_data==1)
		{
			//if(aiow0.user_data==0) {break;}
			//if(aiow0.user_data==3) {ret = -1; break;}
			cellSysutilCheckCallback();
		}

		lv2FsClose(fd);
	}
	if(buf) free(buf);

	return ret;
}

/*int recvfile(int socket_e, const char filename[512], u64 rest)
{
	int ret = -1;
	Lv2FsFile fd;

	unsigned char *buf = memalign(16, 1048576);
	//unsigned char *rbuf[2];
	//rbuf[0]=buf;
	//rbuf[1]=buf+(BUFFER_SIZE);

	if(buf != NULL && lv2FsOpen(filename, LV2_O_WRONLY | LV2_O_CREAT, &fd, 0644, NULL, 0) == 0)
	{
		u64 read_e = 0, pos; //, write_e

		if(rest > 0)
			lv2FsLSeek64(fd, (s64)rest, SEEK_SET, &pos);
		else
			lv2FsFtruncate(fd, 0);

		ret = 0;

		while(1)
		{
			if((read_e = (u64)recv(socket_e, buf, 1048576, MSG_WAITALL)) > 0)
			{
				if(cellFsWrite(fd, buf, read_e, NULL)!=CELL_FS_SUCCEEDED) {ret=-1;break;}
			}
			else
				break;
		}

		lv2FsClose(fd);
	}
	if(buf) free(buf);

	return ret;
}*/

int sendfile(int socket_e, const char filename[512], u64 rest)
{
	int ret = -1;
	int fd;
	u64 bsent=0;
	struct CellFsStat s;
	s.st_size=0;
	if(cellFsStat(filename, &s)!=CELL_FS_SUCCEEDED) return -1;
	//if(lv2FsOpen(filename, LV2_O_RDONLY, &fd, 0, NULL, 0) == 0)
	if(cellFsOpen(filename, CELL_FS_O_RDONLY, &fd, NULL, 0)==CELL_FS_SUCCEEDED)
	{
		u64 read_e, pos;
		unsigned char *buf=NULL;
		u64 buf_size=BUFFER_SIZE;
		int idr0;
		u64 tmp_offset=0;
		u8 c_buf=0;

		cellFsLseek(fd, rest, CELL_FS_SEEK_SET, &pos);

		if( (u64)rest<(u64)s.st_size && ((u64)s.st_size - (u64)rest)<(BUFFER_SIZE) )
		{
			buf_size = ((u64)s.st_size - (u64)rest);
		}

		buf = (unsigned char*)memalign(16, buf_size * 2);

		if(buf)
		{
			//unsigned char *buf2 = buf+(buf_size/2);
			unsigned char *rbuf[2];
			rbuf[0]=buf;
			rbuf[1]=buf+(buf_size);
			ret = 0;
			int aioret=-1;

			CellFsAio aior0;

			aior0.fd = fd;
			aior0.offset = 0;
			aior0.buf = rbuf[c_buf];
			aior0.size = buf_size;

			aior0.user_data=1;
			cellFsAioRead(&aior0, &idr0, callback_aio);

			while(!app_shutdown)
			{
send_next:
				//while(aior0.user_data==1) {cellSysutilCheckCallback();}
				if(!aior0.user_data && tmp_offset<=aior0.offset)
				{
					read_e=(aior0.offset-tmp_offset);
					if(!read_e) break;
					tmp_offset=aior0.offset;
					//memcpy(buf, buf2, read_e);
					c_buf=1-c_buf;
					aior0.buf = rbuf[c_buf];
					aior0.user_data=1;
aioagain:
					aioret=cellFsAioRead(&aior0, &idr0, callback_aio);
					if(aioret==CELL_FS_EBUSY) {sys_timer_usleep(1668); cellSysutilCheckCallback(); goto aioagain;}
					if(aioret!=CELL_FS_SUCCEEDED) {aior0.user_data=aioret; ret=-1;break;};
					bsent=(u64)send(socket_e, rbuf[1-c_buf], (size_t)read_e, 0);
					if(bsent < read_e || bsent>read_e)
					{
						// send error
						ret = -1;
						break;
					}
					goto send_next;
				}
				else
				{
					if(aior0.user_data==1)					// aioRead still reading - check callbacks
					{
						cellSysutilCheckCallback();
						goto send_next;
					}
					else
					{
						if(aior0.user_data==4)				// aioRead busy
						{
							aior0.buf = rbuf[c_buf];
							aior0.user_data=1;
							while(cellFsAioRead(&aior0, &idr0, callback_aio)!=CELL_FS_SUCCEEDED){cellSysutilCheckCallback();sys_timer_usleep(1668);}
						}
						else
							{ret = aior0.user_data; break;}
					}
				}
			}

			free(buf);
		}
		cellFsClose(fd);
	}

	return ret;
}

