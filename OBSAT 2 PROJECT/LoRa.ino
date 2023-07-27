// This module formats handles the Radio Operation.
// J. Libonatti
#include "LoRaWan_APP.h"                            //LoRaWan app header, but we will use only LoRa
#include "Arduino.h"

//Radio Parameters, search for LoRa protocol for futher reading

#define RF_FREQUENCY                      915000000 // Radio Frequency in Hz

#define TX_OUTPUT_POWER                    5        // Tx power in dBm (more power equals more range)

#define LORA_BANDWIDTH                     0        // [0: 125 kHz,
                                                    //  1: 250 kHz,
                                                    //  2: 500 kHz,
                                                    //  3: Reserved]
                                                    
#define LORA_SPREADING_FACTOR              7        // [SF7..SF12] the higher SF, the higher the range but also the power consumption
#define LORA_CODINGRATE                    1        // [1: 4/5,
                                                    //  2: 4/6,
                                                    //  3: 4/7,
                                                    //  4: 4/8]
#define LORA_PREAMBLE_LENGTH               8        // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                0        // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


bool lora_idle=true;                                //Flag for idle radio

static RadioEvents_t RadioEvents;                   //Radio Events Handle
void OnTxDone( void );                              //Called after finished transmission
void OnTxTimeout( void );                           //Called for failed transmission

int SetLoRa(){
  Mcu.begin();                                      // Starts radio call
  
  RadioEvents.TxDone = OnTxDone;                    // Sets Tx Done task
  RadioEvents.TxTimeout = OnTxTimeout;              // Sets Tx Timeout task
    
  Radio.Init( &RadioEvents );                       // Starts the radio
  Radio.SetChannel( RF_FREQUENCY );                 // Selects the radio Frequency
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,                  // Configures Radio Tx
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );        
  vTaskDelay(100);
  return 0;
}

void SendLoRa(const String message){  //Transmits a LoRa message
  Radio.IrqProcess();                 //Processes LoRa interrupts
	if(lora_idle == true)
	{
		Radio.Send((uint8_t*) message.c_str(), strlen(message.c_str())); //send the package out	
    lora_idle = false;                                               //Marks Radio as Busy
	}
}

void OnTxDone( void )
{
  Radio.Sleep( );       //Sleeps to save power
	lora_idle = true;     //Marks Radio as Free
}

void OnTxTimeout( void )  //Does the same as OnTxDone but sets an error
{
  Radio.Sleep( );
  lora_idle = true;
  txTimeout = true;
  LEDOn();
}
