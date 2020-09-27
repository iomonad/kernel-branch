/*
 * Description: Kernel module that list mount
 * points on the target name, with the associated
 * name.
 *
 * (c) iomonad - <iomonad@riseup.net>
 * See: https://github.com/iomonad/kernel-branch/
 *
 * Notes:
 *  - Since 2.2 kernel version, `proc_register_dynamic`
 *    is removed in favor of the regular proc_register;
 *    To have the same behaviour, put zero in the inode
 *    field of the structure.
 */

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/mount.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs_struct.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("List mountpoints on system");
MODULE_AUTHOR("ctrouill");

#define PROCFS_NAME "mymounts"

static struct proc_dir_entry *entry;

static int done;

static ssize_t p_read(struct file *file, char __user *ubuff,
		      size_t count, loff_t *ppos) {
	struct dentry *curdentry;
	int len;

	len = 0;
	if (done)
		return 0;
	list_for_each_entry(curdentry, &current->fs->root.mnt->mnt_root->d_subdirs, d_child) {
		if (curdentry->d_flags & DCACHE_MOUNTED) {
			char format[300]; /* Find smarter way to alloc */

			sprintf(format, "%-10s /sys\n", curdentry->d_name.name);
			if (copy_to_user(ubuff, format, 300))
				return -EINVAL;
			len += strlen(format);
		}
	}
	done = 1;
	return len;
}

static int p_release(struct inode *ip, struct file *fp) {
	done = 0x0;
	return 0;
}

static struct proc_ops pops = { .proc_read = p_read,
	.proc_release = p_release };

static __init int initialize(void)
{
	entry = proc_create(PROCFS_NAME, 0660, NULL, &pops);
	if (entry == NULL) {
		printk(KERN_ERR "Error while creating procfs file !");
		return -EBADR;
	}
	printk(KERN_INFO "The 'mymounts' module is in place !");
	return 0;
}

static __exit void destroy(void)
{
	proc_remove(entry);
	printk(KERN_INFO "Goodbye, sad world.");
}

module_init(initialize);
module_exit(destroy);
