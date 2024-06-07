#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/kthread.h> 

MODULE_AUTHOR("PRATHAM");
MODULE_LICENSE("GPL");

static struct task_struct *data_thread;

u8 receive_data[6];
u8 read_reg = 0xF2;
s16 X,Y,Z;

static int thread_handle(void *a)
{
	int ret;
	while(!kthread_should_stop()){
	ret = spi_write_then_read((struct spi_device *)a,&read_reg,1,receive_data,6);
        	if(ret){
                pr_err("Read failed\n");
        	}
         
        	X = (receive_data[1] << 8)| receive_data[0];
         	Y = (receive_data[3] << 8)| receive_data[2];
         	Z = (receive_data[5] << 8)| receive_data[4];

	        pr_info("X: %d\n Y:%d\n Z:%d\n",X,Y,Z);
		msleep(1000);
	}
	
	return 0;
			
}
static int adxl345_probe(struct spi_device *spi)
{
	int ret;

    printk(KERN_INFO "ADXL345 probe function called\n");
	u8  rx_data,tx_data=0x80;
	u8 config_data[6] = {0x31,0x0B,0x2D,0x08,0x2E,0x80};

	spi->mode = SPI_MODE_3;
 	spi->max_speed_hz = 5000000;
    	spi->bits_per_word = 8;
    	ret = spi_setup(spi);

	spi_write(spi,&tx_data,1);
	spi_read(spi,&rx_data,1);
	pr_info("ADXL id=%d\n",rx_data);
	
	ret = spi_write(spi,config_data,6);
	if(ret){
		pr_err("Config failed\n");
	}

//	spi = adxl_spi;

	 data_thread = kthread_run(thread_handle,spi,"adxl_thread");
        if(data_thread){
                pr_info("Thread created");
        }


	
    // Additional initialization code can be added here
	
    return 0;
}

static void  adxl345_remove(struct spi_device *spi)
{
    printk(KERN_INFO "ADXL345 remove function called\n");

    // Additional cleanup code can be added here

    
}

static const struct spi_device_id adxl345_id[] = {
    { "adxl345", 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, adxl345_id);

static const struct of_device_id adxl345_of_match[] = {
    { .compatible = "adxl,adxl345" },
    { },
};
MODULE_DEVICE_TABLE(of, adxl345_of_match);

static struct spi_driver adxl345_driver = {
    .driver = {
        .name = "adxl345",
        .owner = THIS_MODULE,
        .of_match_table = adxl345_of_match,
    },
    .probe = adxl345_probe,
    .remove = adxl345_remove,
    .id_table = adxl345_id
};

module_spi_driver(adxl345_driver);


