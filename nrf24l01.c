/**
* @file nrf24l01.c
* @author anoel
* @version 0.1
* @date 2 f�vrier 2024
* @brief Cette biblioth�que impl�mente les fonctionnalit�es n�c�ssaire pour communiquer avec un nrf24l01.
* @hardware Nordic semiconductor nrf24l01, utilis� avec ATmega 32u4.
* @datasheet https://www.sparkfun.com/datasheets/Components/nRF24L01_prelim_prod_spec_1_2.pdf
*/ 

#include "nrf24l01.h"

//Variables globales du fichier nrf24l01.c
uint8_t* rxDataBuffer=0;
uint8_t rxDataBufferSize=0;

////////////////////////////////////////////////////////////////////////////// FONCTIONS "PRIV�ES" /////////////////////////////////////////////////////////////////////

/*
* @brief Sert � �crire un octet dans un registre.
* @param uint8_t reg : le registre du nrf24l01 dans lequel il faut �crire, uint8_t value : valeur � �crire dans reg.
* @return void.
*/
void _nrf24l01WriteRegister(uint8_t reg, uint8_t value)
{
	CS_PIN_DOWN;										//Met la pin de Chip Select du master (32u4) � 0 pour que le slave soit � l'�coute (nrf24l01)
	spiMasterReadWrite(W_REGISTER | (reg & 0x1F));		//Transmission spi avec commande d'�criture registre W_REGISTER = 001A AAAA, donc 0x20. Un masque OU est appliqu� avec la valeur du registre qui va au maximum � 31, d'o� le masque ET 0x1F avec la valeur reg.
	spiMasterReadWrite(value);							//Transmet la valeur � �crire dans le registre.
	CS_PIN_UP;											//Impulsion de Chip Enable du nrf24l01 pour envoyer les donn�es.
}

/*********************************************************************************************************************************************************************/

/*
* @brief Tout d�pendant de l'�tat de rw. Si rw=1, la fonction �crit plusieurs octets dans le registre reg. Si rw=0, la fonction lit toutes les donn�es contenues dans le registre R_RX_PAYLOAD.
* @param uint8_t rw : "read" ou "write", uint8_t reg : registre vis� par l'action rw, uint8_t* value : pointeur de donn�es du tableau d'octets � �crire dans le registre, uint8_t size : taille du tableau value.
* @return return 1 : si la radio est disponible, return 0 : si la radio est indisponible.
*/
uint8_t *_nrf24l01WriteMoreRegister(uint8_t rw, uint8_t reg, uint8_t* value, uint8_t size)
{
	if(rw == 1)
		reg=W_REGISTER | reg;
		
	static uint8_t ret[32];
	
	CS_PIN_DOWN;
	spiMasterReadWrite(reg);
	
	for(uint8_t i=0; i<size; i++)
	{
		if(rw==0 && reg != W_TX_PAYLOAD)
		{
			ret[i]=spiMasterReadWrite(NOP);
		}
		else
		{
			spiMasterReadWrite(value[i]);
		}
	}
	
	CS_PIN_UP;
	
	return ret;
}

/*********************************************************************************************************************************************************************/

/*
* @brief Retourne la valeur contenue dans un registre s�l�ctionn�.
* @param uint8_t reg : registre � lire.
* @return uint8_t reg : retourne la valeur du registre reg.
*/
uint8_t _nrf24l01ReadRegister(uint8_t reg)
{
	CS_PIN_DOWN;
	spiMasterReadWrite(R_REGISTER | reg);
	reg=spiMasterReadWrite(NOP);
	CS_PIN_UP;
	return reg;
}

////////////////////////////////////////////////////////////////////////////// FONCTIONS "PUBLIQUES" //////////////////////////////////////////////////////////////////


void nrf24l01Init(uint8_t mode, uint8_t* address, uint8_t channel)
{
	spiMasterInit();								//Initialise le spi pour un AtMega 32u4.
	
	CE_PIN_INIT;									//Initialise la pin de Chip Enable du nrf24l01.
		
	_delay_ms(100);
	
	uint8_t val[5];
	
	val[0]=0x3f;
	_nrf24l01WriteMoreRegister(1, EN_AA, val, 1);	//Se r�f�rer � la "datasheet" pour la configuration des registres plus bas. (Flemme de tous les d�tailler).
	
	val[0]=0x00;
	_nrf24l01WriteMoreRegister(1, SETUP_RETR, val, 1);
	
	val[0]=0x03;
	_nrf24l01WriteMoreRegister(1, EN_RXADDR, val, 1);
	
	val[0]=0x03;
	_nrf24l01WriteMoreRegister(1, SETUP_AW, val, 1);
	
	val[0]=channel;
	_nrf24l01WriteMoreRegister(1, RF_CH, val, 1);
	
	val[0]=0x07;
	_nrf24l01WriteMoreRegister(1, RF_SETUP, val, 1);
	
	int i;
	for(i=0; i<5; i++)
		val[i]=address[i];
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P0, val, 5);
	
	for(i=0; i<5; i++)//AJOUT
		val[i]=0xc2;
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P1, val, 5);//AJOUT
	
	val[0]=0xc3;
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P2, val, 1);//AJOUT
	
	val[0]=0xc4;
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P3, val, 1);//AJOUT
	
	val[0]=0xc5;
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P4, val, 1);//AJOUT
	
	val[0]=0xc6;
	_nrf24l01WriteMoreRegister(1, RX_ADDR_P5, val, 1);//AJOUT

	for(i=0; i<5; i++)
		val[i]=0x12;
	_nrf24l01WriteMoreRegister(1, TX_ADDR, val, 5);
	
	val[0]=0x20;
	_nrf24l01WriteMoreRegister(1, RX_PW_P0, val, 1);
	_nrf24l01WriteMoreRegister(1, RX_PW_P1, val, 1);
	_nrf24l01WriteMoreRegister(1, RX_PW_P2, val, 1);
	_nrf24l01WriteMoreRegister(1, RX_PW_P3, val, 1);
	_nrf24l01WriteMoreRegister(1, RX_PW_P4, val, 1);
	_nrf24l01WriteMoreRegister(1, RX_PW_P5, val, 1);
	
	if(mode == TX_MODE)
	{
		val[0]=0x0E;
		_nrf24l01WriteMoreRegister(1, CONFIG, val, 1);
		CE_PIN_DOWN;
	}
	else if(mode == RX_MODE)
	{
		val[0]=0x0F;
		_nrf24l01WriteMoreRegister(1, CONFIG, val, 1);
		CE_PIN_UP;
	}
	_delay_ms(100);
}

