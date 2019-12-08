#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kfifo.h>

static void kfifo_test(void)
{
    int i, val;
    struct kfifo fifo;

    if (kfifo_alloc(&fifo, PAGE_SIZE, GFP_KERNEL))
    {
        printk("kfifo_alloc failed\n");
        return;
    }

    for (i = 0; i < 20; i++)
    {
        kfifo_in(&fifo, &i, sizeof(i));
    }

    if (sizeof(val) != kfifo_out_peek(&fifo, &val, sizeof(val)))
    {
        printk("kfifo_out_peek failed\n");
        goto err;
    }

    printk("%d\n", val);

    while(kfifo_avail(&fifo))
    {
        if (sizeof(val) != kfifo_out(&fifo, &val, sizeof(val)))
        {
            printk("kfifo_out failed\n");
            goto err;
        }
        printk("%d", val);
    }
    printk("\n");

err:
    kfifo_free(&fifo);
}

static int init(void)
{
    printk("kfifo test init\n");
    kfifo_test();
    return 0;
}

static void fini(void)
{
    printk("kfifo test exit\n");
}

module_init(init);
module_exit(fini);

MODULE_LICENSE("GPL");