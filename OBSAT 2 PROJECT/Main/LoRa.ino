// This module formats handles the Radio Operation.
// J. Libonatti
#include "LoRaWan_APP.h"                            //LoRaWan app header, but we will use only LoRa
#include "Arduino.h"

//Radio Parameters, search for LoRa protocol for futher reading

#define RF_FREQUENCY                      928000000 // Radio Frequency in Hz

#define TX_OUTPUT_POWER                    20        // Tx power (10 = 20dbm)

#define LORA_BANDWIDTH                     0        // [0: 125 kHz,
                                                    //  1: 250 kHz,
                                                    //  2: 500 kHz,
                                                    //  3: Reserved]
                                                    
#define LORA_SPREADING_FACTOR              12        // [SF7..SF12] the higher SF, the higher the range but also the power consumption
#define LORA_CODINGRATE                    1        // [1: 4/5,
                                                    //  2: 4/6,
                                                    //  3: 4/7,
                                                    //  4: 4/8]
#define LORA_PREAMBLE_LENGTH               8        // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                0        // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


volatile bool lora_idle=true;                                //Flag for idle radio
volatile bool ping=false;

static RadioEvents_t RadioEvents;                   //Radio Events Handle
void OnTxDone( void );                              //Called after finished transmission
void OnTxTimeout( void );                           //Called for failed transmission
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

int SetLoRa(){
  Mcu.begin();                                      // Starts radio call
  
  RadioEvents.TxDone = OnTxDone;                    // Sets Tx Done task
  RadioEvents.TxTimeout = OnTxTimeout;              // Sets Tx Timeout task
  RadioEvents.RxDone = OnRxDone;
    
  Radio.Init( &RadioEvents );                       // Starts the radio
  Radio.SetChannel( RF_FREQUENCY );                 // Selects the radio Frequency
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,                  // Configures Radio Tx
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );        
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  vTaskDelay(100);
  return 0;
}

void SendLoRa(const byte *message){  //Transmits a LoRa message
Radio.IrqProcess();                 //Processes LoRa interrupts
	if(lora_idle == true)
	{
    lora_idle = false;                                               //Marks Radio as Busy
		Radio.Send((uint8_t*) message, 32); //send the package out	
	}
}

void OnTxDone( void )
{
  Radio.Rx( 0 );
	lora_idle = true;     //Marks Radio as Free
}

void OnTxTimeout( void )  //Does the same as OnTxDone but sets an error
{
  Radio.Sleep( );
  lora_idle = true;
  txTimeout = true;
  LEDOn();
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ){
  if(size == 4)
    if(payload[0] == 'p' && payload[1] == 'i' && payload[2] == 'n' && payload[3] == 'g')
      ping = true;
}
