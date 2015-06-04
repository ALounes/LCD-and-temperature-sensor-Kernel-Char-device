#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/kernel.h>       /* printk() */
//#include <linux/slab.h>         /* kmalloc() */
//#include <linux/fs.h>           /* everything... */
//#include <linux/errno.h>        /* error codes */
//#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>      /* MKDEV,register_chrdev_region, alloc_chrdev_region,MAJOR */
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
//#include <asm/system.h>         /* cli(), *_flags */
#include <asm/uaccess.h>        /* copy_*_user */
#include <asm/io.h>
#include <linux/delay.h>  /* udelay */

#include "lcd.h"

#define DEVICE_NAME "lcd"
 
void tme_tempo(int d);                   //delay function
void PORT_INIT(void);                    //parallel port initialization
void LCD_INIT(void);                     //LCD initialization
void LCD_CMD(unsigned char data);        //write to LCD command register
void LCD_DATA(unsigned char data);       //write to LCD data register
void LCD_STRING(unsigned char *string);  //write string to LCD
void LCD_CLEAR(void);                    //clear LCD and home cursor
void LCD_HOME(void);                     //home cursor on LCD
void LCD_LINE2(void);                    //move cursor to line 2, position 1
void LCD_RS_HIGH(void);
void LCD_RS_LOW(void);
void LCD_E_HIGH(void);
void LCD_E_LOW(void);

void DS1620_CONFIG(unsigned char data);          //DS1620 configuration
void DS1620_WRITECOMMAND(unsigned char command); //Write command to DS1620
void DS1620_WRITEDATA(int data);                 //Write data to DS1620
int DS1620_READ(void);                          //Read data from DS1620
void DS1620_RESET_HIGH(void);
void DS1620_RESET_LOW(void);
void DS1620_CLK_HIGH(void);
void DS1620_CLK_LOW(void);
void DS1620_DATA_HIGH(void);
void DS1620_DATA_LOW(void);
int powerof2(int);

#define LCD_NR_PORTS	3

MODULE_LICENSE("Dual BSD/GPL");

/*
 * all of the parameters have no "shortp_" prefix, to save typing when
 * specifying them at load time
 */
static int major = 124; /* dynamic by default */
module_param(major, int, 0);
int suis_je_un_lapin = 0;

/* default is the first printer port on PC's. "shortp_base" is there too
   because it's what we want to use in the code */
static unsigned long port_base_addr = 0x378;
static unsigned long base = 0x378;
unsigned long shortp_base = 0;
module_param(port_base_addr, long, 0);

unsigned char kernel_buf[100];


void tme_tempo(int d)
{
	udelay(d << 10);
}

void PORT_INIT(void)
{
  // Q3) En utilisant l'appel systeme ioperm(), permettre au processus d'avoir
  // acces aux ports commencant a l'adresse 0x378.
  // Q4) Initialiser le registre de commande de la maniere suivante
  //     -> SLCTIN à 1 (broche 17, bit 3 de 0x37A)
  //     -> INIT à 0 (broche 16, bit 2 de 0x37A)
  //     -> AUTO à 0 (broche 1, bit 1 de 0x37A)
  //     -> STROBE à 0 (broche 14, bit 0 de 0x37A)
	outb(8 ,port_base_addr + 2);
	tme_tempo(10);
	

}

// Q5) Ecrire la fonction LCD_E_HIGH() qui permet de mettre E a 1

void LCD_E_HIGH(void)
{
	outb(inb(port_base_addr + 2) & 0xFE, port_base_addr + 2);
}

// Q6) Ecrire la fonction LCD_E_LOW() qui permet de mettre E a 0

void LCD_E_LOW(void)
{
	outb(inb(port_base_addr + 2) | 1, port_base_addr + 2);
}

// Q7) Ecrire la fonction LCD_RS_HIGH() qui permet de mettre RS a 1

void LCD_RS_HIGH(void)
{
	outb(inb(port_base_addr + 2) & 0xFD, port_base_addr + 2);
}

