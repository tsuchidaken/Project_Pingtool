#include <linux/kernel.h>
// #include <linux/linkage.h>
#include <linux/uaccess.h>

// copy_to_user(void *dest, void *src, int size)
// copy_from_user(void *dest, const void __user *src, int size)
// Add system calls

asmlinkage void sys_average(int *data, int *result) {
    int res = 0;
    int i = 0;
    int buf[5];
    // int checklen = sizeof(buf);
    // if (size < checklen) checklen = size;
    copy_from_user(buf, data, sizeof(buf));
    while (i < 5) {
        res = buf[i] + res;
        i = i + 1;
        printk("%d", res);
    }
    res = res / 5;
    copy_to_user(result, &res, sizeof(int));
    printk("ping_average_delay_time is %d", res);
}
