#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/io.h>
#include <string.h>

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

// adresse de base des registres du port // 
#define PORT_BASE 0x378

#define DATA_PORT   PORT_BASE
#define STATUS_PORT  PORT_BASE + 1
#define CONTROL_PORT PORT_BASE + 2


// la fonction tem_tempo() qui permet d'attendre 'tempo' millisecondes
// a l'aide de l'appel systeme usleep

void tme_tempo(int d)
{
	usleep(d*1000);
}

void PORT_INIT(void)
{
  // l'appel systeme ioperm(), permet au processus d'avoir
  // acces aux ports commencant a l'adresse 0x378.
	ioperm(DATA_PORT, 3, 1);
  //     Initialise le registre de commande de la maniere suivante
  //     -> SLCTIN à 1 (broche 17, bit 3 de 0x37A)
  //     -> INIT à 0 (broche 16, bit 2 de 0x37A)
  //     -> AUTO à 0 (broche 1, bit 1 de 0x37A)
  //     -> STROBE à 0 (broche 14, bit 0 de 0x37A)
	outb(3,CONTROL_PORT);	
	tme_tempo(10);
}

// Q5) Ecrire la fonction LCD_E_HIGH() qui permet de mettre E a 1

void LCD_E_HIGH(void)
{
	outb( inb(CONTROL_PORT) & ~(1<<0), CONTROL_PORT);
}

// Q6) Ecrire la fonction LCD_E_LOW() qui permet de mettre E a 0

void LCD_E_LOW(void)
{
	outb( inb(CONTROL_PORT) | (1<<0), CONTROL_PORT);
}

// Q7) Ecrire la fonction LCD_RS_HIGH() qui permet de mettre RS a 1

void LCD_RS_HIGH(void)
{
	outb( inb(CONTROL_PORT) & ~(1<<1), CONTROL_PORT);
}

// Q8) Ecrire la fonction LCD_RS_LOW() qui permet de mettre RS a 0

void LCD_RS_LOW(void)
{
	outb( inb(CONTROL_PORT) | (1<<1), CONTROL_PORT);
}

// Q9) Ecrire la fonction LCD_DATA() qui permet de mettre une valeur
//     dans le registre DATA du port //

void LCD_DATA (unsigned char data) 
{
  /* Positionne les données sur les entrées D0 à D7 du LCD
     On écrit le caractère dans le registre 0x378 */
		outb(data, DATA_PORT);
}

// Q9) Ecrire la fonction LCD_CMD() qui permet d'envoyer une commande
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CMD(unsigned char data)
{
		LCD_DATA( data);
		LCD_RS_LOW();
		tme_tempo(1);
		LCD_E_HIGH();
		tme_tempo(1);
		LCD_E_LOW();
		tme_tempo(1);
}

// Q10) Ecrire la fonction LCD_CHAR() qui permet d'envoyer un caractere
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CHAR(unsigned char data)
{
		LCD_DATA( data);
		LCD_RS_HIGH();
		tme_tempo(1);
		LCD_E_HIGH();
		tme_tempo(1);
		LCD_E_LOW();
		tme_tempo(1);
}

// Q11) Ecrire la fonction LCD_STRING() qui permet d'envoyer une
// chaine de caracteres vers le LCD

void LCD_STRING(unsigned char* pdata) 
{
  /* Affiche une chaîne de caractères sur l'écran */
	while (*pdata)
	{
		LCD_CHAR(*pdata);
		pdata++;
	}
}

// Q12) Ecrire la fonction LCD_CLEAR() qui permet d'effacer l'ecran du LCD

void LCD_CLEAR(void)
{
	LCD_CMD(0x01);
}

// Q13) Ecrire la fonction LCD_HOME() qui remet le curseur du LCD
// en premiere ligne, premiere colonne

void LCD_HOME(void)
{
	LCD_CMD(0x02);
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
	outb( inb(CONTROL_PORT) | (1<<2), CONTROL_PORT);
}

// Q16) Ecrire la fonction DS1620_RESET_LOW() qui permet 
// de mettre RESET a 0 pour le thermometre

void DS1620_RESET_LOW(void)
{
	outb( inb(CONTROL_PORT) & ~(1<<2), CONTROL_PORT);
}

// Q17) Ecrire la fonction DS1620_CLK_HIGH() qui permet 
// de mettre CLK a 1 pour le thermometre

void DS1620_CLK_HIGH(void)
{
	outb( inb(CONTROL_PORT) & ~(1<<3), CONTROL_PORT);
}

// Q18) Ecrire la fonction DS1620_CLK_LOW() qui permet 
// de mettre CLK a 0 pour le thermometre

void DS1620_CLK_LOW(void)
{
	outb( inb(CONTROL_PORT) | (1<<3), CONTROL_PORT);
}

// Q19) Ecrire la fonction DS1620_DQ_HIGH() qui permet 
// de mettre DQ a 1 pour le thermometre quand celui ci est en entree

void DS1620_DQ_HIGH(void)
{
	outb( inb(CONTROL_PORT) & ~(1<<1), CONTROL_PORT);
}

// Q20) Ecrire la fonction DS1620_DQ_LOW() qui permet 
// de mettre DQ a 0 pour le thermometre quand celui ci est en entree

void DS1620_DQ_LOW(void)
{
	outb( inb(CONTROL_PORT) | (1<<1), CONTROL_PORT);
}

// Q21) Ecrire la fonction DS1620_WRITECOMMAND() qui permet 
// d'envoyer une commande sur 8 bits au thermometre.
// La commande s'envoie bit apres bit, en commencant par le
// bit de poids faible

void DS1620_WRITECOMMAND(unsigned char command)
{
	int i;

	for(i = 0; i<8 ; i++)
	{
		DS1620_CLK_LOW();
		tme_tempo(1);
		if (command & 1)
		{
			DS1620_DQ_HIGH();
			printf("1 "); fflush(stdout);
		}
		else
		{
			DS1620_DQ_LOW();
			printf("0 "); fflush(stdout);
		}

		//outb( inb(0x37A)|(inb(0x37A) & (((command<<i)&1)<<1)), 0x37A);

		tme_tempo(1);
		DS1620_CLK_HIGH();
		tme_tempo(1);
		command >>=1;
	}
	printf("\n");
}


// Q22) Ecrire la fonction DS1620_READ() qui permet 
// de lire une temperature sur 9 bits depuis le thermometre
// la temperature se lit bit apres bit en commencant par
// le bit de poids faible.

int DS1620_READ(void)
{
	int ret=0;

	int i;
	
	DS1620_DQ_HIGH();

	for(i = 0; i<9 ; i++)
	{
		DS1620_CLK_LOW();
		if (inb(STATUS_PORT) & 0x40)
		{
			ret+=1<<i;
			printf("1 "); fflush(stdout);
		}
		else
		{
			printf("0 "); fflush(stdout);
		}

		//ret |=  ((inb(0x379) >> 6) & 1) << i;

		tme_tempo(1);
		DS1620_CLK_HIGH();
		tme_tempo(1);
	}
	printf("\n");

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
	LCD_STRING("L & M ");
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
		sprintf(tempstring,"Couscous %1.1f'",temp/2.0);
		LCD_CLEAR();
		LCD_STRING(tempstring);
	}


	return 0;
}
