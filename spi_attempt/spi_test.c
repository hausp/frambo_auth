#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
 
#define MY_BUS_NUM 0
static struct spi_device *spi_device;
 
static int __init spi_init(void)
{
    int ret;
    unsigned char ch = 0x00;
    struct spi_master *master;
     
    //Register information about your slave device:
    struct spi_board_info spi_device_info = {
        .modalias = "rc522",
        .max_speed_hz = 10000000, //speed your device (slave) can handle
        .bus_num = MY_BUS_NUM,
        .chip_select = 0,
        .mode = 3,
    };
     
    /*To send data we have to know what spi port/pins should be used. This information 
      can be found in the device-tree. */
    master = spi_busnum_to_master( spi_device_info.bus_num );
    if( !master ){
        printk("MASTER not found.\n");
            return -ENODEV;
    }
     
    // create a new slave device, given the master and device info
    spi_device = spi_new_device( master, &spi_device_info );
 
    if( !spi_device ) {
        printk("FAILED to create slave.\n");
        return -ENODEV;
    }
     
    spi_device->bits_per_word = 8;
 
    ret = spi_setup( spi_device );
     
    if( ret ){
        printk("FAILED to setup slave.\n");
        spi_unregister_device( spi_device );
        return -ENODEV;
    }
 
    spi_write(spi_device, &ch, sizeof(ch));
     
    return 0;
}
 
 
static void __exit spi_exit(void)
{
    unsigned char ch = 0Xff;
 
    if( spi_device ){
        spi_write(spi_device, &ch, sizeof(ch));
        spi_unregister_device( spi_device );
    }
}
 
module_init(spi_init);
module_exit(spi_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Piktas Zuikis <[email protected]>");
MODULE_DESCRIPTION("SPI module example");
