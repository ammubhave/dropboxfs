#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/hashtable.h>
#include <linux/bio.h>
#include <linux/elevator.h>
#include <linux/proc_fs.h>
#include "dropboxfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amol Bhave");

static int dropboxfs_major = 0;
module_param(dropboxfs_major, int, 0);
static int logical_block_size = 4096;//512;
module_param(logical_block_size, int, 0);
static int nsectors = 409600;
module_param(nsectors, int, 0);

#define DEVICE_NAME "dropbox"
#define KERNEL_SECTOR_SIZE	512

static struct request_queue *Queue;

struct dropboxfs_device {
	unsigned long size;
	spinlock_t lock;
	struct gendisk *gd;
} Device;

#define READER_HASH_TABLE_BITS 4
DECLARE_HASHTABLE(Reader, READER_HASH_TABLE_BITS);

/*static short int myshort = 1;
static int myint = 420;
static long int mylong = 9999;
static char *mystring = "blah";
static int myintArray[2] = { -1, -1 };
static int arr_argc = 0;

 
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits, 
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 *

module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");
module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(myint, "An integer");
module_param(mylong, long, S_IRUSR);
MODULE_PARM_DESC(mylong, "A long integer");
module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, "A character string");*/

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Device_Open = 0;

static int dropboxfs_open(struct block_device *dev, fmode_t fmode)
{
	//if (Device_Open)
	//	return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process closes the device file.
 */
static void dropboxfs_release(struct gendisk *disk, fmode_t fmode)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);
}

static void dropboxfs_subprocessinfo_cleanup(struct subprocess_info *info)
{
	//kfree(info);
	//if (info->argv[4])
	//	kfree(info->argv[4]);
	//kfree(info->argv[3]);

	/*if (info && info->argv)
		printk (KERN_ALERT "dbxfs: cleanup: 3: %p 4: %p 5: %p info: %p argv: %p envp: %p", info->argv[3], info->argv[4], info->argv[5], info, info->argv, info->envp);
	else if (info)
		printk (KERN_ALERT "dbxfs: cleanup: info: %p, argv: %p", info, info->argv);
	else
		printk (KERN_ALERT "dbxfs: cleanup: info: %p", info);
*/
	kfree(info->argv[5]);
	kfree(info->argv[4]);
	kfree(info->argv[3]);
	kfree(info->argv);
	kfree(info->envp);
}

static void dropboxfs_transfer(struct request *req) {
	sector_t sector = blk_rq_pos(req);
	unsigned long nsect = blk_rq_cur_sectors(req);
	char* buffer = bio_data(req->bio);
	int write = rq_data_dir(req);

	unsigned long offset = sector * KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect * KERNEL_SECTOR_SIZE;
	char **argv, **envp;
	struct subprocess_info *info;
	struct dropboxfs_reader_work *work;

	if ((offset + nbytes) > Device.size) {
		printk (KERN_NOTICE "dropboxfs: Beyond-end write (%ld %ld %ld)\n", offset, nbytes, Device.size);
		return;
	}

	/*
	if (write)
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);*/
	argv = kmalloc(sizeof(char*)*7, GFP_ATOMIC);
	envp = kmalloc(sizeof(char*)*4, GFP_ATOMIC);
	argv[0] = "/usr/bin/python2";
	argv[1] = "/home/ambhave/dropboxfs/front.py";
	argv[3] = kmalloc(11, GFP_ATOMIC);
	sprintf(argv[3], "%ld", (sector * KERNEL_SECTOR_SIZE)/logical_block_size);
	argv[4] = kmalloc(11, GFP_ATOMIC);
	sprintf(argv[4], "%ld", (nsect * KERNEL_SECTOR_SIZE)/logical_block_size);

	if (write) {
		char *src, *dst;
		unsigned long i;

		argv[2] = "w";
		argv[5] = kmalloc(sizeof(char) * 2 * nbytes, GFP_ATOMIC);

		src = buffer;
		dst = argv[5];
		for (i = 0; i < nbytes; i++) {
			if (*src == '\0') {
				*dst++ = '\\';
				*dst++ = '0';
				src++;
			} else if (*src == '\\') {
				*dst++ = '\\';
				*dst++ = *src++;
			} else
				*dst++ = *src++;
		}
		*dst = '\0';

		argv[6] = NULL;
	} else {
		argv[2] = "r";

		work = kmalloc(sizeof(struct dropboxfs_reader_work), GFP_ATOMIC);
		work->req = req;

		argv[5] = kmalloc(17, GFP_ATOMIC);
		sprintf(argv[5], "%p", work);

		printk("Adding to Reader: &work->node: %p key: %p", &work->node, work);

		hash_add(Reader, &work->node, (u64)work);

		printk(" .. done\n");

		argv[6] = NULL;
	}


	envp[0] = "HOME=/";
	envp[1] = "TERM=xterm";
	envp[2] = "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games";
	envp[3] = NULL;

	info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, dropboxfs_subprocessinfo_cleanup, NULL);

	call_usermodehelper_exec(info, UMH_NO_WAIT);

	//printk (KERN_ALERT "dbxfs: request: 3: %p 4: %p 5: %p info: %p argv: %p envp: %p", info->argv[3], info->argv[4], info->argv[5], info, info->argv, info->envp);

	/*kfree(info->argv[5]);
	kfree(info->argv[4]);
	kfree(info->argv[3]);
	kfree(info->argv);
	kfree(info->envp);*/
}

