/*  
 *   *  hello-1.c - The simplest kernel module.
 *    */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */

static int __init hello_init_module(void)
{
		printk(KERN_INFO "Hello world 1.\n");

			/* 
			 * 	 * A non 0 return means init_module failed; module can't
			 * 	 be loaded. 
			 * 	 	 */
			return 0;
}

static void __exit hello_cleanup_module(void)
{
		printk(KERN_INFO "Goodbye world 1.\n");
}

module_init(hello_init_module);
module_exit(hello_cleanup_module);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pedro H. Penna");
