/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>	/* for put_user */
#include <linux/random.h>

/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 20		/* Max length of the message from the device */

static int Major = 293;
static int Device_Open = 0;
static char msg[BUF_LEN];
static unsigned char key[16];

static int msg_write_counter = 0;
static int has_key = 0;

static char *msg_Ptr;

static int read_counter = 0;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
    register_chrdev(Major, DEVICE_NAME, &fops);
	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void){
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open>0)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);


	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}


static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		
	module_put(THIS_MODULE);

	return 0;
}


static ssize_t device_read(struct file *filp,	
			   char *buffer,
			   size_t length,	
			   loff_t * offset){
	
	int bytes_read = 0;

	
	if (*msg_Ptr == 0)
		return 0;

	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {
	
	if(has_key == 0){
		has_key = 1;
		get_random_bytes(key, sizeof(key));
	}


	
	printk("msg: %s \n", msg);
	printk("len: %d \n", len);

	copy_from_user(msg+msg_write_counter,buff,len)
	msg_write_counter+=len;
	msg_Ptr = msg;
	printk("msg: %s \n", msg);

	return len;
}