static void dropboxfs_request(struct request_queue *q)
{
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		if (req->cmd_type != REQ_TYPE_FS) {
			printk (KERN_NOTICE "Skip non-fs request\n");
			__blk_end_request_all(req, -EIO);
			req = blk_fetch_request(q);
			continue;
		}

		dropboxfs_transfer(req);

		printk (KERN_NOTICE "Req dir %d pos %ld, nr %d \n",
						  rq_data_dir(req),
						  blk_rq_pos(req), blk_rq_sectors(req));

		//if (!rq_data_dir(req))
		//	return;

		if (!rq_data_dir(req) || ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

int dropboxfs_revalidate_disk(struct gendisk *gd)
{
	return 0;
}

int dropboxfs_getgeo(struct block_device * dev, struct hd_geometry *geo) {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

int dropboxfs_ioctl (struct block_device *dev, fmode_t fmode, unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	printk("IOCTL fs cmd: %u arg: %p", cmd, (void*)arg);

	switch(cmd) {
    	case BLKGETSIZE:
	        printk ("rxd: debug: request = BLKGETSIZE\n" );
	        size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);         /* number of sectors which equate to 64 MB */

	        if (copy_to_user ((void __user *)arg, &size, sizeof (size)))
	            return -EFAULT;

	        return 0;
		case HDIO_GETGEO:
	        printk ("rxd: debug: request = HDIO_GETGEO\n" );
			/*
			* Get geometry: since we are a virtual device, we have to make
			* up something plausible.  So we claim 16 sectors, four heads,
			* and calculate the corresponding number of cylinders.  We set the
			* start of data at sector four.
			*/
			dropboxfs_getgeo(dev, &geo);
			if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
				return -EFAULT;
			return 0;
	}
 
	return -ENOTTY;
}

ssize_t dropboxack_write(struct file *file, const char *buf, size_t size, loff_t *offp)
{
	struct dropboxfs_data_from_user frmusr;
	struct dropboxfs_reader_work *work;
	//unsigned long flags;

	copy_from_user((void*)&frmusr, buf, sizeof(void*));
	//printk(KERN_ALERT "proc write %p %p %lu\n", frmusr.key, offp, size);
	//printk(KERN_ALERT "      %p %p", frmusr.key, frmusr.data);

//	switch(cmd) {
//		case IOCTL_SEND_DATA_DBX:
//			printk(KERN_ALERT "Got IOCTL_SEND_DATA_DBX cmd: %u %p", cmd, (void*)arg);
			hash_for_each_possible(Reader, work, node, (u64)frmusr.key) {
				if (work == frmusr.key) {
					//printk(KERN_ALERT "Removing from Reader: &work->node: %p key: %p\n", &work->node, work);
					//printk(KERN_ALERT "%s", frmusr.data);
					copy_from_user(bio_data(work->req->bio), buf + sizeof(void*), blk_rq_cur_sectors(work->req) * KERNEL_SECTOR_SIZE);

					//printk(KERN_ALERT " going in spin_lock_irqsave %lu\n", work->nsect * logical_block_size);
					//spin_lock_irqsave(work->q->queue_lock, flags);
					//printk(KERN_ALERT " out of __blk_end_request_cur\n");
					//__blk_end_request_all(work->req, 0);
					if(blk_end_request_cur(work->req, 0)) {
						dropboxfs_transfer(work->req);
					}
					//printk(KERN_ALERT " dropboxfs_request\n");
					//spin_unlock_irqrestore(work->q->queue_lock, flags);
					//printk(KERN_ALERT " out of spin_unlock_irqrestore\n");

					hash_del(&work->node);
					kfree(work);
					return size;
				}
			}
			printk(KERN_ALERT "IOCTL_SEND_DATA_DBX: Cannot find work: %p\n", frmusr.key);
			//tmp = (char*)arg;
			//copy_from_user()
			return -EFAULT;
//	}
 
	return size;// -ENOTTY;
}

struct block_device_operations bdops = {
	.owner 				= THIS_MODULE,
	.open 				= dropboxfs_open,
	.release 			= dropboxfs_release,
	.media_changed 		= NULL,
	.revalidate_disk 	= dropboxfs_revalidate_disk,
	.ioctl 				= dropboxfs_ioctl,
	.getgeo 			= dropboxfs_getgeo
};

struct file_operations fops = {
	.owner    = THIS_MODULE,
	.write    = dropboxack_write
};

static int __init dropboxfs_init(void)
{
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);

	Queue = blk_init_queue(dropboxfs_request, &Device.lock);
	if (Queue == NULL)
		goto out;

	dropboxfs_major = register_blkdev(0, DEVICE_NAME);
	if (dropboxfs_major < 0) {
		printk(KERN_ALERT "Registering block device failed with %d\n", dropboxfs_major);
		return -EBUSY;
	}
	proc_create("dropbox0", 0, NULL, &fops);
	hash_init(Reader);

	blk_queue_logical_block_size(Queue, logical_block_size);
	blk_queue_physical_block_size(Queue, logical_block_size);
	blk_queue_io_min(Queue, logical_block_size);
	blk_queue_max_hw_sectors(Queue, logical_block_size / KERNEL_SECTOR_SIZE);

	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;

	Device.gd->major = dropboxfs_major;
	Device.gd->minors = 16;
	Device.gd->first_minor = 0;
	Device.gd->fops = &bdops;
	Device.gd->private_data = &Device;
	snprintf (Device.gd->disk_name, 32, "dropbox0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);

	printk(KERN_INFO "I was assigned major number %d.\n", dropboxfs_major);
	printk(KERN_INFO "'mknod /dev/%s b %d 0'.\n", DEVICE_NAME, dropboxfs_major);
	printk(KERN_INFO "IOCTL_SEND_DATA_DBX = %lu\n", IOCTL_SEND_DATA_DBX);

	return 0;

out_unregister:
	unregister_blkdev(dropboxfs_major, DEVICE_NAME);
out:
	return -ENOMEM;
}

static void dropboxfs_exit(void)
{
	/* 
	 * Unregister the device 
	 */
	if (Device.gd) {
		del_gendisk(Device.gd);
		put_disk(Device.gd);
	}
	unregister_blkdev(dropboxfs_major, DEVICE_NAME);
	remove_proc_entry("dropbox0", NULL);
	if (Queue) {
		blk_cleanup_queue(Queue);
	}
}

module_init(dropboxfs_init);
module_exit(dropboxfs_exit);
