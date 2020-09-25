#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("/dev/forytwo char device");
MODULE_AUTHOR("iomonad");

#define DEVICE_NAME "fortytwo"

static int major;
static int busy;
static int done;

static char buffer[] = "ctrouill\n";

static int driver_open(struct inode *ip, struct file *fp)
{
	printk(KERN_ALERT "User opened driver.");
	return 0;
}

static ssize_t driver_read(struct file *fp, char __user *user, size_t size,
			   loff_t *loff)
{
    ssize_t len;

    if (done)
	return 0;
    busy = 1;
    len = sizeof(buffer) / sizeof(buffer[0]);
    if (copy_to_user(user, buffer, len)) /* Kernel space to userspace helper */
	printk(KERN_ALERT "Copy kernel to user buffer error");
    done = 1;
    return len;			/* Return Buffer len */
}

static ssize_t driver_write(struct file *fp, const char __user *buffer,
			    size_t size, loff_t *loff)
{
    char input[128];

    copy_from_user(input, buffer, 128);
    printk(KERN_INFO "Read from user: \"%s\"", input);
    if ((strncmp(input, "ctrouill", 8)) == 0) {
	return strlen(buffer);	/* Correct case */
    } else {
	return -EINVAL; /* write error: invalid argument */
    }
}

static int driver_release(struct inode *ip, struct file *fp)
{
    /* ON RELEASE, RESET STATE */
    done = busy = 0x0;
    return 0;
}


static struct file_operations fops = { .owner = THIS_MODULE,
                                       .open = driver_open,
				       .read = driver_read,
				       .write = driver_write,
				       .release = driver_release
};

static __init int initialize(void)
{
    done = busy = 0x0;		/* INIT STATE */
    if ((major = register_chrdev(0, DEVICE_NAME, &fops)) < 0) {
	printk(KERN_ALERT "Error while creating char device: %d", major);
    }
    return 0;
}

static __exit void destroy(void)
{
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "Removed /dev/fortytwo device");
}

module_init(initialize);
module_exit(destroy);
