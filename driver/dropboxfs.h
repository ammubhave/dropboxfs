#ifndef DROPBOXFS_H
#define DROPBOXFS_H

#include <linux/ioctl.h>

#define DROPBOXFS_MAJOR_NO 251
//253
#define IOCTL_SEND_DATA_DBX _IOR(DROPBOXFS_MAJOR_NO, 0, char *)

struct dropboxfs_reader_work {
	struct request* req;
	struct request_queue* q;
	sector_t sector;
	unsigned long nsect;
	char *buffer;

	struct hlist_node node;
};

struct dropboxfs_data_from_user {
	struct dropboxfs_reader_work *key;
};

#endif
