#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include "uart16550.h"
#include "uart16550_hw.h"
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("Uart16550 driver");
MODULE_LICENSE("GPL");

#ifdef __DEBUG
 #define dprintk(fmt, ...)     printk(KERN_DEBUG "%s:%d " fmt, \
                                __FILE__, __LINE__, ##__VA_ARGS__)
#else
 #define dprintk(fmt, ...)     do { } while (0)
#endif
#define mem_buffer_size 4096

#define COM1 "com1"
#define COM2 "com2"

static struct class *uart16550_class = NULL;
/*
 * TODO: Populate major number from module options (when it is given).
 */

struct device_data 
{
    DECLARE_KFIFO(data_from_user,uint8_t,mem_buffer_size);
    DECLARE_KFIFO(data_from_device,uint8_t,mem_buffer_size);
    struct task_struct **task_user_get_data;
    struct task_struct **task_user_push_data;
};


static size_t uart16550_write(struct file *, const char *, size_t, loff_t *);
static size_t uart16550_read(struct file *, const char *, size_t ,loff_t *);
static int uart16550_open(struct inode *, struct file *);
static int uart16550_release (struct inode *, struct file *);
static struct task_struct **ftask_user_get_data(void *);
static struct task_struct **ftask_user_push_data(void *);
static struct kfifo* fdata_from_user (void*);
static struct kfifo* fdata_from_device (void *);
static bool can_read(struct kfifo *);
static bool can_write(struct kfifo *);
static int wait_for_queue(struct task_struct**, bool(*can_proceed)(struct kfifo*), struct kfifo * );

static struct cdev* firstcdev = NULL;
static struct cdev* secondcdev = NULL;

static struct task_struct **ftask_user_get_data(void *data){
    return ((struct device_data*)data)->task_user_get_data;
}
static struct task_struct **ftask_user_push_data(void * data){
    return ((struct device_data*)data)->task_user_push_data;
}


static struct kfifo* fdata_from_user (void * data){
    
    return &(((struct device_data*)data)->data_from_user);
}
static struct kfifo* fdata_from_device (void *data){
    return &(((struct device_data*)data)->data_from_device);
}

static int major = 42;
static int behavior=0x3;

module_param(major, int, S_IRUGO);
module_param(behavior, int, S_IRUGO);

static uint8_t charToU8(char c)
{
    return c;
}

static char u8ToChar(uint8_t i)
{
    return i;
}
static struct file_operations fops = {
  .open = uart16550_open,
  .read = uart16550_read,
  .write = uart16550_write,
  .release = uart16550_release
  /*.ioctl = uart16550_ioctl*/
};
static bool can_read(struct kfifo *queue){ return !kfifo_is_empty(queue);}
static bool can_write(struct kfifo *queue){ return !kfifo_is_full(queue);}



static size_t uart16550_write(struct file *filp, const char *user_buffer, size_t size, loff_t *offset)
{
        struct task_struct **task_user_push_data = ftask_user_push_data(filp->private_data);
        struct kfifo* data_from_user = fdata_from_user(filp->private_data);
        int i;
        for(i = 0; i<size; i++){
            wait_for_queue(task_user_push_data,can_write,data_from_user);
            kfifo_put(data_from_user,charToU8(user_buffer[i]));
        }
        
        return size;
}

static size_t uart16550_read(struct file *filp, const char *buffer, size_t size, loff_t *offset) 
{
    struct task_struct **task_user_get_data = ftask_user_get_data(filp->private_data);
    struct kfifo *data_from_device = fdata_from_device (filp->private_data);
    uint8_t u8;
    int i;
    char* pos = buffer;
    for(i =0;i<size;i++)
    {
        wait_for_queue(task_user_get_data,can_read,data_from_device);
        kfifo_get(data_from_device,&u8);
        put_user(u8ToChar(u8),pos+i);
    }
   
   return size;
}
static int wait_for_queue(struct task_struct** task_storage, bool(*can_proceed)(struct kfifo*), struct kfifo * queue) {
    
    *task_storage = current;
    
    set_current_state(TASK_INTERRUPTIBLE);
    while (!can_proceed(queue)) {
          schedule();
          set_current_state(TASK_INTERRUPTIBLE);
    }
    __set_current_state(TASK_RUNNING);
    *task_storage = NULL;
    return 0;
}
irqreturn_t interrupt_handler(int irq_no, void *data)
{
        int device_status;
        uint32_t device_port = 0x0;
        /*
         * TODO: Write the code that handles a hardware interrupt.
         * TODO: Populate device_port with the port of the correct device.
         */

        int ret = IRQ_HANDLED;
        if(irq_no == COM1_IRQ) {
            device_port = COM1_BASEPORT;
        }
        else if(irq_no == COM2_IRQ) {
            device_port = COM2_BASEPORT;
        }
        if(device_port)
        {

            disable_irq(irq_no);
            device_status = uart16550_hw_get_device_status(device_port);

            struct task_struct **task_user_get_data = ftask_user_get_data(data);
            struct task_struct **task_user_push_data = ftask_user_push_data(data);
            struct kfifo * data_from_user = fdata_from_user (data);
            struct kfifo * data_from_device = fdata_from_device (data);

            while (uart16550_hw_device_can_send(device_status) && !kfifo_is_empty(data_from_user)) {
                    uint8_t byte_value;
                    /*
                     * TODO: Populate byte_value with the next value
                     *      from the kernel device outgoing buffer.
                     */
                    
                    kfifo_get(data_from_user,&byte_value);
                    if(*task_user_push_data)
                        wake_up_process(*task_user_push_data);
                    

                    uart16550_hw_write_to_device(device_port, byte_value);
                    device_status = uart16550_hw_get_device_status(device_port);
            }

            while (uart16550_hw_device_has_data(device_status) && !kfifo_is_full(data_from_device)) {
                    uint8_t byte_value;
                    byte_value = uart16550_hw_read_from_device(device_port);
                    /*
                     * TODO: Store the read byte_value in the kernel device
                     *      incoming buffer.
                     */
                     kfifo_put(data_from_device,byte_value);
                     if(*task_user_get_data)
                        wake_up_process(*task_user_get_data);
                     
                    device_status = uart16550_hw_get_device_status(device_port);
            }
            enable_irq(irq_no);
        }
        else
        {
            ret = -1;
        }
        
        return ret;
}