// Q8) Ecrire la fonction LCD_RS_LOW() qui permet de mettre RS a 0

void LCD_RS_LOW(void)
{
	outb(inb(port_base_addr + 2) | 2, port_base_addr + 2);
}

// Q9) Ecrire la fonction LCD_DATA() qui permet de mettre une valeur
//     dans le registre DATA du port //

void LCD_DATA (unsigned char data) 
{
	outb(data ,port_base_addr);
}

// Q9) Ecrire la fonction LCD_CMD() qui permet d'envoyer une commande
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CMD(unsigned char data)
{
	LCD_E_LOW();
	LCD_RS_LOW();
	LCD_DATA(data);
	LCD_E_HIGH();
	tme_tempo(1);
	LCD_E_LOW();	
}

// Q10) Ecrire la fonction LCD_CHAR() qui permet d'envoyer un caractere
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CHAR(unsigned char data)
{
	LCD_E_LOW();
	LCD_RS_HIGH();
	LCD_DATA(data);
	LCD_E_HIGH();
	tme_tempo(1);
	LCD_E_LOW();
}

// Q11) Ecrire la fonction LCD_STRING() qui permet d'envoyer une
// chaine de caracteres vers le LCD

void LCD_STRING(unsigned char* pdata) 
{
	int i,length = (int)strlen(pdata);
	for(i=0;i < length;i++){
		LCD_CHAR(pdata[i]);
	}
}

// Q12) Ecrire la fonction LCD_CLEAR() qui permet d'effacer l'ecran du LCD

void LCD_CLEAR(void)
{
	LCD_CMD(0x01);
	tme_tempo(1.6);
}

// Q13) Ecrire la fonction LCD_HOME() qui remet le curseur du LCD
// en premiere ligne, premiere colonne

void LCD_HOME(void)
{
	LCD_CMD(0x02);
	tme_tempo(1.6);
}

void LCD_LINE2(void)
{
}

// Q14) Ecrire la fonction LCD_INIT() qui reinitialise le LCD
// La reinitialisation consiste a envoyer la sequence de commandes
// 0x38, 0x38, 0x38, 0x0C, 0x06

void LCD_INIT(void) 
{
	LCD_CMD(0x38);
	LCD_CMD(0x38);
	LCD_CMD(0x38);
	LCD_CMD(0x0C);
	LCD_CMD(0x06);
}

// Q15) Ecrire la fonction DS1620_RESET_HIGH() qui permet 
// de mettre RESET a 1 pour le thermometre

void DS1620_RESET_HIGH(void)
{
	outb(inb(port_base_addr + 2) | 4, port_base_addr + 2);
}

// Q16) Ecrire la fonction DS1620_RESET_LOW() qui permet 
// de mettre RESET a 0 pour le thermometre

void DS1620_RESET_LOW(void)
{
	outb(inb(port_base_addr + 2) & 0xFB, port_base_addr + 2);
}

// Q17) Ecrire la fonction DS1620_CLK_HIGH() qui permet 
// de mettre CLK a 1 pour le thermometre

void DS1620_CLK_HIGH(void)
{
	outb(inb(port_base_addr + 2) & 0xF7, port_base_addr + 2);
}

// Q18) Ecrire la fonction DS1620_CLK_LOW() qui permet 
// de mettre CLK a 0 pour le thermometre

void DS1620_CLK_LOW(void)
{
	outb(inb(port_base_addr + 2) | 8, port_base_addr + 2);
}

// Q19) Ecrire la fonction DS1620_DQ_HIGH() qui permet 
// de mettre DQ a 1 pour le thermometre quand celui ci est en entree

void DS1620_DQ_HIGH(void)
{
	outb(inb(port_base_addr + 2) & 0xFD, port_base_addr + 2);
}

// Q20) Ecrire la fonction DS1620_DQ_LOW() qui permet 
// de mettre DQ a 0 pour le thermometre quand celui ci est en entree

void DS1620_DQ_LOW(void)
{
	outb(inb(port_base_addr + 2) | 0x2, port_base_addr + 2);
}

