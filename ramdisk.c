/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAX_NAME 512

//Globals

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

typedef struct __data {
	char name[MAX_NAME];
	int  isdir;
	struct stat st;
	int data_size;
} Ndata;

typedef struct element {
	Ndata data;
	char * filedata;
	struct element * parent;
	struct element * child;
	struct element * next;
} Node;

long freememory;
Node * Root;


static int ram_getattr(const char *path, struct stat *stbuf)
{
	FILE *fp;
	fp = fopen("/home/agupta27/log.txt","a+");
	fprintf(fp, "%s\n", "getattr");
	fprintf(fp, "%s\n", path);
	fclose(fp);
	
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		//stbuf->st_mode = S_IFDIR | 0755;
		//stbuf->st_nlink = 2;
		*stbuf = Root->data.st;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else
		res = -ENOENT;

	return res;
}

static int ram_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	FILE *fp;
	fp = fopen("/home/agupta27/log.txt","a+");
	fprintf(fp, "%s\n", "readdir");
	fprintf(fp, "%s\n", path);
	fclose(fp);

	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;
}

static int ram_open(const char *path, struct fuse_file_info *fi)
{
	FILE *fp;
	fp = fopen("/home/agupta27/log.txt","a+");
	fprintf(fp, "%s\n", "open");
	fprintf(fp, "%s\n", path);
	fclose(fp);
	
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int ram_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	FILE *fp;
	fp = fopen("/home/agupta27/log.txt","a+");
	
	fprintf(fp, "%s\n", "read");
	fprintf(fp, "%s\n", path);
	fclose(fp);

	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations hello_oper = {
	.getattr	= ram_getattr,
	.readdir	= ram_readdir,
	.open		= ram_open,
	.read		= ram_read,
};

int main(int argc, char *argv[])
{
	FILE *fp;
	fp = fopen("/home/agupta27/log.txt","w+");
	fclose(fp);

	//for extra credit
	char filedump[MAX_NAME];

	if (argc < 3) {
		fprintf(stderr, "ramdisk:Too few arguments\n");
		fprintf(stderr, "ramdisk <mount_point> <size>");
		return -1;
	}

	if (argc > 4) {
		fprintf(stderr, "ramdisk:Too many arguments\n");
		fprintf(stderr, "ramdisk <mount_point> <size> [<filename>]");
		return -1;
	}

	if (argc == 4) {
		strncpy(filedump, argv[3], MAX_NAME);
		argc--;
	}

	freememory = atol(argv[2]) * 1024 * 1024;
	if (freememory <= 0) {
		fprintf(stderr, "Invalid Memory Size\n");
		return -1;
	}

	//initialize the root


	Root = (Node *)calloc(1, sizeof(Node));
	strcpy(Root->data.name, "/");
	Root->data.isdir = 1;
	Root->data.st.st_nlink = 2;   // . and ..
	Root->data.st.st_uid = 0;
	Root->data.st.st_gid = 0;
	Root->data.st.st_mode = S_IFDIR |  0755; //755 is default directory permissions

	time_t T;
	time(&T);

	Root->data.st.st_atime = T;
	Root->data.st.st_mtime = T;
	Root->data.st.st_ctime = T;
	freememory = freememory - sizeof(Node); 

	return fuse_main(argc-1, argv, &hello_oper, NULL);
}
