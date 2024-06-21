#include  <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>
#include <linux/sysfs.h>



#define ADDR 0x53

static struct i2c_adapter *adxl_i2c_adapter = NULL;
static struct i2c_client *adxl_i2c_client;
static struct task_struct *data_thread;
static 


static const struct i2c_device_id  adxl_id[] ={
	{"adxl_345" , 0},
	{  }
};


static int adxl_probe(struct i2c_client *client)
{
	s32 ret;

	pr_info("ADXL Detected\n");
	ret = i2c_smbus_read_word_data(client, 0x00);
	pr_info("Address = %d\n",ret);

	ret = i2c_smbus_write_byte_data(client, 0x31,0x0B);
	ret = i2c_smbus_write_byte_data(client, 0x2D,0x08);
	ret = i2c_smbus_write_byte_data(client, 0x2E,0x80);

//	i2c_smbus_read_i2c_block_data(client,0x32,6,data);
	
	//s16 X = (data[1] << 8)| data[0];
	//s16 Y = (data[3] << 8)| data[2];
	//s16 Z = (data[5] << 8)| data[4];
	
	//pr_info("Data X: %d\n",X);
	//pr_info("Data Y: %d\n",Y);
	//pr_info("Data Z: %d\n",Z);


	return 0;

}

MODULE_DEVICE_TABLE(i2c,adxl_id);

static int thread_handle(void *a)
{
	u8 data[6];
	while(!kthread_should_stop()){
	i2c_smbus_read_i2c_block_data(adxl_i2c_client,0x32,6,data);

    	s16 X = (data[1] << 8)| data[0];
        s16 Y = (data[3] << 8)| data[2];
        s16 Z = (data[5] << 8)| data[4];

        pr_info("Data X: %d\n",X);
        pr_info("Data Y: %d\n",Y);
        pr_info("Data Z: %d\n",Z);

	msleep(1000);
	}
	return 0;
}

static void adxl_remove (struct i2c_client *client)
{

	pr_info("Adxl removed\n");

}

static struct i2c_driver adxl_driver = {
	.driver = {
		.name = "adxl_345",
	
	},
	.probe = adxl_probe,
	.remove = adxl_remove,
	.id_table = adxl_id,
};

static struct i2c_board_info adxl_board_info = {
	I2C_BOARD_INFO("adxl_345", ADDR)
};



static int __init adxl345_init(void)
{
	int ret;
	adxl_i2c_adapter = i2c_get_adapter(1);

	if(adxl_i2c_adapter != NULL){
	adxl_i2c_client = i2c_new_client_device(adxl_i2c_adapter,&adxl_board_info);
	}

	if(adxl_i2c_client != NULL){
	i2c_add_driver(&adxl_driver);
	ret = 0;
	}

	i2c_put_adapter(adxl_i2c_adapter);

	
	data_thread = kthread_run(thread_handle,NULL,"adxl_thread");
	if(data_thread){
		pr_info("Thread created");
	}

	pr_info("Driver added\n");
	return ret;
}

static void __exit adxl345_clean(void)
{
	kthread_stop(data_thread);
	i2c_unregister_device(adxl_i2c_client);
	i2c_del_driver(&adxl_driver);
	pr_info("Driver removed\n");


}
module_init(adxl345_init);
module_exit(adxl345_clean);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham");
MODULE_DESCRIPTION("adxl 345 i2c driver");

