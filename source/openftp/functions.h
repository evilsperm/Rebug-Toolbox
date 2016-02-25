// General functions

#ifndef _openps3ftp_generalfunc_
#define _openps3ftp_generalfunc_

#define isempty(str) (str[0] == '\0')

void absPath(char* absPath, const char* path, const char* cwd);
int exists(const char* path);
int isDir(const char* path);

//void stoupper(char *s);
int ssplit(const char* str, char* left, int lmaxlen, char* right, int rmaxlen);

#endif /* _openps3ftp_generalfunc_ */
