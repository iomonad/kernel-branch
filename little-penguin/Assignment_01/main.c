#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Assignement 01");
MODULE_AUTHOR("IOMONAD");

static __init int initialize(void)
{
    printk(KERN_INFO "Hello world !");
    return 0;
}

static __exit void destroy(void)
{
    printk(KERN_INFO "Cleaning up module");
}

module_init(initialize);
module_exit(destroy);
