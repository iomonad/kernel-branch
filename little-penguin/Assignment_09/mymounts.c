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
 *  - Since >4.9 kernel version, file_operations removed
 *    in favor of proc_operation ! THIS SHOULD NOT COMPILE
 *    ON BLEEDING EDGE SYSTEMS.
 *
 * Lecture:
 *  - https://kernelnewbies.org/Documents/SeqFileHowTo
 */

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <linux/seq_file.h>
#include <linux/fs_struct.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("List mountpoints on system");
MODULE_AUTHOR("ctrouill");

#define PROCFS_NAME "mymounts"

static struct proc_dir_entry *entry;
static char *shared_buffer;

static int mount_lambda(struct vfsmount *root, void *data) {
	struct super_block *sb;
	struct seq_file *seqptr;
	struct path fpath;
	char *dentry_path;

	seqptr = (struct seq_file*)data;
	if (!seqptr) {
		printk(KERN_ERR "SEQUENCE IS NULL");
		return 0;
	}
	/* Assign superblock */
	sb = root->mnt_sb;

	/* Build path */
	fpath.mnt = root;
	fpath.dentry = root->mnt_root;

	/* Return the path of a dentry */
	dentry_path = d_path(&fpath, shared_buffer, PAGE_SIZE);
	seq_printf(seqptr, "%-15s %s\n", sb->s_id, dentry_path);
	return 0;
}

static int seq_implementation(struct seq_file *s, void *v)
{
	struct path path;
	struct vfsmount *root;
	struct vfsmount *(*collect_mountpoints)(const struct path*) =
		(void *)kallsyms_lookup_name("collect_mounts");
	int (*iterate_mountpoints)(int(*)(struct vfsmount*, void*), void*,
	        struct vfsmount*) = (void *)kallsyms_lookup_name("iterate_mounts");
	/*
	 * Since we don't use Module.symvers, we should manually
	 * assign symbols for indy use, otherwise, kernel will drop
	 * us in dmesg:
	 *
	 * [  909.960914] mymounts: Unknown symbol collect_mounts (err -2)
	 * [  909.960947] mymounts: Unknown symbol iterate_mounts (err -2)
	 *
	 */
	kern_path("/", 0, &path);
	root = collect_mountpoints(&path);
        iterate_mountpoints(mount_lambda, (void*)s, root);
	return 0;
}

static int p_open(struct inode *i, struct file *f)
{
	/*
	 * Build our char sequence that will automaticaly
	 * generate the boilerplate for for the `seq_read`
	 * function.
	 *
	 * It provides a safer interface to the /proc filesystem
	 * than previous procfs methods because it protects against overflow
	 * of the output buffer and easily handles procfs files that are larger
	 * than one page. It also provides methods for traversing a list of
	 * kernel items and iterating on that list. It provides procfs output
	 * facilities that are less error-prone than the
	 * previous procfs interfaces.
	 */
	return single_open(f, &seq_implementation, NULL);
}

static struct file_operations pops = { .open = p_open,
	                               .read = seq_read };

static __init int initialize(void)
{
	shared_buffer = kmalloc(sizeof(char) * PAGE_SIZE, GFP_KERNEL);
	if (!shared_buffer) {
		printk(KERN_ERR "Error while allocating memory!");
		return 1;
	}
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
	kfree(shared_buffer);
	proc_remove(entry);
	printk(KERN_INFO "Goodbye, sad world.");
}

module_init(initialize);
module_exit(destroy);
