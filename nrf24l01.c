/**
* @file nrf24l01.c
* @author anoel
* @version 0.1
* @date 2 février 2024
* @brief Cette bibliothèque implémente les fonctionnalitées nécéssaire pour communiquer avec un nrf24l01.
* @hardware Nordic semiconductor nrf24l01, utilisé avec ATmega 32u4.
* @datasheet https://www.sparkfun.com/datasheets/Components/nRF24L01_prelim_prod_spec_1_2.pdf
*/ 

#include "nrf24l01.h"

//Variables globales du fichier nrf24l01.c
uint8_t* rxDataBuffer=0;
uint8_t rxDataBufferSize=0;

////////////////////////////////////////////////////////////////////////////// FONCTIONS "PRIVÉES" /////////////////////////////////////////////////////////////////////

/*
* @brief Sert à écrire un octet dans un registre.
* @param uint8_t reg : le registre du nrf24l01 dans lequel il faut écrire, uint8_t value : valeur à écrire dans reg.
* @return void.
*/
void _nrf24l01WriteRegister(uint8_t reg, uint8_t value)
{
	CS_PIN_DOWN;										//Met la pin de Chip Select du master (32u4) à 0 pour que le slave soit à l'écoute (nrf24l01)
	spiMasterReadWrite(W_REGISTER | (reg & 0x1F));		//Transmission spi avec commande d'écriture registre W_REGISTER = 001A AAAA, donc 0x20. Un masque OU est appliqué avec la valeur du registre qui va au maximum à 31, d'où le masque ET 0x1F avec la valeur reg.
	spiMasterReadWrite(value);							//Transmet la valeur à écrire dans le registre.
	CS_PIN_UP;											//Impulsion de Chip Enable du nrf24l01 pour envoyer les données.
}

/*********************************************************************************************************************************************************************/

/*
* @brief Tout dépendant de l'état de rw. Si rw=1, la fonction écrit plusieurs octets dans le registre reg. Si rw=0, la fonction lit toutes les données contenues dans le registre R_RX_PAYLOAD.
* @param uint8_t rw : "read" ou "write", uint8_t reg : registre visé par l'action rw, uint8_t* value : pointeur de données du tableau d'octets à écrire dans le registre, uint8_t size : taille du tableau value.
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
* @brief Retourne la valeur contenue dans un registre séléctionné.
* @param uint8_t reg : registre à lire.
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
	_nrf24l01WriteMoreRegister(1, EN_AA, val, 1);	//Se référer à la "datasheet" pour la configuration des registres plus bas. (Flemme de tous les détailler).
	
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
	if(((_nrf24l01ReadRegister(STATUS)&(1<<RX_DR))!=0) && ((_nrf24l01ReadRegister(FIFO_STATUS)&(1<<RX_EMPTY))!=1))	//Vérifie que la fifo rx reçois des données et qu'elle n'est pas vide.(La deuxièeme condition est redondante mais ne sait-on jamais)
		return 1;																									//Retourne 1 si le module reçoit des données et est donc disponible
	return 0;																										//Retourne 0 si le module ne reçoit pas de données et n'est donc pas disponible.
}

/*********************************************************************************************************************************************************************/

void nrf24l01Transmit(uint8_t *payload, uint8_t size)
{	
	for(uint8_t i=size; i<32; i++)
		payload[i]=0x00;																						//Remplit le reste du tableau de 0, ce n'est pas la meilleure solution!
	
	CE_PIN_UP;
		
	_nrf24l01WriteMoreRegister(0, FLUSH_TX, payload, 0);														//"Flush" le buffer TX.
	_nrf24l01WriteMoreRegister(0, W_TX_PAYLOAD, payload, 32);													//Écrit les données vers la fifo tx, il n'est pas obligatoire d'écrire sur les 32 octets de la fifo tx.
	
	while(!(_nrf24l01ReadRegister(STATUS) & ((1<<TX_DS) | (1<<MAX_RT))));										//Attend que la transmission soit completée, aucun mécanisme en place si jamais il se passe un problème (oups).

	CE_PIN_DOWN;

	_nrf24l01WriteRegister(STATUS, (_nrf24l01ReadRegister(STATUS) | (1<<TX_DS) | (1<<MAX_RT) | (1<<RX_DR)));	//"Clear" les "flags" qui doivent l'être après la transmission.
}

/*********************************************************************************************************************************************************************/

uint8_t* nrf24l01Receive()
{
	if((_nrf24l01ReadRegister(FIFO_STATUS) & (1<<RX_EMPTY))!=0)
		_nrf24l01WriteMoreRegister(0, FLUSH_TX, rxDataBuffer, 0);				//"Flush" la fifo rx si elle est au maximum de sa capacité.
		
	rxDataBuffer=_nrf24l01WriteMoreRegister(0, R_RX_PAYLOAD, rxDataBuffer, 32);	
		
	_nrf24l01WriteRegister(STATUS, _nrf24l01ReadRegister(STATUS) | (1<<RX_DR));	//Réinitialiser le bit RX_DR n'est pas obligatoire, mais c'est une bonne pratique.
	
	return rxDataBuffer;
}

/*********************************************************************************************************************************************************************/

uint8_t nrf24l01RxBufferSize(void)
{
	rxDataBufferSize=0;
	
	while (rxDataBuffer[rxDataBufferSize] != 0x00)	//Rend la taille du tableau, ce n'est pas la meilleure des solutions car si une donnée utile dans le tableau est égale à 0, la lecture va s'arrêter à cette donnée.
	{
		rxDataBufferSize++;
	}	
	return rxDataBufferSize;
}

/*********************************************************************************************************************************************************************/

void powerDownMode(void)
{
	CE_PIN_DOWN;
	_nrf24l01WriteRegister(CONFIG, _nrf24l01ReadRegister(CONFIG) & ~(1<<PWR_UP));	//Met PWR_UP à 0 pour passer en mode power down.
}

/*********************************************************************************************************************************************************************/

void powerUpMode(void)
{
	_nrf24l01WriteRegister(CONFIG, _nrf24l01ReadRegister(CONFIG) | (1<<PWR_UP));	//Met PWR_UP à 1 pour passer en mode power up.
}

/*********************************************************************************************************************************************************************/


