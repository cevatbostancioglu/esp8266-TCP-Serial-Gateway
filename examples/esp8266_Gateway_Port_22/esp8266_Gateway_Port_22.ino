/* 
  @Project Name     : WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266
  @Author           : Cevat Bostancioglu
  @Purpose          : Tcp packets to Uart packets with Wi-Fi
  @Usage            : just put ssid and password and you are ready to go.
  @History          : i just found esp8266 code github on doing some transparent operation.
                      i add some new features on code and i made our 3D printer to 3D Wifi available Printer.
*/
#include <ESP8266WiFi.h>

///////////////////// user should be EDIT

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS           1

//transparent working port on esp8266
#define TransparentPort           22

//print connected,ssid,ready etc statements. useful when it's your first deal with ESP
#define DEBUG_STATE_OVER_SERIAL   0

const char* ssid = "Your-SSID(wifiID)";
const char* password = "YourWifiPassword";

////////////////////// end of user should be EDIT

///// Dont edit the code, it just work fine.

WiFiServer server(TransparentPort);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() 
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    #if(DEBUG_STATE_OVER_SERIAL)
        Serial.print("\nConnecting to "); Serial.println(ssid);
    #endif

    while (WiFi.status() != WL_CONNECTED) delay(500);

    //start UART and the server
    //Serial.begin(115200);
    server.begin();
    server.setNoDelay(true);
    
    //Serial.print("Ready! Use 'telnet ");
    //Serial.print(WiFi.localIP());
    //Serial.println(" 23' to connect");
}

void loop() 
{
    int8_t i;
    
    //check if there are any new clients
    if (server.hasClient())
    {
        for(i = 0; i < MAX_SRV_CLIENTS; i++)
        {
            //find free/disconnected spot
            if (!serverClients[i] || !serverClients[i].connected())
            {
                if(serverClients[i]) 
                {serverClients[i].stop();}

                serverClients[i] = server.available();
                
                #if(DEBUG_STATE_OVER_SERIAL)
                  Serial.print("New client: "); Serial.print(i);
                #endif
                
                continue;
            }
        }
        
        //no free/disconnected spot so reject
        WiFiClient serverClient = server.available();
        serverClient.stop();
    }

    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
        if (serverClients[i] && serverClients[i].connected())
        {
            if(serverClients[i].available())
            {
            //get data from the telnet client and push it to the UART
            while(serverClients[i].available()) Serial.write(serverClients[i].read());
            }
        }
    }

    //check UART for data
    if(Serial.available())
    {
        size_t len = Serial.available();
        uint8_t sbuf[len];
        Serial.readBytes(sbuf, len);
        //push UART data to all connected telnet clients
      
        for(i = 0; i < MAX_SRV_CLIENTS; i++)
        {
          if (serverClients[i] && serverClients[i].connected())
          {
              serverClients[i].write(sbuf, len);
              delay(1);
          }
        }
    }
}
