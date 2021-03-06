#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My eudyptula debugfs implementation");
MODULE_AUTHOR("iomonad");

#define DEBUGFS_FOLDER_NAME "fortytwo"
#define KERN_PSIZE 4096

static struct dentry *mentry = NULL;

/* `id` file implementation */

static ssize_t id_busy;
static ssize_t id_done;

static ssize_t id_read(struct file *fp, char __user *user, size_t size,
		       loff_t *loff)
{
	ssize_t len;
	static char local_buffer[] = "ctrouill\n";

	id_busy = 1;
	len = sizeof(local_buffer) / sizeof(local_buffer[0]);
	if (id_done)
	    return 0;
	if (copy_to_user(user, local_buffer, len)) {
		printk(KERN_ALERT "COPY ON USER");
		return -EAGAIN;
	}
	id_done = 1;
	return len;
}

static ssize_t id_write(struct file *fp, const char __user *buffer, size_t size,
			loff_t *loff)
{
	char input[128];

	copy_from_user(input, buffer, 128);
	printk(KERN_INFO "Got buffer: \"%s\"", input);
	if ((strncmp(input, "ctrouill", 8)) == 0) {
		return strlen(buffer);
	} else {
		return -EINVAL; /* write error: invalid argument */
	}
}

static int id_release(struct inode *ip, struct file *fp)
{
        id_done = id_busy = 0x0;
	return 0;
}

static struct file_operations id_fops = { .read = id_read, .write = id_write, .release = id_release};

/* `jiffies` file implementation */

static int jiffies_done;

static ssize_t jiffies_read(struct file *fp, char __user *user, size_t size,
			    loff_t *loff)
{
	char buffer[128];
	ssize_t len;
	unsigned long jiffie_now;

	if (jiffies_done)
	    return 0;
	jiffie_now = jiffies;
	sprintf(buffer, "%lu\n", jiffie_now);
	len = sizeof(buffer) / sizeof(buffer[0]);
	if (copy_to_user(user, buffer, len)) {
		printk(KERN_ALERT "COPY ON USER");
		return -EAGAIN;
	}
	jiffies_done = 1;
	return len;
}

static int jiffies_release(struct inode *ip, struct file *fp)
{
    jiffies_done = 0x0;
    return 0;
}

static struct file_operations jiffies_fops = { .read = jiffies_read, .release = jiffies_release };

/* `foo` file implementation */

static ssize_t foo_busy;
static ssize_t foo_done;
static char shared_foo_buffer[KERN_PSIZE] = {}; /* HC BUFFER */

static ssize_t foo_read(struct file *fp, char __user *user, size_t size,
			loff_t *loff)
{
	ssize_t len;

	if (foo_done)
		return 0;
	len = sizeof(shared_foo_buffer) / sizeof(shared_foo_buffer[0]);
	if (copy_to_user(user, shared_foo_buffer, len)) {
		printk(KERN_ALERT "COPY ON USER");
		return -EAGAIN;
	}
	foo_done = 1;
	return len;
}

static ssize_t foo_write(struct file *fp, const char __user *buffer,
			 size_t size, loff_t *loff)
{
	size_t len;
	char *temporary;

	foo_busy = 1; /* Reset on release cb */
	if ((temporary = (char *)kmalloc(KERN_PSIZE, GFP_KERNEL)) == NULL) {
		return 0;
	}
	memset(shared_foo_buffer, 0x0, KERN_PSIZE);
	len = sizeof(shared_foo_buffer) / sizeof(shared_foo_buffer[0]);
	copy_from_user(temporary, buffer,
		       KERN_PSIZE); /* Errors need to be checked */
	shared_foo_buffer[KERN_PSIZE] = '\0';
	memcpy(shared_foo_buffer, temporary, KERN_PSIZE);
	return len;
}

static int foo_release(struct inode *ip, struct file *fp)
{
	foo_done = foo_busy = 0x0;
	return 0;
}

static struct file_operations foo_fops = { .read = foo_read,
					   .write = foo_write,
					   .release = foo_release };

/** MODULE CORE */

static __init int initialize(void)
{
	struct dentry *id, *jiffies, *foo;

	foo_done = foo_busy = 0x0;
	if ((mentry = debugfs_create_dir(DEBUGFS_FOLDER_NAME, NULL)) == NULL) {
		printk(KERN_ALERT "Error while creating debugfs root folder");
		return 1;
	}
	if ((id = debugfs_create_file("id", 666, mentry, NULL, &id_fops)) ==
	    NULL) {
		return 1;
	}
	if ((jiffies = debugfs_create_file("jiffies", 444, mentry, NULL,
					   &jiffies_fops)) == NULL) {
		return 1;
	}
	if ((foo = debugfs_create_file("foo", 604, mentry, NULL, &foo_fops)) ==
	    NULL) {
		return 1;
	}
	return 0;
}

static __exit void destroy(void)
{
        return debugfs_remove_recursive(mentry);
}

module_init(initialize);
module_exit(destroy);