// Q21) Ecrire la fonction DS1620_WRITECOMMAND() qui permet 
// d'envoyer une commande sur 8 bits au thermometre.
// La commande s'envoie bit apres bit, en commencant par le
// bit de poids faible

void DS1620_WRITECOMMAND(unsigned char command)
{
	int i;
	tme_tempo(2);

	for( i = 0; i < 8; i++){
		DS1620_CLK_LOW();
		tme_tempo(2);
		if(command & 1)
			DS1620_DQ_HIGH();
		else
			DS1620_DQ_LOW();

		tme_tempo(2);
		DS1620_CLK_HIGH();
		tme_tempo(2);
		command = command >> 1;
	}
	tme_tempo(5);

}
// Q22) Ecrire la fonction DS1620_READ() qui permet 
// de lire une temperature sur 9 bits depuis le thermometre
// la temperature se lit bit apres bit en commencant par
// le bit de poids faible.

int DS1620_READ(void)
{
	int ret=0;//,temp = 0;
	int i;

	DS1620_DQ_HIGH();
	tme_tempo(2);
	for( i = 0; i < 9 ; i++){
		DS1620_CLK_LOW();
		tme_tempo(2);
		ret += ((inb(port_base_addr+1) >> 6) & 1) << i;
		tme_tempo(2);
		DS1620_CLK_HIGH();
		tme_tempo(2);
	}

  	return ret;
}

int lcd_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "lcd_couscous: release\n");
	LCD_INIT();
	LCD_HOME();
	LCD_CLEAR();
	if(MINOR(inode->i_rdev) != 0) {
		printk("mauvais numero d unite\n");
		return -EINVAL;
	}
	return 0;
}

int lcd_release(struct inode *inode, struct file *filp)
{
        printk(KERN_INFO "lcd_couscous: release\n");
        return 0;
}

ssize_t lcd_read(struct file *filp, char __user *user_buf, size_t count,
                loff_t *f_pos)
{
	int temperature = 0;


	if(suis_je_un_lapin == 0)
	{
		DS1620_RESET_HIGH();
		DS1620_WRITECOMMAND(READTEMP);
		temperature = DS1620_READ();
		DS1620_RESET_LOW();
		printk(KERN_INFO "lcd_couscous: read |%d|\n",temperature/2);

		tme_tempo(100);
		
		sprintf(kernel_buf,"%d", temperature/2);
		copy_to_user(user_buf, kernel_buf, count);
		suis_je_un_lapin = 1;
		count = 6;
		return count;
	}
	else
	{
		suis_je_un_lapin = 0;
		count = 0;
	}	


	return 0;
}

ssize_t lcd_write(struct file *filp, const char __user *user_buf, size_t count,
                loff_t *f_pos)
{

	if (count > 16) 
		count = 16;

	copy_from_user(kernel_buf,user_buf,count);

	kernel_buf[count-1]='\0';
	LCD_STRING(kernel_buf);

	printk(KERN_INFO "lcd_couscous: write %s \n",kernel_buf);
	
        return count;
}

int lcd_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
        return 0;
}

struct file_operations lcd_fops = 
{
        .owner =    THIS_MODULE,
        .read =     lcd_read,
        .write =    lcd_write,
        //.ioctl =    lcd_ioctl,
        .open =     lcd_open,
        .release =  lcd_release,
};

static int lcd_init(void)
{
	int err = 0;

	shortp_base = base;

	if (err = register_chrdev(major,"lcd",&lcd_fops))
		printk("*** unable to get major LCD_MAJOR for Lcd devices ***\n");

	PORT_INIT();
	tme_tempo(10);

	printk(KERN_ALERT "lcd_couscous:lcd_init\n");
	printk(KERN_ALERT "lcd_couscous:basePortAddress is 0x%lx\n", shortp_base);

	return err;
}

static void lcd_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	printk(KERN_ALERT "lcd_couscous:lcd_exit\n");
}

module_init(lcd_init);
module_exit(lcd_exit);
