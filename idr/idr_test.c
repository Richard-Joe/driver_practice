#include <linux/slab.h>
#include <linux/module.h>
#include <linux/idr.h>

struct student {
    char name[64];
    int id;
};


#define CHECK_ERR(ret) \
    if (ret) \
    { \
        printk("check failed [%d]\n", ret); \
        goto err; \
    }

static DEFINE_IDR(student_idr);

static inline int student_add(struct student *stu)
{
    int ret;

    ret = idr_alloc(&student_idr, stu, 0, 1024, GFP_KERNEL);
    if (ret >= 0)
    {
        stu->id = ret;
        return 0;
    }
    return ret;
}

static inline struct student *student_del(int id)
{
    return idr_remove(&student_idr, id);
}

static inline struct student *student_find(int id)
{
    return idr_find(&student_idr, id);
}

static int print_student(int id, void *p, void *data)
{
    struct student *stu = (struct student *)p;
    printk("ID: %d, Name: %s\n", stu->id, stu->name);
    return 0;
}

static void student_show(void)
{
    idr_for_each(&student_idr, print_student, NULL);
}

static void idr_test(void)
{
    struct student *tmp;
    struct student stu0 = {.name = "000"};
    struct student stu1 = {.name = "111"};
    struct student stu2 = {.name = "222"};
    struct student stu3 = {.name = "333"};

    CHECK_ERR(student_add(&stu0));
    CHECK_ERR(student_add(&stu1));
    CHECK_ERR(student_add(&stu2));
    CHECK_ERR(student_add(&stu3));

    student_show();

    tmp = student_find(1);
    printk("%s\n", tmp->name);

    tmp = student_del(0);
    printk("%s\n", tmp->name);

    tmp = student_del(2);
    printk("%s\n", tmp->name);

    student_show();

err:
    idr_destroy(&student_idr);
}

static int init(void)
{
    printk("idr test init\n");
    idr_test();
    return 0;
}

static void fini(void)
{
    printk("idr test exit\n");
}

module_init(init);
module_exit(fini);

MODULE_LICENSE("GPL");