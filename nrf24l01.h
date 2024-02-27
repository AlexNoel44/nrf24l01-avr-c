/**
* @file nrf24l01.h
* @author anoel
* @version 0.1
* @date 2 février 2024
* @brief Cette bibliothèque offre les fonctionnalitées nécéssaire pour communiquer avec un nrf24l01.
* @hardware Nordic semiconductor nrf24l01, utilisé avec ATmega 32u4.
* @datasheet https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf
*/

#ifndef NRF24L01_H_
#define NRF24L01_H_

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "spi.h"
#include "usart.h"

#define F_CPU 16000000
#include <util/delay.h>

//Commandes de contrôle, p.48.
#define W_REGISTER			0X20
#define R_REGISTER			0X00	
#define R_RX_PAYLOAD        0X61        
#define W_TX_PAYLOAD        0XA0    
#define FLUSH_TX            0XE1              
#define FLUSH_RX            0XE2              
#define REUSE_TX_PL         0XE3              
#define R_RX_PL_WID         0X60              
#define W_ACK_PAYLOAD       0XA0              
#define W_TX_PAYLOAD_NOACK  0XB0              
#define NOP		            0XFF   

//Registres de contrôle, p.54-59.
#define CONFIG				0x00
#define EN_AA				0x01
#define EN_RXADDR			0x02
#define SETUP_AW			0x03
#define SETUP_RETR			0x04
#define RF_CH				0x05
#define RF_SETUP			0x06
#define STATUS				0x07
#define OBSERVE_TX			0x08
#define CD					0x09
#define RX_ADDR_P0			0x0A
#define RX_ADDR_P1			0x0B
#define RX_ADDR_P2			0x0C
#define RX_ADDR_P3			0x0D
#define RX_ADDR_P4			0x0E
#define RX_ADDR_P5			0x0F
#define TX_ADDR				0x10
#define RX_PW_P0			0x11
#define RX_PW_P1			0x12
#define RX_PW_P2			0x13
#define RX_PW_P3			0x14
#define RX_PW_P4			0x15
#define RX_PW_P5			0x16
#define FIFO_STATUS			0x17
#define DYNPD				0x1C

//Bits du registre CONFIG
#define MASK_RX_DR			6
#define MASK_TX_DS			5
#define MASK_MAX_RT			4
#define EN_CRC				3
#define CRCO				2
#define PWR_UP				1
#define PRIM_RX				0

//Bits du registre EN_AA
#define ENAA_P5				5
#define ENAA_P4				4
#define ENAA_P3				3
#define ENAA_P2				2
#define ENAA_P1				1
#define ENAA_P0				0

//Bits du registre EN_RXADDR	
#define ERX_P5				5
#define ERX_P4				4
#define ERX_P3				3
#define ERX_P2				2
#define ERX_P1				1
#define ERX_P0				0

//Bit du registre SETUP_AW.
#define AW					0

//Bits dU REGISTRE SETUP_RETR.
#define ARD					4 
#define ARC					0 

//Bits de définition de regsitre RF_SETUP.
#define PLL_LOCK			4
#define RF_DR				3
#define RF_PWR				1 

//Bits du registre STATUS.
#define RX_DR				6
#define TX_DS				5
#define MAX_RT				4
#define RX_P_NO				1
#define TX_FULL				0

//Bits du registre OBSERVE_TX.
#define PLOS_CNT			4 
#define ARC_CNT				0 

//Bits du registre FIFO_STATUS.
#define TX_REUSE			6
#define FIFO_FULL			5
#define TX_EMPTY			4
#define RX_FULL				1
#define RX_EMPTY			0

//Bits du registre DYNPD
#define DPL_P0				0
#define DPL_P1				1
#define DPL_P2				2
#define DPL_P3				3
#define DPL_P4				4
#define DPL_P5				5

//Macros de définition de mode.
#define TX_MODE				0
#define RX_MODE				1

//Macros d'opération de la pin CE
#define CE_PIN_INIT			DDRD |= (1<<1)
#define CE_PIN_UP			PORTD |= (1<<1)
#define CE_PIN_DOWN			PORTD &= ~(1<<1) 
#define CE_TOGGLE			PORTD= PORTD &= ~(1<<1) | (1<<1)

/*
* @brief Initialisation nrf24l01, l'adresse et le canal doivent être les mêmes des deux cotés (rx/tx) pour que la communication fonctionne.
* @param uint8_t mode : mode d'opération du nrf24l01, uint8_t* address : adresse du module, uint8_t channel : canal utilisé par le module pour la communication.
* @return void
*/
void nrf24l01Init(uint8_t mode, uint8_t* address, uint8_t channel);

/*
* @brief Retourne l'état de la radio, si elle est disponible ou pas en réception (RX).
* @param void
* @return return 1 : si la radio est disponible, return 0 : si la radio est indisponible.
*/
uint8_t nrf24l01Available();

/*
* @brief Transmission des données avec le nrf24l01, uniquement en mode tx.
* @param uint8_t *payload : tableau de données à envoyer, uint8_t size : taille du tableau de donnée.
* @return void
*/
void nrf24l01Transmit(uint8_t *payload, uint8_t size);

/*
* @brief Reception et retour des données contenues dans la FIFO rx, uniquement en mode rx.
* @param void.
* @return uint8_t* rxDataBuffer : données disponibles dans le FIFO rx du nrf24l01.
*/
uint8_t* nrf24l01Receive();
//void nrf24l01Receive(uint8_t* rxData, uint8_t size);

/*
* @brief Renvoie la taille du tableau en reception.
* @param void.
* @return uint8_t rxDataBufferSize.
*/
uint8_t nrf24l01RxBufferSize(void);

/*
* @brief Place le module en mode power down en mettant le bit PWR_UP du registre CONFIG à 0. Cette action place le nrf24l01 en veille, c'est dans ce mode qu'il consomme le moins d'énergie.
* @param void.
* @return void.
*/
void powerDownMode(void);

/*
* @brief Place le module en mode power up en mettant le bit PWR_UP du registre CONFIG à 1. Cette action place le nrf24l01 en mode d'opération normale.
* @param void.
* @return void.
*/
void powerUpMode(void);

#endif /* NRF24L01_H_ */