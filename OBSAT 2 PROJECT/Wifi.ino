// This module formats handles the ESP32 wifi module and http requisition according to OBSAT Specs.
// J. Libonatti

#include "Curie.h"
#include <WiFi.h>         // Wifi Lib
#include <HTTPClient.h>   // Http Lib

WiFiClient client;        // Client that communicates with wifi server

int SetWifi(){                          // Starts wifi and connects to OBSAT network
  btStop();                             // Stops any bluetooth so it won't trouble us ;)

  WiFi.setSleep(WIFI_PS_MAX_MODEM);     // Set modem to max energy saving
  WiFi.setTxPower(WIFI_POWER_5dBm);     // Set Tx to a lower power level as the hotspot is placed very near

  WiFi.begin(wifiSSID, wifiPassword);   // Starts wifi connecting to the given SSID with the given password
  delay(1000);                          // Waits a bit for the wifi to connect (wait for it, wait for it)
  
  if(WiFi.status() != WL_CONNECTED)     // If it failed (in it's only purpose) 
    return -71;                         // return error

  return 0;
}

void SendHttpRequest(const String query){
    if(WiFi.status()== WL_CONNECTED){           // Checks if wifi is still connected
      HTTPClient http;                          // Starts an http client
    
      http.begin(client, serverName);           // Starts connection with the http server
      int httpResponseCode = http.POST(query);  // Posts an http query to the http server and retrieves response code

      if(httpResponseCode != 200){              // checks if http was sucessful and logs if not
          LEDOn();
          httpFailed = true;
      }
  
      http.end();                               // Finishes http
    } else {                                    // checks if http was is connected and logs if not
      LEDOn();
      wifiNotConnected = true;
    }
}
