#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

static int prod = 0;
static int cons = 0;
static int size = 0;

module_param(prod, int, 0);
module_param(cons, int, 0);
module_param(size, int, 0);

static struct semaphore empty;
static struct semaphore full;

static struct task_struct **producer_threads;
static struct task_struct **consumer_threads;

int producer_function(void *data) {
    int id = *(int *)data;
    char thread_name[16];
    snprintf(thread_name, sizeof(thread_name), "Producer-%d", id);

    while (!kthread_should_stop()) {
        down_interruptible(&empty);
        // Simulate producing an item
        printk(KERN_INFO "An item has been produced by %s\n", thread_name);
        up(&full);
        msleep(1000); // Sleep to simulate work
    }
    return 0;
}

int consumer_function(void *data) {
    int id = *(int *)data;
    char thread_name[16];
    snprintf(thread_name, sizeof(thread_name), "Consumer-%d", id);

    while (!kthread_should_stop()) {
        down_interruptible(&full);
        // Simulate consuming an item
        printk(KERN_INFO "An item has been consumed by %s\n", thread_name);
        up(&empty);
        msleep(1000); // Sleep to simulate work
    }
    return 0;
}

static int __init producer_consumer_init(void) {
    int i;

    sema_init(&empty, size);
    sema_init(&full, 0);

    producer_threads = kmalloc_array(prod, sizeof(struct task_struct *), GFP_KERNEL);
    consumer_threads = kmalloc_array(cons, sizeof(struct task_struct *), GFP_KERNEL);

    for (i = 0; i < prod; i++) {
        int *id = kmalloc(sizeof(int), GFP_KERNEL);
        *id = i + 1;
        producer_threads[i] = kthread_run(producer_function, id, "Producer-%d", i + 1);
    }

    for (i = 0; i < cons; i++) {
        int *id = kmalloc(sizeof(int), GFP_KERNEL);
        *id = i + 1;
        consumer_threads[i] = kthread_run(consumer_function, id, "Consumer-%d", i + 1);
    }

    return 0;
}

static void __exit producer_consumer_exit(void) {
    int i;

    for (i = 0; i < prod; i++) {
        if (producer_threads[i]) {
            kthread_stop(producer_threads[i]);
            kfree(producer_threads[i]);
        }
    }

    for (i = 0; i < cons; i++) {
        if (consumer_threads[i]) {
            kthread_stop(consumer_threads[i]);
            kfree(consumer_threads[i]);
        }
    }

    kfree(producer_threads);
    kfree(consumer_threads);

    printk(KERN_INFO "Producer-consumer module unloaded.\n");
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vamsi Krishna Vasa");
MODULE_DESCRIPTION("Project-3 Producer Consumer Code");

