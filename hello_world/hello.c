#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>

MODULE_LICENSE("Dual BSD/GPL");

static void cur_process(void) {
    printk(KERN_ALERT "The process is \"%s\" (pid %i)\n",
        current->comm, current->pid);
}

static int hello_init(void) {
    printk(KERN_ALERT "Hello, world\n");
    cur_process();
    return 0;
}

static void hello_exit(void) {
    printk(KERN_ALERT "Googbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
