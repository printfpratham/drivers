#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>


MODULE_AUTHOR("PRATHAM");
MODULE_LICENSE("GPL");


static struct spi_device *adxl_spi;

struct spi_master *master;
//	.bus_num = 0,
//	.num_chipselect = 1,
//	.slave=true,

struct spi_board_info adxl_info = 
{
	.modalias = "adxl_spi_driver",
	.max_speed_hz = 5000000,
	.bus_num = 0,
	.chip_select = 0,
	.mode = SPI_MODE_3
};

static const struct spi_device_id adxl_spi_id[] = {
	{"adxl345",0},
	{  }
};

MODULE_DEVICE_TABLE(spi,adxl_spi_id);

static const struct of_device_id adxl345_dt[] = {
	{.compatible = "adi,my_adxl"},
	{  },
};

MODULE_DEVICE_TABLE(of,adxl345_dt);



static int adxl_probe(struct spi_device *spi)
{

 	//adxl_spi = spi_new_device(spi->master,&adxl_info);
	int ret;
	s16 data = 0x00;
	
	pr_info("Probed\n");
	u16 buf;
	ret = spi_write(adxl_spi,&data,sizeof(data));
	if(ret != 0){
		pr_err("Write error\n");
	}

	ret = spi_read(adxl_spi,&buf,sizeof(buf));
	if(ret == 0){
		pr_info("Device id: %d\n",buf);
	}
	else{
		pr_err("Read error\n");
	}



	return 0;
}


static void adxl_remove(struct spi_device *spi)
{
	pr_info("Device removed\n");

}


static struct spi_driver adxl_spi_driver = {
	.driver = {
		.name = "adxl_345",

	},
	.probe = adxl_probe,
	.remove = adxl_remove,
	.id_table = adxl_spi_id,
};


static int __init adxl_spi_init(void)
{
	int ret;

	master = spi_busnum_to_master(0);
	if(master == NULL){
		pr_err("Bus num not found");
	}
//	adxl_spi = spi_alloc_device(&master);
//	if(adxl_spi == NULL){
//		pr_err("Alloc error\n");
//	}


	adxl_spi = spi_new_device(master,&adxl_info);
	if(adxl_spi == NULL){
		pr_err("New device error\n");
	}

//	ret = spi_add_device(adxl_spi);
//	if(ret != 0){
//		pr_err("Device error\n");
//	}
	
	
	ret = spi_register_driver(&adxl_spi_driver);	
	if(ret != 0){
		pr_err("Register error\n");
	}	
	pr_info("Driver Registered\n");
	

	

	
 //	adxl_spi = spi_new_device(master,&adxl_info);
//	if(adxl_spi == NULL){
//		pr_err("Device not found!!!\n");
//	}

//	adxl_spi->bits_per_word = 8;
//	ret = spi_setup(adxl_spi);
//	if(ret != 0){
//		pr_err("Setup error!!\n");
//	}
	return 0;
}

static void __exit adxl_spi_exit(void)
{
	spi_unregister_driver(&adxl_spi_driver);
	spi_unregister_device(adxl_spi);

}



module_init(adxl_spi_init);
module_exit(adxl_spi_exit);