static struct cdev *char_device_register(dev_t num,struct file_operations *fops)
{
    struct cdev *my_cdev = cdev_alloc();
    my_cdev -> ops = &fops;
    my_cdev -> owner = THIS_MODULE;
    cdev_init(my_cdev, fops);
   int ret=cdev_add(my_cdev,num, 1);
   if(ret < 0 )
   {
     printk(KERN_INFO "Unable to allocate cdev");
    
    }
     return &my_cdev;
}

static int uart16550_init(void)
{
        int have_com1 = 0;
        int have_com2 = 0;
        /*registe device*/
        dev_t first = MKDEV(major,0);
        dev_t second = MKDEV(major,1);
        firstcdev=char_device_register(first, &fops);
        secondcdev=char_device_register(second, &fops);

         if(behavior==0x3){
                have_com1=1; have_com2=1;
         }
         else if(behavior==0x2){
                have_com1=0; have_com2=1;
         }
         else if(behavior==0x1){
                have_com1=1; have_com2=0;
         }

        /*
         * Setup a sysfs class & device to make /dev/com1 & /dev/com2 appear.
         */
        uart16550_class = class_create(THIS_MODULE, "uart16550");

        if (have_com1) {
                /* Setup the hardware device for COM1 */
                uart16550_hw_setup_device(COM1_BASEPORT, THIS_MODULE->name);
                /* Create the sysfs info for /dev/com1 */
                device_create(uart16550_class, NULL, first, NULL, "com1");
        }
        if (have_com2) {
                /* Setup the hardware device for COM2 */
                uart16550_hw_setup_device(COM2_BASEPORT, THIS_MODULE->name);
                /* Create the sysfs info for /dev/com2 */
                device_create(uart16550_class, NULL, second, NULL, "com2");
        }
       
        return 0;
}

static void uart16550_cleanup(void)
{
        int have_com1 =0;
        int have_com2 = 0;
        cdev_del(firstcdev);
        cdev_del(secondcdev);         

          if(behavior==0x3){
                have_com1=1; have_com2=1;
         }
         else if(behavior==0x2){
                have_com1=0; have_com2=1;
         }
         else if(behavior==0x1){
                have_com1=1; have_com2=0;
         }

        if (have_com1) {
                /* Reset the hardware device for COM1 */
                uart16550_hw_cleanup_device(COM1_BASEPORT);
                /* Remove the sysfs info for /dev/com1 */
                device_destroy(uart16550_class, MKDEV(major, 0));
        }
        if (have_com2) {
                /* Reset the hardware device for COM2 */
                uart16550_hw_cleanup_device(COM2_BASEPORT);
                /* Remove the sysfs info for /dev/com2 */
                device_destroy(uart16550_class, MKDEV(major, 1));
        }
        
        class_unregister(uart16550_class);
        class_destroy(uart16550_class);
}
static int uart16550_open(struct inode *inode, struct file *file){
    
    struct device_data *devicedata;
       
    devicedata=kmalloc(sizeof(struct device_data), GFP_KERNEL);
    if(!devicedata){
        return -1;
    }
    file -> private_data=devicedata; 
    INIT_KFIFO(devicedata->data_from_user);   
    INIT_KFIFO(devicedata->data_from_device);   
    devicedata -> task_user_push_data = kmalloc(sizeof(struct task_struct*),GFP_KERNEL);
    devicedata -> task_user_get_data = kmalloc(sizeof(struct task_struct*),GFP_KERNEL);
    *(devicedata -> task_user_push_data) = NULL; 
    *(devicedata -> task_user_get_data) = NULL;

    return 0;
}
static int uart16550_release (struct inode * node, struct file *file){
    struct  device_data *devicedata =  file->private_data;
    kfifo_free(&(devicedata->data_from_user));
    kfifo_free(&(devicedata->data_from_device));
    kfree(devicedata->task_user_push_data);
    kfree(devicedata->task_user_get_data);
    kfree(file->private_data);
    return 0;
}
module_init(uart16550_init)
module_exit(uart16550_cleanup)
//.PHONY:kbuild instal clean
