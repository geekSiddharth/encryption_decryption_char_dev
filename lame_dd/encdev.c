
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
#define DEVICE_NAME "encdev"	
#define BUF_LEN 320	
#define KEY_LEN 16

static int Major = 600;
static char msg[BUF_LEN];
static unsigned char key[KEY_LEN];
static int lock = 0;

static unsigned char orig_key[KEY_LEN];

static int msg_write_counter = 0;
static int key_counter = 0;
static int has_key = 0;

static char *msg_Ptr;

static struct file_operations fops = {
	.write = device_write,
	.release = device_release,
	.open = device_open,
	.read = device_read
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

	if (lock>0){
		printk("more than one user accesing it. It is bad \n");
		return -EBUSY;
	}

	lock++;

	try_module_get(THIS_MODULE);

	return SUCCESS;
}


static int device_release(struct inode *inode, struct file *file)
{
	lock--;		
	module_put(THIS_MODULE);

	return 0;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	
	static int counter;

	if (*msg_Ptr == 0 ||  msg_write_counter == 0) {
		printk("From encdev: please write something before reading\n");
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

static ssize_t device_write (struct file *filp, const char *buff, size_t len, loff_t * off) {
	
	static int i;
	static int temp_msg_write_counter;
	temp_msg_write_counter = msg_write_counter;

	if(temp_msg_write_counter+len >= BUF_LEN) {
		printk("From encdev: Read from the buffer, before writing anymore");
		printk("\n");
		return -1;
	}

	if(has_key == 0){
		// random key initialisation
		get_random_bytes(key, sizeof(key));
		strncpy(orig_key,key,16);
		has_key = 1;
		key_counter = 0;
	}	

	copy_from_user(msg+msg_write_counter,buff,len);
	msg_write_counter+=len;

	printk("In encdev: msg is : %s \n", msg);
	
	// encrypting
    for(i = temp_msg_write_counter; i < msg_write_counter; i++) {

		if (msg[i] == '\0'){
			// handles EOF
			has_key = 0;
			key_counter = 0;
			msg_write_counter = 0;
			printk("Stuffs set to 0. Key will be resent in next write calls\n");
			return len;
		}
		
        msg[i] = msg[i] ^ key[key_counter];
		key[key_counter] = msg[i];

		++key_counter;
		key_counter = key_counter % KEY_LEN;

    }
	i = 0;

	printk("In encdev: encrypted msg is: %s \n", msg);

	// set to mst_Ptr for printing in read
	msg_Ptr = msg;

	return len;
}

EXPORT_SYMBOL(orig_key);