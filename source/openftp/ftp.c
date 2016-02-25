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

const char* VERSION = "Rebug ToolBox";	// used in the welcome message and displayed on-screen

//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <netex/libnetctl.h>
#include <netex/errno.h>
#include <netex/net.h>
#include <netex/sockinfo.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <cell/cell_fs.h>

#include "ftp_filesystem.h"
#include "ftp.h"
#include "common_ftp.h"

extern int ss_timer;
extern int ss_timer_last;
extern u8 restart_request;
extern u8 app_shutdown;
// default login details
#define D_USER "anonymous"
#define D_PASS "any"

char userpass[64] = D_PASS;
char status[128];
int timeout=0;

int exitapp = 0;
//int xmbopen = 0;
int currentBuffer = 0;


static void handleclient(u64 conn_s_p)
{
	int conn_s = (int)conn_s_p; // main communications socket
	int data_s = -1; // data socket

	int connactive = 1; // whether the ftp connection is active or not
	int dataactive = 0; // prevent the data connection from being closed at the end of the loop
	int loggedin = 0;	// whether the user is logged in or not

	char user[32];		// stores the username that the user entered
	char rnfr[1024];	// stores the path/to/file for the RNFR command

	char cwd[1024];		// Current Working Directory
	int rest = 0;		// for resuming file transfers

	char buffer[2048];

	srand(conn_s);
	int p1x = (rand() % 96) + 32;
	int p2x = rand() % 256;

	union CellNetCtlInfo net_info;
	cellNetCtlGetInfo(16, &net_info);

	char ip_address[24];
	char pasv_output[64];
	sprintf(ip_address, "%s", net_info.ip_address); for(u8 n=0;n<strlen(ip_address);n++) if(ip_address[n]=='.') ip_address[n]=',';
	sprintf(pasv_output, "227 Entering Passive Mode (%s,%i,%i)\r\n", ip_address, p1x, p2x);

	// set working directory
	strcpy(cwd, "/");

	// welcome message
	ssend(conn_s, "220-PS3 FTP Server\r\n");
	sprintf(buffer, "220 %s (Login as anonymous with any password)\r\n", VERSION);	ssend(conn_s, buffer);

	timeout=0;
	struct timeval tv;
	tv.tv_usec = 0;

	while(exitapp == 0 && connactive == 1 && !app_shutdown)
	{
		if(recv(conn_s, buffer, 2047, 0) > 0)
		{
			ss_timer=0;
			ss_timer_last=time(NULL);
			// get rid of the newline at the end of the string
			buffer[strcspn(buffer, "\n")] = '\0';
			buffer[strcspn(buffer, "\r")] = '\0';

			char cmd[16], param[1024];
			int split = ssplit(buffer, cmd, 15, param, 1023);

			if(loggedin == 1)
			{
				// available commands when logged in
				if(strcasecmp(cmd, "CWD") == 0)
				{
					char tempcwd[1024];
					strcpy(tempcwd, cwd);

					if(split == 1)
					{
						absPath(tempcwd, param, cwd);
					}

					if(isDir(tempcwd))
					{
						strcpy(cwd, tempcwd);
						sprintf(buffer, "250 Directory change successful: %s\r\n", cwd);
						ssend(conn_s, buffer);
					}
					else
					{
						ssend(conn_s, "550 Cannot access directory\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "CDUP") == 0)
				{
					int pos = strlen(cwd) - 2;

					for(int i = pos; i > 0; i--)
					{
						if(i < pos && cwd[i] == '/')
						{
							break;
						}
						else
						{
							cwd[i] = '\0';
						}
					}

					sprintf(buffer, "250 Directory change successful: %s\r\n", cwd);
					ssend(conn_s, buffer);
				}
				else
				if(strcasecmp(cmd, "PASV") == 0)
				{
					rest = 0;

					int data_ls = slisten(getPort(p1x, p2x), 1);

					if(data_ls > 0)
					{
						ssend(conn_s, pasv_output);

						if((data_s = accept(data_ls, NULL, NULL)) > 0)
						{
							dataactive = 1;
						}
						else
						{
							ssend(conn_s, "451 Data connection failed\r\n");
						}

						sclose(&data_ls);
					}
					else
					{
						ssend(conn_s, "451 Cannot create data socket\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "PORT") == 0)
				{
					if(split == 1)
					{
						rest = 0;

						char data[6][4];
						char *st = strtok(param, ",");

						int i = 0;
						while(st != NULL && i < 6)
						{
							strcpy(data[i++], st);
							st = strtok(NULL, ",");
						}

						if(i == 6)
						{
							char ipaddr[16];
							sprintf(ipaddr, "%s.%s.%s.%s", data[0], data[1], data[2], data[3]);

							if(sconnect(&data_s, ipaddr, getPort(atoi(data[4]), atoi(data[5]))) == 0)
							{
								ssend(conn_s, "200 PORT command successful\r\n");
								dataactive = 1;
							}
							else
							{
								ssend(conn_s, "451 Data connection failed\r\n");
							}
						}
						else
						{
							ssend(conn_s, "501 Insufficient connection info\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No connection info given\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "LIST") == 0)
				{
					if(data_s > 0)
					{
						char tempcwd[1024];
						strcpy(tempcwd, cwd);

						if(split == 1)
						{
							absPath(tempcwd, param, cwd);
						}

						Lv2FsFile fd;

						if(lv2FsOpenDir(isDir(tempcwd) ? tempcwd : cwd, &fd) == 0)
						{
							ssend(conn_s, "150 Accepted data connection\r\n");

							Lv2FsDirent entry;
							u64 read_e;

							while(lv2FsReadDir(fd, &entry, &read_e) == 0 && read_e > 0)
							{
								if(!strcmp(entry.d_name, "app_home") || !strcmp(entry.d_name, "host_root")) continue;
								char filename[512];
								absPath(filename, entry.d_name, cwd);

								Lv2FsStat buf;
								lv2FsStat(filename, &buf);

								char tstr[16];
								strftime(tstr, 15, "%b %d %H:%M", localtime(&buf.st_mtime));

								sprintf(buffer, "%s%s%s%s%s%s%s%s%s%s   1 root  root        %llu %s %s\r\n",
									((buf.st_mode & S_IFDIR) != 0) ? "d" : "-",
									((buf.st_mode & S_IRUSR) != 0) ? "r" : "-",
									((buf.st_mode & S_IWUSR) != 0) ? "w" : "-",
									((buf.st_mode & S_IXUSR) != 0) ? "x" : "-",
									((buf.st_mode & S_IRGRP) != 0) ? "r" : "-",
									((buf.st_mode & S_IWGRP) != 0) ? "w" : "-",
									((buf.st_mode & S_IXGRP) != 0) ? "x" : "-",
									((buf.st_mode & S_IROTH) != 0) ? "r" : "-",
									((buf.st_mode & S_IWOTH) != 0) ? "w" : "-",
									((buf.st_mode & S_IXOTH) != 0) ? "x" : "-",
									(unsigned long long)buf.st_size, tstr, entry.d_name);

								ssend(data_s, buffer);
							}

							lv2FsCloseDir(fd);
							if(strlen(tempcwd)>6)
							{
								uint32_t blockSize;
								uint64_t freeSize;
								char tempstr[128];
								if(strchr(tempcwd+1, '/'))
									tempcwd[strchr(tempcwd+1, '/')-tempcwd]=0;
								cellFsGetFreeSize(tempcwd, &blockSize, &freeSize);
								sprintf(tempstr, "226 Transfer complete [%s] [ %i MB free ]\r\n", tempcwd, (blockSize*freeSize)>>20);
								ssend(conn_s, tempstr);
							}
							else
								ssend(conn_s, "226 Transfer complete\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot access directory\r\n");
						}
					}
					else
					{
						ssend(conn_s, "425 No data connection\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "MLSD") == 0)
				{
					if(data_s > 0)
					{
						char tempcwd[1024];
						strcpy(tempcwd, cwd);

						if(split == 1)
						{
							absPath(tempcwd, param, cwd);
						}

						Lv2FsFile fd;

						if(lv2FsOpenDir(isDir(tempcwd) ? tempcwd : cwd, &fd) == 0)
						{
							ssend(conn_s, "150 Accepted data connection\r\n");

							Lv2FsDirent entry;
							u64 read_e;

							while(lv2FsReadDir(fd, &entry, &read_e) == 0 && read_e > 0)
							{
								if(!strcmp(entry.d_name, "app_home") || !strcmp(entry.d_name, "host_root")) continue;
								char filename[512];
								absPath(filename, entry.d_name, cwd);

								Lv2FsStat buf;
								lv2FsStat(filename, &buf);

								char tstr[16];
								strftime(tstr, 15, "%Y%m%d%H%M%S", localtime(&buf.st_mtime));

								char dirtype[2];
								if(strcmp(entry.d_name, ".") == 0)
								{
									dirtype[0] = 'c';
								}
								else
								if(strcmp(entry.d_name, "..") == 0)
								{
									dirtype[0] = 'p';
								}
								else
								{
									dirtype[0] = '\0';
								}

								dirtype[1] = '\0';

								sprintf(buffer, "type=%s%s;siz%s=%llu;modify=%s;UNIX.mode=0%i%i%i;UNIX.uid=root;UNIX.gid=root; %s\r\n",
									dirtype,
									((buf.st_mode & S_IFDIR) != 0) ? "dir" : "file",
									((buf.st_mode & S_IFDIR) != 0) ? "d" : "e", (unsigned long long)buf.st_size, tstr,
									(((buf.st_mode & S_IRUSR) != 0) * 4 + ((buf.st_mode & S_IWUSR) != 0) * 2 + ((buf.st_mode & S_IXUSR) != 0) * 1),
									(((buf.st_mode & S_IRGRP) != 0) * 4 + ((buf.st_mode & S_IWGRP) != 0) * 2 + ((buf.st_mode & S_IXGRP) != 0) * 1),
									(((buf.st_mode & S_IROTH) != 0) * 4 + ((buf.st_mode & S_IWOTH) != 0) * 2 + ((buf.st_mode & S_IXOTH) != 0) * 1),
									entry.d_name);

								ssend(data_s, buffer);
							}

							lv2FsCloseDir(fd);
							if(strlen(tempcwd)>6)
							{
								uint32_t blockSize;
								uint64_t freeSize;
								char tempstr[128];
								if(strchr(tempcwd+1, '/'))
									tempcwd[strchr(tempcwd+1, '/')-tempcwd]=0;
								cellFsGetFreeSize(tempcwd, &blockSize, &freeSize);
								sprintf(tempstr, "226 Transfer complete [%s] [ %i MB free ]\r\n", tempcwd, (blockSize*freeSize)>>20);
								ssend(conn_s, tempstr);
							}
							else
								ssend(conn_s, "226 Transfer complete\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot access directory\r\n");
						}
					}
					else
					{
						ssend(conn_s, "425 No data connection\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "STOR") == 0)
				{
					if(data_s > 0)
					{
						if(split == 1)
						{
							char filename[512];
							absPath(filename, param, cwd);

							ssend(conn_s, "150 Accepted data connection\r\n");

							tv.tv_sec = 65;
							tv.tv_usec = 0;
							setsockopt(conn_s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

							int rr=recvfile(data_s, filename, rest);

							if(rr == 0)
							{
								ssend(conn_s, "226 Transfer complete\r\n");
							}
							else
							{
								sprintf(param, "451 Transfer failed (ERROR %X)\r\n", rr);
								ssend(conn_s, param);
							}
						}
						else
						{
							ssend(conn_s, "501 No file specified\r\n");
						}
					}
					else
					{
						ssend(conn_s, "425 No data connection\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "RETR") == 0)
				{
					if(data_s > 0)
					{
						if(split == 1)
						{
							char filename[512];
							absPath(filename, param, cwd);

							if(exists(filename) == 0)
							{
								ssend(conn_s, "150 Accepted data connection\r\n");
								tv.tv_sec = 65;
								tv.tv_usec = 0;
								setsockopt(conn_s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

								int rr=sendfile(data_s, filename, rest);

								if( rr == 0)
								{
									ssend(conn_s, "226 Transfer complete\r\n");
								}
								else
								{
									sprintf(param, "451 Transfer failed (ERROR %X)\r\n", rr);
									ssend(conn_s, param);
								}
							}
							else
							{
								ssend(conn_s, "550 File does not exist\r\n");
							}
						}
						else
						{
							ssend(conn_s, "501 No file specified\r\n");
						}
					}
					else
					{
						ssend(conn_s, "425 No data connection\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "PWD") == 0)
				{
					sprintf(buffer, "257 \"%s\" is the current directory\r\n", cwd);
					ssend(conn_s, buffer);
				}
				else
				if(strcasecmp(cmd, "TYPE") == 0)
				{
					ssend(conn_s, "200 TYPE command successful\r\n");
					dataactive = 1;
				}
				else
				if(strcasecmp(cmd, "REST") == 0)
				{
					if(split == 1)
					{
						ssend(conn_s, "350 REST command successful\r\n");
						rest = atoi(param);
						dataactive = 1;
					}
					else
					{
						ssend(conn_s, "501 No restart point\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "DELE") == 0)
				{
					if(split == 1)
					{
						char filename[512];
						absPath(filename, param, cwd);

						if(unlink(filename) == 0)
						{
							ssend(conn_s, "250 File successfully deleted\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot delete file\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No filename specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "MKD") == 0)
				{
					if(split == 1)
					{
						char filename[512];
						absPath(filename, param, cwd);

						if(lv2FsMkdir(filename, 0755) == 0)
						{
							sprintf(buffer, "257 \"%s\" was successfully created\r\n", param);
							ssend(conn_s, buffer);
						}
						else
						{
							ssend(conn_s, "550 Cannot create directory\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No filename specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "RMD") == 0)
				{
					if(split == 1)
					{
						char filename[512];
						absPath(filename, param, cwd);

						if(lv2FsRmdir(filename) == 0)
						{
							ssend(conn_s, "250 Directory was successfully removed\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot remove directory\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No filename specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "RNFR") == 0)
				{
					if(split == 1)
					{
						absPath(rnfr, param, cwd);

						if(exists(rnfr) == 0)
						{
							ssend(conn_s, "350 RNFR accepted - ready for destination\r\n");
						}
						else
						{
							ssend(conn_s, "550 RNFR failed - file does not exist\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No file specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "RNTO") == 0)
				{
					if(split == 1)
					{
						char rnto[512];
						absPath(rnto, param, cwd);

						if(lv2FsRename(rnfr, rnto) == 0)
						{
							ssend(conn_s, "250 File was successfully renamed or moved\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot rename or move file\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No file specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "SITE") == 0)
				{
					if(split == 1)
					{
						char param2[512];
						split = ssplit(param, cmd, 31, param2, 511);

						if(strcasecmp(cmd, "CHMOD") == 0)
						{
							if(split == 1)
							{
								char temp[4], filename[512];
								split = ssplit(param2, temp, 3, filename, 511);

								if(split == 1)
								{
									char perms[5];
									sprintf(perms, "0%s", temp);

									char absFilePath[1024]; // place-holder for absolute path
									absPath(absFilePath, filename, cwd); // making sure that we use the absolute path

									//tested and working for both dir and files :0)
									if(lv2FsChmod(absFilePath, strtol(perms, NULL, 8)) == 0) //cleaned up
									{
										ssend(conn_s, "250 File permissions successfully set\r\n");
									}
									else
									{
										ssend(conn_s, "550 Cannot set file permissions\r\n");
									}
								}
								else
								{
									ssend(conn_s, "501 Not enough parameters\r\n");
								}
							}
							else
							{
								ssend(conn_s, "501 No parameters given\r\n");
							}
						}
						else
						if(strcasecmp(cmd, "HELP") == 0)
						{
							ssend(conn_s, "214-Special commands:\r\n");
							ssend(conn_s, " SITE QUITMM - Exit to XMB\r\n");
							ssend(conn_s, " SITE RESTARTMM - Restart the application\r\n");
							ssend(conn_s, " SITE SHUTDOWN - Shutdown the PS3 system\r\n");
							ssend(conn_s, " SITE RESTART - Restart the PS3 system\r\n");
							ssend(conn_s, " SITE HELP - Show this message\r\n");
							ssend(conn_s, "214 End\r\n");
						}
						else
						if(strcasecmp(cmd, "QUITMM") == 0)
						{
							ssend(conn_s, "221 Exiting to XMB\r\n");
							exitapp = 1;
							exit(0);
						}
						else
						if(strcasecmp(cmd, "RESTARTMM") == 0)
						{
							ssend(conn_s, "221 Restarting\r\n");
							restart_request=1;
							exitapp = 1;
						}
						else
						if(strcasecmp(cmd, "SHUTDOWN") == 0)
						{
							ssend(conn_s, "221 Shutting down PS3\r\n");
							exitapp = 1;
							{system_call_4(379,0x1100,0,0,0);}
							exit(0);
						}
						else
						if(strcasecmp(cmd, "RESTART") == 0)
						{
							ssend(conn_s, "221 Restarting PS3\r\n");
							exitapp = 1;
							{system_call_4(379,0x1200,0,0,0);}
							exit(0);
						}
						else
						{
							ssend(conn_s, "500 Unknown SITE command\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No SITE command specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "NOOP") == 0)
				{
					ssend(conn_s, "200 NOOP command successful\r\n");
				}
				else
				if(strcasecmp(cmd, "NLST") == 0)
				{
					if(data_s > 0)
					{
						char tempcwd[1024];
						strcpy(tempcwd, cwd);

						if(split == 1)
						{
							absPath(tempcwd, param, cwd);
						}

						Lv2FsFile fd;

						if(lv2FsOpenDir(isDir(tempcwd) ? tempcwd : cwd, &fd) == 0)
						{
							ssend(conn_s, "150 Accepted data connection\r\n");

							Lv2FsDirent entry;
							u64 read_e;

							while(lv2FsReadDir(fd, &entry, &read_e) == 0 && read_e > 0)
							{
								if(!strcmp(entry.d_name, "app_home") || !strcmp(entry.d_name, "host_root")) continue;
								sprintf(buffer, "%s\r\n", entry.d_name);
								ssend(data_s, buffer);
							}

							lv2FsCloseDir(fd);
							ssend(conn_s, "226 Transfer complete\r\n");
						}
						else
						{
							ssend(conn_s, "550 Cannot access directory\r\n");
						}
					}
					else
					{
						ssend(conn_s, "425 No data connection\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "MLST") == 0)
				{
					char tempcwd[1024];
					strcpy(tempcwd, cwd);

					if(split == 1)
					{
						absPath(tempcwd, param, cwd);
					}

					Lv2FsFile fd;

					if(lv2FsOpenDir(isDir(tempcwd) ? tempcwd : cwd, &fd) == 0)
					{
						ssend(conn_s, "250-Directory Listing:\r\n");

						Lv2FsDirent entry;
						u64 read_e;

						while(lv2FsReadDir(fd, &entry, &read_e) == 0 && read_e > 0)
						{
							if(!strcmp(entry.d_name, "app_home") || !strcmp(entry.d_name, "host_root")) continue;
							char filename[512];
							absPath(filename, entry.d_name, cwd);

							Lv2FsStat buf;
							lv2FsStat(filename, &buf);

							char tstr[16];
							strftime(tstr, 15, "%Y%m%d%H%M%S", localtime(&buf.st_mtime));

							char dirtype[2];
							if(strcmp(entry.d_name, ".") == 0)
							{
								dirtype[0] = 'c';
							}
							else
							if(strcmp(entry.d_name, "..") == 0)
							{
								dirtype[0] = 'p';
							}
							else
							{
								dirtype[0] = '\0';
							}

							dirtype[1] = '\0';

							sprintf(buffer, " type=%s%s;siz%s=%llu;modify=%s;UNIX.mode=0%i%i%i;UNIX.uid=root;UNIX.gid=root; %s\r\n",
								dirtype,
								((buf.st_mode & S_IFDIR) != 0) ? "dir" : "file",
								((buf.st_mode & S_IFDIR) != 0) ? "d" : "e", (unsigned long long)buf.st_size, tstr,
								(((buf.st_mode & S_IRUSR) != 0) * 4 + ((buf.st_mode & S_IWUSR) != 0) * 2 + ((buf.st_mode & S_IXUSR) != 0) * 1),
								(((buf.st_mode & S_IRGRP) != 0) * 4 + ((buf.st_mode & S_IWGRP) != 0) * 2 + ((buf.st_mode & S_IXGRP) != 0) * 1),
								(((buf.st_mode & S_IROTH) != 0) * 4 + ((buf.st_mode & S_IWOTH) != 0) * 2 + ((buf.st_mode & S_IXOTH) != 0) * 1),
								entry.d_name);

							ssend(conn_s, buffer);
						}

						lv2FsCloseDir(fd);
						ssend(conn_s, "250 End\r\n");
					}
					else
					{
						ssend(conn_s, "550 Cannot access directory\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "QUIT") == 0 || strcasecmp(cmd, "BYE") == 0)
				{
					ssend(conn_s, "221 Sayonara!\r\n");
					connactive = 0;
				}
				else
				if(strcasecmp(cmd, "FEAT") == 0)
				{
					ssend(conn_s, "211-Extensions supported:\r\n");

					static char feat_cmds[12][64] =
					{
						"PASV",
						"PORT",
						"SIZE",
						"CDUP",
						"MLSD",
						"MDTM",
						"ABOR",
						"MLST type*;size*;modify*;UNIX.mode*;UNIX.uid*;UNIX.gid*;",
						"REST STREAM",
						"SITE CHMOD",
						"SITE PASSWD",
						"SITE EXITAPP"
					};

					const int feat_cmds_count = 12;

					for(int i = 0; i < feat_cmds_count; i++)
					{
						sprintf(buffer, " %s\r\n", feat_cmds[i]);
						ssend(conn_s, buffer);
					}

					ssend(conn_s, "211 End\r\n");
				}
				else
				if(strcasecmp(cmd, "ABOR") == 0)
				{
					sclose(&data_s);
					ssend(conn_s, "226 ABOR command successful\r\n");
				}
				else
				if(strcasecmp(cmd, "SIZE") == 0)
				{
					if(split == 1)
					{
						char filename[512];
						absPath(filename, param, cwd);

						Lv2FsStat buf;

						if(lv2FsStat(filename, &buf) == 0)
						{
							sprintf(buffer, "213 %llu\r\n", (unsigned long long)buf.st_size);
							ssend(conn_s, buffer);
							dataactive = 1;
						}
						else
						{
							ssend(conn_s, "550 File does not exist\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No file specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "SYST") == 0)
				{
					ssend(conn_s, "215 UNIX Type: L8\r\n");
				}
				else
				if(strcasecmp(cmd, "MDTM") == 0)
				{
					if(split == 1)
					{
						char filename[512];
						absPath(filename, param, cwd);

						Lv2FsStat buf;

						if(lv2FsStat(filename, &buf) == 0)
						{
							char tstr[16];
							strftime(tstr, 15, "%Y%m%d%H%M%S", localtime(&buf.st_mtime));
							sprintf(buffer, "213 %s\r\n", tstr);
							ssend(conn_s, buffer);
						}
						else
						{
							ssend(conn_s, "550 File does not exist\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No file specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "USER") == 0 || strcasecmp(cmd, "PASS") == 0)
				{
					ssend(conn_s, "230 You are already logged in\r\n");
				}
				else
				if(strcasecmp(cmd, "OPTS") == 0
				|| strcasecmp(cmd, "REIN") == 0 || strcasecmp(cmd, "ADAT") == 0
				|| strcasecmp(cmd, "AUTH") == 0 || strcasecmp(cmd, "CCC") == 0
				|| strcasecmp(cmd, "CONF") == 0 || strcasecmp(cmd, "ENC") == 0
				|| strcasecmp(cmd, "EPRT") == 0 || strcasecmp(cmd, "EPSV") == 0
				|| strcasecmp(cmd, "LANG") == 0 || strcasecmp(cmd, "LPRT") == 0
				|| strcasecmp(cmd, "LPSV") == 0 || strcasecmp(cmd, "MIC") == 0
				|| strcasecmp(cmd, "PBSZ") == 0 || strcasecmp(cmd, "PROT") == 0
				|| strcasecmp(cmd, "SMNT") == 0 || strcasecmp(cmd, "STOU") == 0
				|| strcasecmp(cmd, "XRCP") == 0 || strcasecmp(cmd, "XSEN") == 0
				|| strcasecmp(cmd, "XSEM") == 0 || strcasecmp(cmd, "XRSQ") == 0
				|| strcasecmp(cmd, "STAT") == 0)
				{
					ssend(conn_s, "502 Command not implemented\r\n");
				}
				else
				{
					ssend(conn_s, "500 Unrecognized command\r\n");
				}

				if(dataactive == 1)
				{
					dataactive = 0;
				}
				else
				{
					sclose(&data_s);
					rest = 0;
				}
			}
			else
			{
				// available commands when not logged in
				if(strcasecmp(cmd, "USER") == 0)
				{
					if(split == 1)
					{
						strcpy(user, param);
						sprintf(buffer, "331 User %s OK. Password required\r\n", param);
						ssend(conn_s, buffer);
					}
					else
					{
						ssend(conn_s, "501 No user specified\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "PASS") == 0)
				{
					if(split == 1)
					{
						if(DISABLE_PASS || (strcmp(D_USER, user) == 0 && strcmp(userpass, param) == 0))
						{
							ssend(conn_s, "230 Welcome!\r\n");
							loggedin = 1;
						}
						else
						{
							ssend(conn_s, "430 Invalid username or password\r\n");
						}
					}
					else
					{
						ssend(conn_s, "501 No password given\r\n");
					}
				}
				else
				if(strcasecmp(cmd, "QUIT") == 0 || strcasecmp(cmd, "BYE") == 0)
				{
					ssend(conn_s, "221 Bye!\r\n");
					connactive = 0;
				}
				else
				{
					ssend(conn_s, "530 Not logged in\r\n");
				}
			}
			timeout=0;
			ss_timer=0;
			ss_timer_last=time(NULL);
		}

		timeout++;
		if(timeout>1)
		{
			timeout=0;
			ssend(conn_s, "421 Bye! (Timeout)\r\n");
			connactive = 0;
			break;
		}

		sys_timer_usleep(1668);
		sys_ppu_thread_yield();
	}

	sclose(&conn_s);
	sclose(&data_s);
	sys_ppu_thread_exit(0);
}

static void handleconnections(u64 unused)
{
	(void) unused;
	int list_s = slisten(FTPPORT, 2);

	if(list_s > 0)
	{
		while(exitapp == 0 && !app_shutdown)
		{
			int conn_s;
			if((conn_s = accept(list_s, NULL, NULL)) > 0)
			{
				sys_ppu_thread_t id;
				sys_ppu_thread_create(&id, handleclient, (u64)conn_s, 490, 131072, 0, "ClientCmdHandler"); //BUFFER_SIZE
			}

			sys_timer_usleep(1668);
			sys_ppu_thread_yield();
		}

		sclose(&list_s);
	}

	sys_ppu_thread_exit(0);
}

int main_ftp(u8 _mode)
{
	if(_mode) { exitapp=1; return 0;}
	// initialize libnet
	//netInitialize();

	// handle system events
	//sysRegisterCallback(EVENT_SLOT0, eventHandler, NULL);

	// format version string
	char version[32];
	sprintf(version, "Version %s", VERSION);

	// check if dev_flash is mounted rw
	//int rwflashmount = (exists("/dev_blind") == 0 || exists("/dev_rwflash") == 0 || exists("/dev_fflash") == 0 || exists("/dev_Alejandro") == 0);

	// start listening for connections
	sys_ppu_thread_t id;
	sys_ppu_thread_create(&id, handleconnections, 0, 1600, 0x1000, 0, "ServerConnectionHandler");

	return 0;
}

