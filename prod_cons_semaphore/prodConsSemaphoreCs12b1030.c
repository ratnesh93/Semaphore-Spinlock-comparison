#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h> //for kmalloc
#include <linux/time.h>
#include <linux/delay.h>

int capacity=100;
int np=10;
int nc=15;
int cntp=15;
int cntc=10;
int t1=8;
int t2=10;

struct timespec bPtime,bCtime,APtime,ACtime,PAvgtime,CAvgtime;
int buffer=0;

struct semaphore mutex;
struct semaphore fillCount;
struct semaphore emptyCount;

struct task_struct **producer;
struct task_struct **consumer;

int producer_thread_fn(void *n){
    int *threadId;
    int j;
    threadId=(int*)n;

    for(j=0;j<cntp;j++){
        down_interruptible(&emptyCount);
        down_interruptible(&mutex);
        
        getnstimeofday (&bPtime);
        //put item into buffer
        buffer++;
        printk("\nItem number %d produced by thread %d at time into buffer location %d",j,*threadId,buffer);
   
        getnstimeofday  (&APtime);

        PAvgtime.tv_sec   += APtime.tv_sec - bPtime.tv_sec; 
        PAvgtime.tv_nsec  += APtime.tv_nsec - bPtime.tv_nsec;

        up(&mutex);
        up(&fillCount);
        msleep(t1);

    }
    set_current_state(TASK_INTERRUPTIBLE);
    while (!kthread_should_stop()) {
        schedule();
        set_current_state(TASK_INTERRUPTIBLE);
    }
    __set_current_state(TASK_RUNNING);

    printk("\nthe average time by taken by producer:  %ld nanoseconds",((PAvgtime.tv_sec)*1000000000 + (PAvgtime.tv_nsec))/np*cntp);
   
    return 0;
}

int consumer_thread_fn(void *n){
    int *threadId;
    int k;
    threadId=(int*)n;
    
    for(k=0;k<cntc;k++){
        down_interruptible(&fillCount);
        down_interruptible(&mutex);
        
        getnstimeofday(&bCtime);
        //remove item into buffer
        
        printk("\nItem number %d consumed by thread %d at time into buffer location %d",k,*threadId,buffer);
        buffer--;

        getnstimeofday (&ACtime);
        CAvgtime.tv_sec   += ACtime.tv_sec - bCtime.tv_sec; 
        CAvgtime.tv_nsec  += ACtime.tv_nsec - bCtime.tv_nsec;

        up(&mutex);
        up(&emptyCount);
        msleep(t2);
    }   
    set_current_state(TASK_INTERRUPTIBLE);
    while (!kthread_should_stop()) {
        schedule();
        set_current_state(TASK_INTERRUPTIBLE);
    }
    __set_current_state(TASK_RUNNING);
   
    printk("\nthe average time by taken by consumer:  %ld nanoseconds",((CAvgtime.tv_sec)*1000000000 + (CAvgtime.tv_nsec))/nc*cntc);
    
    return 0;
}

static int kernel_init(void)
{
    int i;
    sema_init(&mutex, 1);
    sema_init(&fillCount, 0);
    sema_init(&emptyCount, capacity);

    PAvgtime=(struct timespec){ 0 };
    CAvgtime=(struct timespec){ 0 };

    producer=kmalloc(sizeof(struct task_struct)*np,GFP_KERNEL);
    if(!producer)
        printk(KERN_DEBUG"\nCannot assign memory for producer");

    consumer=kmalloc(sizeof(struct task_struct)*nc,GFP_KERNEL);
    if(!consumer)
        printk(KERN_DEBUG"\nCannot assign memory for consumer");

    printk(KERN_INFO"\n--------------------------------------------");
    
    for(i=0;i<np;i++){
        int *t;
        t=&i;
        producer[i] = kthread_run(&producer_thread_fn,(void*)t,"producer");
    }
    
    for(i=0;i<nc;i++){
        int *t;
        t=&i;
        consumer[i] = kthread_run(&consumer_thread_fn,(void*)t,"consumer");
    }

    return 0;
}

static void kernel_exit(void)
{
    int i;
    for(i=0;i<np;i++){
        kthread_stop(producer[i]);
    }
    for(i=0;i<nc;i++){
        kthread_stop(consumer[i]);
    }
    kfree(producer);
    kfree(consumer);
}

module_init(kernel_init);
module_exit(kernel_exit);
MODULE_AUTHOR("RatnechChandak");
MODULE_LICENSE("GPL");