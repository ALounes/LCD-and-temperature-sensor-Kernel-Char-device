#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/io.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

/******** DS1620 COMMAND DEFINITIONS ***************************************/

#define READTEMP 0xaa                //Read Temperature
#define READCOUNTER 0xa0             //Read Counter register
#define READSLOPE 0xa9               //Read Slope counter register
#define STARTCONVERT 0xee            //Start temperature conversion
#define STOPCONVERT 0x22             //Stop temperature conversion

#define WRITETH  0x01                //Write to TH register
#define WRITETL  0x02                //Write to TL register
#define READTH   0xa1                //Read TH register
#define READTL   0xa2                //Read TL register
#define WRITECONFIG 0x0c             //Write to Configuration register
#define READCONFIG 0xac              //Read Configuration register

int port;

void tme_tempo(int d);                   //delay function
void PORT_INIT(void);                    //parallel port initialization
void LCD_E_HIGH(void);
void LCD_E_LOW(void);
void LCD_RS_HIGH(void);
void LCD_RS_LOW(void);
void LCD_DATA(unsigned char data);       //write to port // data  reg
void LCD_CMD(unsigned char data);        //write LCD command register
void LCD_CHAR(unsigned char data);       //write LCD command register
void LCD_STRING(unsigned char *string);  //write string to LCD
void LCD_INIT(void);                     //LCD initialization
void LCD_CLEAR(void);                    //clear LCD and home cursor
void LCD_HOME(void);                     //home cursor on LCD

void DS1620_RESET_HIGH(void);
void DS1620_RESET_LOW(void);
void DS1620_CLK_HIGH(void);
void DS1620_CLK_LOW(void);
void DS1620_DATA_HIGH(void);
void DS1620_DATA_LOW(void);
void DS1620_WRITECOMMAND(unsigned char command); //Write command to DS1620
int DS1620_READ(void);                          //Read data from DS1620

// Q1) Definir l'adresse de base des registres du port // 
#define port_base_addr 0x378

int file_des;
// Q2) Ecrire la fonction tem_tempo() qui permet d'attendre 'tempo' millisecondes
//     a l'aide de l'appel systeme usleep

void tme_tempo(int d)
{
	usleep(d*1000);
}

void PORT_INIT(void)
{
  // Q3) En utilisant l'appel systeme ioperm(), permettre au processus d'avoir
  // acces aux ports commencant a l'adresse 0x378.
	//ioperm(port_base_addr, 3, 1);
	file_des = open("/dev/parport0", O_RDWR);
	if(ioctl(file_des,PPCLAIM)){
		printf("erreur claim");
		exit(1);
	}
  // Q4) Initialiser le registre de commande de la maniere suivante
  //     -> SLCTIN à 1 (broche 17, bit 3 de 0x37A)
  //     -> INIT à 0 (broche 16, bit 2 de 0x37A)
  //     -> AUTO à 0 (broche 1, bit 1 de 0x37A)
  //     -> STROBE à 0 (broche 14, bit 0 de 0x37A)
	//outb(8 ,port_base_addr + 2);
	unsigned char byte = 0x08;
	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	tme_tempo(10);
	

}

// Q5) Ecrire la fonction LCD_E_HIGH() qui permet de mettre E a 1

void LCD_E_HIGH(void)
{
	//outb(inb(port_base_addr + 2) & 0xFE, port_base_addr + 2);

	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte & 0xFE;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q6) Ecrire la fonction LCD_E_LOW() qui permet de mettre E a 0

