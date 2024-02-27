/**
* @file main.h
* @author anoel
* @version 0.1
* @date 2 février 2024
* @brief Code éxécutant la communication entre un ordinateur et un AtMega 32u4 puis d'un AtMega 32u4 vers un nrf24l01.
* @hardware Nordic semiconductor nrf24l01, utilisé avec ATmega 32u4 et realterm.
*/

#include <avr/io.h>
#include "usart.h"
#include "spi.h"
#include "nrf24l01.h"

uint8_t* rxData;
uint8_t txData[]={'H','e','l','l','o',' ','w','o','r','l','d','!'};
		
int main(void)
{
	uint8_t address[5]={0x12, 0x12, 0x12, 0x12, 0x12};	//Adresse de 5 octets de taille qui est la même des deux cotés

	usartInit(115200,16000000);
	
	//RX
	//nrf24l01Init(RX_MODE, address, 76);
	
	//TX
	//nrf24l01Init(TX_MODE, address, 76);//INIT TX
					 		
    while (1) 
    {
				
		//TX
		/*
		nrf24l01Transmit(txData, 12); //12 est la taille de la donnée en transmission, donc la taille de txData.
		*/
		
		//RX
		/*		
		if(nrf24l01Available())
		{
			rxData=nrf24l01Receive();
			usartSendBytes(rxData, 12); //12 est la taille de la donnée en reception qui est la donnée de transmission, donc la taille de txData.
		}
		*/
    }
}
