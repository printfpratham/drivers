#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define BUF_SIZE 1024


static int major_number;
uint8_t *kernel_buffer;

static int device_open(struct inode *inode, struct file *file)
{

    	kernel_buffer = kmalloc(BUF_SIZE,GFP_KERNEL);
	printk(KERN_INFO "Device opened\n");
    	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    	kfree(kernel_buffer);
	printk(KERN_INFO "Device closed\n");
	return 0;
}

static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    copy_to_user(buffer,kernel_buffer,BUF_SIZE);
    pr_info("Data Read done\n");
    return bytes_read;
}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
    
    copy_from_user(kernel_buffer, buffer, length);
    pr_info("Data written\n");
    return length;
 
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init chardev_init(void)
{
	
//	alloc_chrdev_region(&dev, 0, 1, "mychardev");
	

    major_number = register_chrdev(0, "mychardev", &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "Registered correctly with major number %d\n", major_number);
    return 0;
}

static void __exit chardev_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
        pr_info("Device Driver Remove...Done!!!\n");
    printk(KERN_INFO "Unregistered the device\n");
}

module_init(chardev_init);
module_exit(chardev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham");
MODULE_DESCRIPTION("CHARACTER DRIVER");
