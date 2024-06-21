#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/i2c-dev.h>
#define I2C_BUS_AVAILABLE   1           
#define SLAVE_DEVICE_NAME   "mpu6050"            
#define MPU6050_SLAVE_ADDR  0x68

static struct i2c_adapter *etx_i2c_adapter=NULL;
static struct i2c_client *etx_i2c_client_mpu=NULL;
static struct task_struct *mpu_thread;

static int I2C_Write(unsigned char *buf, unsigned int len)
{
    int ret = i2c_master_send(etx_i2c_client_mpu, buf, len);

    return ret;
}

static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    int ret = i2c_master_recv(etx_i2c_client_mpu, out_buf, len);

    return ret;
}

static void MPU6050_DisplayInit(void)
{
    unsigned char buf1[2];
    unsigned char buf2[2];
    unsigned char buf3[2];
    unsigned char buf4[2];
    unsigned char buf5[2];

    buf1[0] = 0x19;
    buf1[1] = 0x07;
    I2C_Write(buf1, 2);

    buf2[0] = 0x6B;
    buf2[1] = 0x01;
    I2C_Write(buf2, 2);

    buf3[0] = 0x1A;
    buf3[1] = 0x00;
    I2C_Write(buf3, 2);

    buf4[0] = 0x1B;
    buf4[1] = 0x18;
    I2C_Write(buf4, 2);

    buf5[0] = 0x38;
    buf5[1] = 0x01;
    I2C_Write(buf5, 2);
}

static int MPU6050_read_xyz(void)
{
    int ret;
    unsigned char buf[1];
    buf[0] = 0x75;
    I2C_Write(buf, 1);

    ret = i2c_smbus_read_byte_data(etx_i2c_client_mpu, 0x75); // WHO_AM_I register address
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to read MPU6050 ID\n");
        return ret;
    }

    unsigned char id_reg = ret;
    printk(KERN_INFO "MPU6050 ID: 0x%02X\n", id_reg);

    unsigned char buf_gyro[14];
    int i;
    unsigned char reg_addr = 0x3B; // Starting register address

    // Write the starting register address to the device
    buf_gyro[0] = reg_addr;
    I2C_Write(buf_gyro, 1);

    // Read registers 0x3B to 0x48 sequentially
    for (i = 0; i < 14; i++)
    {
        ret = i2c_smbus_read_byte_data(etx_i2c_client_mpu, reg_addr + i);
        if (ret < 0)
        {
            printk(KERN_ERR "Failed to read register 0x%02X\n", reg_addr + i);
            return ret;
        }
        buf_gyro[i] = ret;
    }

    // Print the read values
    for (i = 0; i < 14; i++)
    {
        printk(KERN_INFO "Register 0x%02X: 0x%02X\n", reg_addr + i, buf_gyro[i]);
    }

    return 0;
}

static int MPU6050_thread_fn(void *data)
{
    while (!kthread_should_stop())
    {
        MPU6050_read_xyz();
        msleep(100); // Adjust as needed
    }
    return 0;
}

static int MPU6050_probe(struct i2c_client *client)
{
    MPU6050_DisplayInit();

    etx_i2c_client_mpu = client;
    mpu_thread = kthread_run(MPU6050_thread_fn, NULL, "mpu6050_thread");

    pr_info("MPU6050 Probed!!!\n");

    return 0;
}

static void MPU6050_remove(struct i2c_client *client)
{
    if (mpu_thread)
        kthread_stop(mpu_thread);
    pr_info("MPU6050 Removed!!!\n");
}

static const struct i2c_device_id MPU6050_id[] = {
    { SLAVE_DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, MPU6050_id);

static const struct of_device_id mpu_i2c_id[] = {
	{ .compatible = "adi,my_adxl" },
	{  }
};
MODULE_DEVICE_TABLE(of,mpu_i2c_id);

static struct i2c_driver mpu6050_driver = {
    .driver = {
        .name   = SLAVE_DEVICE_NAME,
        .owner  = THIS_MODULE,
	.of_match_table = mpu_i2c_id,
    },
    .probe          = MPU6050_probe,
    .remove         = MPU6050_remove,
    .id_table       = MPU6050_id,
};

static struct i2c_board_info mpu6050_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MPU6050_SLAVE_ADDR)
};

static int __init etx_driver_init(void)
{
    int ret = -1;
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if (etx_i2c_adapter != NULL)
    {
    
	etx_i2c_client_mpu = i2c_new_client_device(etx_i2c_adapter, &mpu6050_i2c_board_info);
	if (etx_i2c_client_mpu != NULL)
        {
            i2c_add_driver(&mpu6050_driver);
            ret = 0;
        }

        i2c_put_adapter(etx_i2c_adapter);
    }

    pr_info("Driver Added!!!\n");
    return ret;
}

static void __exit etx_driver_exit(void)
{
    i2c_unregister_device(etx_i2c_client_mpu);
    i2c_del_driver(&mpu6050_driver);
    pr_info("Driver Removed!!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tarang");
MODULE_DESCRIPTION("Simple I2C driver mpu6050");
MODULE_VERSION("1.34");