/*********************************************************************************************************************************************************************/

uint8_t nrf24l01Available()
{	
	if(((_nrf24l01ReadRegister(STATUS)&(1<<RX_DR))!=0) && ((_nrf24l01ReadRegister(FIFO_STATUS)&(1<<RX_EMPTY))!=1))	//V�rifie que la fifo rx re�ois des donn�es et qu'elle n'est pas vide.(La deuxi�eme condition est redondante mais ne sait-on jamais)
		return 1;																									//Retourne 1 si le module re�oit des donn�es et est donc disponible
	return 0;																										//Retourne 0 si le module ne re�oit pas de donn�es et n'est donc pas disponible.
}

/*********************************************************************************************************************************************************************/

void nrf24l01Transmit(uint8_t *payload, uint8_t size)
{	
	for(uint8_t i=size; i<32; i++)
		payload[i]=0x00;																						//Remplit le reste du tableau de 0, ce n'est pas la meilleure solution!
	
	CE_PIN_UP;
		
	_nrf24l01WriteMoreRegister(0, FLUSH_TX, payload, 0);														//"Flush" le buffer TX.
	_nrf24l01WriteMoreRegister(0, W_TX_PAYLOAD, payload, 32);													//�crit les donn�es vers la fifo tx, il n'est pas obligatoire d'�crire sur les 32 octets de la fifo tx.
	
	while(!(_nrf24l01ReadRegister(STATUS) & ((1<<TX_DS) | (1<<MAX_RT))));										//Attend que la transmission soit complet�e, aucun m�canisme en place si jamais il se passe un probl�me (oups).

	CE_PIN_DOWN;

	_nrf24l01WriteRegister(STATUS, (_nrf24l01ReadRegister(STATUS) | (1<<TX_DS) | (1<<MAX_RT) | (1<<RX_DR)));	//"Clear" les "flags" qui doivent l'�tre apr�s la transmission.
}

/*********************************************************************************************************************************************************************/

uint8_t* nrf24l01Receive()
{
	if((_nrf24l01ReadRegister(FIFO_STATUS) & (1<<RX_EMPTY))!=0)
		_nrf24l01WriteMoreRegister(0, FLUSH_TX, rxDataBuffer, 0);				//"Flush" la fifo rx si elle est au maximum de sa capacit�.
		
	rxDataBuffer=_nrf24l01WriteMoreRegister(0, R_RX_PAYLOAD, rxDataBuffer, 32);	
		
	_nrf24l01WriteRegister(STATUS, _nrf24l01ReadRegister(STATUS) | (1<<RX_DR));	//R�initialiser le bit RX_DR n'est pas obligatoire, mais c'est une bonne pratique.
	
	return rxDataBuffer;
}

/*********************************************************************************************************************************************************************/

uint8_t nrf24l01RxBufferSize(void)
{
	rxDataBufferSize=0;
	
	while (rxDataBuffer[rxDataBufferSize] != 0x00)	//Rend la taille du tableau, ce n'est pas la meilleure des solutions car si une donn�es utile dans le tableau est �gale � 0, la lecture va s'arr�ter � cette donn�e.
	{
		rxDataBufferSize++;
	}	
	return rxDataBufferSize;
}

/*********************************************************************************************************************************************************************/

void powerDownMode(void)
{
	CE_PIN_DOWN;
	_nrf24l01WriteRegister(CONFIG, _nrf24l01ReadRegister(CONFIG) & ~(1<<PWR_UP));	//Met PWR_UP � 0 pour passer en mode power down.
}

/*********************************************************************************************************************************************************************/

void powerUpMode(void)
{
	_nrf24l01WriteRegister(CONFIG, _nrf24l01ReadRegister(CONFIG) | (1<<PWR_UP));	//Met PWR_UP � 1 pour passer en mode power up.
}

/*********************************************************************************************************************************************************************/


