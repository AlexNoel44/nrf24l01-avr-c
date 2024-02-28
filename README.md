# nrf24l01-avr-c
This library was made for a school project, it was made under four weeks. So, it is not perfect nor optimized as it could be. Nevertheless, it is functionnal and can send and receive data effectively. Since the time was limited, this version is not final and is subject to change. For the same reason of short time, I took inspiration from tutorials found online and the rf24 library used for the Arduino IDE.

The tutorial : https://gizmosnack.blogspot.com/2013/04/tutorial-nrf24l01-and-avr.html
rf24 library : https://github.com/nRF24/RF24

Hardware and software used 
  -Arduino Micro/AtMega 32u4 with Pickit4 debugger on Microchip Studio IDE.
  -RealTerm in combination with UART school made serial communication with USB to TTL cable on TX and RX pin of AtMega 32u4.
  -nrf24l01+

Recommendations
  Try not to supply the voltage for the nrf24l01 from the Arduino Micro board. It caused a lot of trouble for nothing for an obscure reason.
  Used external power supply like the HW-131 power supply for breadboard during tests. Maybe mine was broken but I didn't waste any time on testing 
  why.

SPI
  SPI configuration is not this critical but simple is best. SPI mode 0, 8MHz speed, master mode.
  Pins are, on the 32u4 : PB2 for MOSI, PB1 for SCK and PB0 for ChipSelect.

UART
  Low speed on the serial baud rate seems to create loss of data, I have put 115200 but didn't played too much to test what fits best.
  I used a library that we made in school which use interruptions, but I do guess that anything similar would work. So all the include and functions which contain
  the word "usart" are related to that schoolmade library and are a substitute of the Serial.print() types of functions that can be found in Arduino IDE.

CSN, CE and IRQ pins
  CSN is another way to say ChipSelect, so don't bother with that one if you already initialised it in your spi library. CE, who stand for ChipEnable can be       
  connected to any unused GPIO on the MCU. IRQ, Interrupt Request Pin must be connected to an interrupt pin like PD1(INT1) or PD0(INT0) on AtMega 32u4, but 
  the IRQ functionalities are used yet is this version of the library.

To use as a receiver (RX) 

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
  	nrf24l01Init(RX_MODE, address, 76);
   
    while (1) 
    {  
      //RX
      if(nrf24l01Available())
      {
        rxData=nrf24l01Receive();
        usartSendBytes(rxData, 12); //12 est la taille de la donnée en reception qui est la donnée de transmission, donc la taille de txData.
      }
    }
  }

To use as a transmiter (TX)

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

  	//TX
  	nrf24l01Init(TX_MODE, address, 76);//INIT TX
  					 		
    while (1) 
    {
      //TX
      nrf24l01Transmit(txData, 12); //12 est la taille de la donnée en transmission, donc la taille de txData.
    }
  }
