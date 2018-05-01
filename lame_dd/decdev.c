#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>	/* for put_user */
#include <linux/random.h>
#include <linux/fs.h>

void cleanup_module(void);
int init_module(void);
static int device_release(struct inode *, struct file *);
static int device_open(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "decdev"	
#define BUF_LEN 320	
#define KEY_LEN 16	

static int Major = 601;

static char msg[BUF_LEN];
static char *msg_Ptr;

static int lock = 0;

// static unsigned char key[KEY_LEN];
extern unsigned char orig_key[KEY_LEN];

static int msg_write_counter = 0;
static int key_counter = 0;
// static int has_key = 0;

static struct file_operations fops = {
	.write = device_write,
	.release = device_release,
	.open = device_open,
	.read = device_read
};

int init_module(void){
    register_chrdev(Major, DEVICE_NAME, &fops);
	return SUCCESS;
}

void cleanup_module(void){
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file){

	if (lock>0){
		printk("more than one user accesing it. It is bad \n");
		return -EBUSY;
	}
	lock++;

	try_module_get(THIS_MODULE);
	return SUCCESS;
}


static int device_release(struct inode *inode, struct file *file){
	lock--;		
	module_put(THIS_MODULE);
	return 0;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	
	static int counter;

	if (*msg_Ptr == 0 || msg_write_counter == 0) {
		// check null and premature reading
		printk("From decdev: please write something before reading \n");
		return 0;
	}
	counter = 0;

	while (length > 0 && *msg_Ptr && msg_write_counter >= 0) {
		length--;
		put_user(*(msg_Ptr++), buffer++);
		counter++;
		msg_write_counter--;
	}


	return counter;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {
	
	static int i = 0;
	static int temp_msg_write_counter;
	temp_msg_write_counter = msg_write_counter;

	if(temp_msg_write_counter+len >= BUF_LEN) {
		printk("From decdev: Read from the buffer, before writing anymore");
		printk("\n");
		return -1;
	}

	printk("In side decdev: write: key is: %s: \n", orig_key);

	// copying buff from user to kernel
	copy_from_user(msg+msg_write_counter,buff,len);

	// contains the new pointer in the message array
	msg_write_counter+=len;

	printk("In decdev: the msg is: %s \n", msg);
	
	// decryption of the msg it has received right now
    for(i = temp_msg_write_counter; i < msg_write_counter; i++) {
        
		char temp = msg[i];
        msg[i] = msg[i] ^ orig_key[key_counter];
		orig_key[key_counter] = temp;
		++key_counter;
		key_counter = key_counter % KEY_LEN;

    }

	printk("In decdev: the decrypted msg is: %s \n", msg);

	msg_Ptr = msg;
	return len;
}