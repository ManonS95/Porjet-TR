#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/wait.h>
#include <linux/init.h>		/* module_{init,exit}() */
#include <linux/slab.h>		/* kmalloc()/kfree() */
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/uaccess.h>


#define  DEVICE_NAME "encodeur"    ///< The device will appear at /dev/mychar using this value
#define  CLASS_NAME  "encodeur"        ///< The device class -- this is a character device driv

static struct class*  charClass  = NULL; ///< The device-driver class struct pointer
static struct device* charDevice = NULL; ///< The device-driver device struct pointer

int major;

MODULE_LICENSE( "GPL" );
MODULE_DESCRIPTION( "Pilote pour récupération Interruptions sur Raspberry Pi" );



// Numéros des pins utilisés :
#define IRQ_PIN_1 22
#define IRQ_PIN_2 23

// Adresse de base pour les registres correspondant aux GPIO de la Raspberry Pi :
static volatile unsigned* gpio;

// Définition des macros pour l'utilisation directe des GPIO de la Raspberry Pi :
#define INP_GPIO(g) *(gpio + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio + (((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio + ((g)/10)) |= (1<<(((g)%10)*3))
#define GET_GPIO(g) (*(gpio + 13) & (1<<g)) // 0 if LOW and (1 << g) if HIGH

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

// Les variables globales et les fonctions sont déclarées statiques afin qu'elles ne soient visibles
// que dans le code de ce fichier. On évite ainsi toute ambiguïté avec autres variables du noyau
// qui partage le même segment de mémoire.

// Valeur correspondant à la position du codeur :
static long encoder_count = 0;

// Déclaration des numéros de pins comme paramètres du module :
static int irq_pin_1 = IRQ_PIN_1;
static int irq_pin_2 = IRQ_PIN_2;

module_param( irq_pin_1, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
module_param( irq_pin_2, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );


// Structure utilisée pour traiter les interruptions :
typedef struct encoder_data {
	char* label;
	int irq_pin;
	long* count;
	unsigned int irq;
} encoder_data;

// Déclaration des structures de données utilisées pour traiter les interruptions :
static encoder_data data_1;



/*
 * File operations
 */
static ssize_t char_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
  int err;
  char str[20];
  sprintf(str, "%ld", encoder_count);
  printk("encodeur_count = %ld\n", encoder_count);
  if (count < 20) { return 0; }
  err = copy_to_user(buf, str, count);
  if(err != 0)
    printk(KERN_ALERT "The message from driver buffer has not been copied: %d bytes were not copied!\n", err);
  return count;
}

static struct file_operations char_fops = {
	.owner =	THIS_MODULE,
	.read =		char_read,
};

//-------------------------------------
// Fonctions gérant les interruptions :
//-------------------------------------

// Fonction appelée à chaque interruption :
static irqreturn_t encoder_irq_handler( int irq, void* dev_id )
{
	// Interprétation du pointeur vers les données de l'interruption :
	encoder_data* data = (encoder_data*) dev_id;

	int pin_2 = GET_GPIO(IRQ_PIN_2);
	printk("pin 2 = %d\n", pin_2);
	if (pin_2 > 0)
		(*data->count)--;
	else
		(*data->count)++;

	printk("irq_rcvd count = %ld \n", *data->count);
	return IRQ_HANDLED;
}


// Fonction pour allouer les pins :
static void setup_irq_pin( encoder_data* data )
{
	// Initialisations propre à la Raspberry Pi :
	INP_GPIO( data->irq_pin );
	SET_GPIO_ALT( data->irq_pin, 0 );

	// Allocation de l'interruption :
	gpio_request_one( data->irq_pin, GPIOF_IN, data->label ); // Réservation système en input

	data->irq = gpio_to_irq( data->irq_pin ); // Recherche du numéro d'interruption à partir du numéro de pin

	if ( request_any_context_irq(
		data->irq,
		encoder_irq_handler,
		IRQF_TRIGGER_RISING,
		THIS_MODULE->name,
		data) >= 0 )
		printk( KERN_INFO "%s: interruption \"%s\" allocated on line %u\n", THIS_MODULE->name, data->label, data->irq );
}

// Fonction pour libérer les pins :
static void free_encoder_pins( encoder_data* data )
{
	free_irq( data->irq, data );
	gpio_free( data->irq_pin );
}


//-----------------------------
// Fonctions gérant le module :
//-----------------------------

// Fonction appelée au chargement du module :
int init_module( void )
{
	void* gpio_map;

	// Translation des adresses pour l'utilisation directe des GPIO de la Raspberry Pi :
	gpio_map = ioremap( 0x3F200000, SZ_16K );
	if ( gpio_map == NULL )
	{
		printk( KERN_ALERT "%s: ioremap failed !\n", THIS_MODULE->name );
		return -EBUSY;
	}
	gpio = (volatile unsigned*) gpio_map;

	// Remplissage des structures de données utilisées pour traiter les interruptions :
	data_1.label = "irq 1";
	data_1.irq_pin = irq_pin_1;
	data_1.count = &encoder_count;

	INP_GPIO( irq_pin_2 );

	// Allocation des pins :
	setup_irq_pin( &data_1 );
	// enable pull-up on GPIO24&25
    GPIO_PULL = 2;
    // clock on GPIO 22 & 23 (bit 24 & 25 set)
    GPIO_PULLCLK0 = (1 << IRQ_PIN_1) | (1 << IRQ_PIN_2);	


	int ret;
	ret = register_chrdev(major, "encodeur", &char_fops);
	if (ret<0){
		return ret;
	}
	major = ret;

	/****** Automatically creating virtual files in /dev ******/
	// Register the device class
	charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charClass)){                // Check for error and clean up if there is
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(charClass);          // Correct way to return an error on a pointer
	}
	
	// Register the device driver
	charDevice = device_create(charClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charDevice)){               // Clean up if there is an error
		class_destroy(charClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(major, DEVICE_NAME);
		return PTR_ERR(charDevice);
	}

  	return 0;
}

// Fonction appelée au retrait du module :
void cleanup_module( void )
{
	// Libération des pins :
	free_encoder_pins( &data_1 );
	GPIO_PULL = 0;
    GPIO_PULLCLK0 = 0;

	printk( KERN_INFO "%s: module removed\n", THIS_MODULE->name );

	device_destroy(charClass, MKDEV(major, 0));     // remove the device
	class_unregister(charClass);                          // unregister the device class
	class_destroy(charClass);                             // remove the device class
	
	unregister_chrdev(major, "encodeur");
}