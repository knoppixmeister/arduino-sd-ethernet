#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,10);

File root;

EthernetServer server(80);

void setup()
{
   Serial.begin(9600);

   // disable w5100 SPI while setting up SD
   pinMode(53, OUTPUT);
   digitalWrite(53, HIGH);

   // set up SD
   if(SD.begin(4) == 0) 
      Serial.println("SD failed");
   else
      Serial.println("SD ok");

   // set up w5100
   Ethernet.begin(mac, ip);
   // disable w5100 SPI
   digitalWrite(10, HIGH);

   // takes a second for the w5100 to get ready
   delay(2000);

    root = SD.open("/");
  
    printDirectory(root, 0);

   // rest of your setup
   Serial.println("Setup done");
   
   server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    
    printDirectory(root, 0);
    
    boolean currentLineIsBlank = true;
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if(c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
	  client.println("Refresh: 2");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          
          client.print("DS sensor temp: ");
          //client.print(getTemp());
          client.print(" *C");
          client.println("<br/>");
          
          /*
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          
          if(!isnan(t) && !isnan(h)) {
            client.print("DHT sensor temp: ");
            client.print(t);
            client.print(" *C; ");
            client.print("Humidity: ");
            client.print(h);
            client.println("<br/>");
          }
          else {
            client.println("DHT sensor data read failed!<br/>");
          }
          */
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    client.stop();
    Serial.println("client disconnected");
  }
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     File entry = dir.openNextFile();
     if(!entry) {
       // no more files
       Serial.println("**no files**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