void LCD_E_LOW(void)
{
	//outb(inb(port_base_addr + 2) | 1, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte | 0x1;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q7) Ecrire la fonction LCD_RS_HIGH() qui permet de mettre RS a 1

void LCD_RS_HIGH(void)
{
	//outb(inb(port_base_addr + 2) & 0xFD, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte & 0xFD;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q8) Ecrire la fonction LCD_RS_LOW() qui permet de mettre RS a 0

void LCD_RS_LOW(void)
{
	//outb(inb(port_base_addr + 2) | 2, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte | 0x2;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q9) Ecrire la fonction LCD_DATA() qui permet de mettre une valeur
//     dans le registre DATA du port //

void LCD_DATA (unsigned char data) 
{
	//outb(data ,port_base_addr);

	if(ioctl(file_des,PPWDATA,&data)){
		printf("erreur");
		exit(1);
	}
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
	//outb(inb(port_base_addr + 2) | 4, port_base_addr + 2);
	
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte | 0x4;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q16) Ecrire la fonction DS1620_RESET_LOW() qui permet 
// de mettre RESET a 0 pour le thermometre

void DS1620_RESET_LOW(void)
{
	//outb(inb(port_base_addr + 2) & 0xFB, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte & 0xFB;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q17) Ecrire la fonction DS1620_CLK_HIGH() qui permet 
// de mettre CLK a 1 pour le thermometre

void DS1620_CLK_HIGH(void)
{
	//outb(inb(port_base_addr + 2) & 0xF7, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte & 0xF7;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q18) Ecrire la fonction DS1620_CLK_LOW() qui permet 
// de mettre CLK a 0 pour le thermometre

void DS1620_CLK_LOW(void)
{
	//outb(inb(port_base_addr + 2) | 8, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte | 0x8;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q19) Ecrire la fonction DS1620_DQ_HIGH() qui permet 
// de mettre DQ a 1 pour le thermometre quand celui ci est en entree

void DS1620_DQ_HIGH(void)
{
	//outb(inb(port_base_addr + 2) & 0xFD, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte & 0xFD;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q20) Ecrire la fonction DS1620_DQ_LOW() qui permet 
// de mettre DQ a 0 pour le thermometre quand celui ci est en entree

void DS1620_DQ_LOW(void)
{
	//outb(inb(port_base_addr + 2) | 0x2, port_base_addr + 2);
	unsigned char byte;
	if(ioctl(file_des,PPRCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
	byte = byte | 0x2;

	if(ioctl(file_des,PPWCONTROL,&byte)){
		printf("erreur");
		exit(1);
	}
}

// Q21) Ecrire la fonction DS1620_WRITECOMMAND() qui permet 
// d'envoyer une commande sur 8 bits au thermometre.
// La commande s'envoie bit apres bit, en commencant par le
// bit de poids faible

void DS1620_WRITECOMMAND(unsigned char command)
{
	//DS1620_CLK_HIGH();
	tme_tempo(2);
	int i;
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
	int ret=0,temp = 0;
	//DS1620_CLK_HIGH();
	int i;
	unsigned char byte = 0;

	DS1620_DQ_HIGH();
	tme_tempo(2);
	for( i = 0; i < 9 ; i++){
		byte = 0;
		DS1620_CLK_LOW();
		tme_tempo(2);
		if(ioctl(file_des,PPRSTATUS,&byte)){
			printf("erreur lecture");
			exit(1);
		}
		ret += ((byte >> 6) & 1) << i;
		tme_tempo(2);
		DS1620_CLK_HIGH();
		tme_tempo(2);
	}

  	return ret;
}


int main(int argc,char *argv[])
{
	int temp;
	char tempstring [16];

	tme_tempo(250);
	PORT_INIT();
	LCD_INIT();
	LCD_CLEAR();
	
	LCD_STRING("L & M");
	tme_tempo(1000);
	
  	DS1620_RESET_HIGH();
	DS1620_WRITECOMMAND(STARTCONVERT);
  	DS1620_RESET_LOW();
	tme_tempo(1000);

	while (1)
	{
  		DS1620_RESET_HIGH();
		DS1620_WRITECOMMAND(READTEMP);
		temp=DS1620_READ();
  		DS1620_RESET_LOW();
		tme_tempo(1000);
		sprintf(tempstring,"Couscous %1.1f C",temp/2.0);
		LCD_CLEAR();
		LCD_STRING(tempstring);
	}

	return 0;
}
