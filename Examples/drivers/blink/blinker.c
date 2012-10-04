/*
 *  blinker     device driver
 *
 *  Author:     Carlos Camargo
 *  Created:    Abril 29 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>  /* We want an interrupt */
#include <linux/irq.h>        /* We want an interrupt */
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <asm/delay.h>
#include <linux/delay.h>

#include <asm/uaccess.h>  
#include <asm/io.h>
#include <linux/gpio.h>
#include <mach/mxs.h>





#define LED_PIN         MXS_GPIO_NR(2,1)     //OLIMEX

//#define LED_PIN         MXS_GPIO_NR(0,7)    //STAMP


#define SUCCESS 0
#define DEVICE_NAME "blink" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80    /* Max length of the message from the device */

static int is_device_open = 0; /* Used to prevent multiple access to device */
static int Major;

static int device_open(struct inode *inode, struct file *file)
{

  unsigned int i;
  printk( KERN_INFO "Open BLINKER\n" );
  if (is_device_open)
    return -EBUSY;

  is_device_open = 1;


  for( i=0; i<5; i++ ){
    gpio_set_value(LED_PIN, 1);
    mdelay(0x0040);
    gpio_set_value(LED_PIN, 0);
    mdelay(0x0040);
  }

  try_module_get(THIS_MODULE);

  return SUCCESS;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t count, loff_t * off)
{
  const char cmd = buff[0];
  
  if(cmd=='Q')
  {
    printk(KERN_INFO "Q...\n");
    gpio_set_value(LED_PIN, 1);
  }
  else
    if(cmd=='S'){
      printk(KERN_INFO "S...\n");
      gpio_set_value(LED_PIN, 0);
    }
  
  return 1;
}


static int device_release(struct inode *inode, struct file *file)
{ 
  is_device_open = 0;

  module_put(THIS_MODULE);

  printk( KERN_INFO "Close BLINKER\n" );

  return 0;
}

struct file_operations fops = {
  .open    = device_open,
  .write   = device_write,
  .release = device_release,
};

static int __init blink_init(void)
{
  printk(KERN_INFO "BLINK module is Up.\n");

  Major = register_chrdev(0, DEVICE_NAME, &fops);         

  if (Major < 0) {
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  } 

  printk(KERN_ALERT  "I was assigned major number %d. To talk to\n", Major);
  printk(KERN_ALERT  "the driver, create a dev file with\n");
  printk(KERN_ALERT  "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

  gpio_request(LED_PIN, "led");
  gpio_direction_output(LED_PIN,0);
  gpio_set_value(LED_PIN, 1);


  return 0;
}


static void __exit blink_exit(void)
{
  gpio_set_value(LED_PIN, 0);
  unregister_chrdev(Major, DEVICE_NAME);
  printk( KERN_INFO "BLINK driver is down...\n" );
}


module_init(blink_init);
module_exit(blink_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Camargo <cicamargoba@gmail.com>");
MODULE_DESCRIPTION("BLINKER LED driver");
MODULE_VERSION("1:0.1");



