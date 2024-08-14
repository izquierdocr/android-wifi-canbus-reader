//#include <Arduino_BuiltIn.h>

/*
  Arduino CANBus WiFi Reader

  Read data from ODB2 CANBus and send it to a PC via Wifi


  created 14 Aug 2024
    by Ramon Izquierdo Cordova



*/

// CAN bus libs -----------------------
#include <Adafruit_MCP2515.h>


// CAN Bus vars -----------------------
#define CS_PIN    10
// Set CAN bus baud rate
#define CAN_BAUDRATE (500000)
#define MCP2515_CLOCK_FREQUENCY 8e6

Adafruit_MCP2515 mcp(CS_PIN);

// program vars -----------------------


int readAnyPacket() {
  // try to parse packet
  int packetSize = mcp.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received ");

    if (mcp.packetExtended()) {
      Serial.print("extended ");
    }

    if (mcp.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(mcp.packetId(), HEX);

    if (mcp.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(mcp.packetDlc());
    } else {
      Serial.print(" and length ");
      Serial.println(packetSize);

      // only print packet data for non-RTR packets
      while (mcp.available()) {
        Serial.print((char)mcp.read());
      }
      Serial.println();
    }

    Serial.println();
  }

  int tempCAN;
  tempCAN = random(MIN_TEMP_VALUE, MAX_TEMP_VALUE);
  return tempCAN;
}


int readRPM() {
  mcp.beginPacket(0x7df, 8);
  mcp.write(0x02);           // number of additional bytes
  mcp.write(0x01);           // mode 01 - show current data
  mcp.write(0x0c);           // engine RPM
  mcp.endPacket();
  // wait for response
  while (mcp.parsePacket() == 0 ||
         mcp.read() != 4 ||         // correct length
         mcp.read() != 0x41 ||      // correct mode
         mcp.read() != 0x0c);       // correct PID
  // engine RPM = (A * 256 + B) / 4
  float rpm = ((mcp.read() * 256.0) + mcp.read()) / 4.0;
  Serial.print("Engine RPM = ");
  Serial.println(rpm);
  // ...
  delay(1000);
}


int readTemp(void) {

  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");
/*
  mcp.beginPacket(0x7df, 8);
  mcp.write(0x02);           // number of additional bytes
  mcp.write(0x01);           // mode 01 - show current data
  mcp.write(0x05);           // engine coolant temperature
  mcp.write(0x00);           // extra bytes
  mcp.write(0x00);           // extra bytes
  mcp.write(0x00);           // extra bytes
  mcp.write(0x00);           // extra bytes
  mcp.endPacket();
*/
  Serial.print("Here... ");
  float engineTemp = 0;


  // wait for response
/*
  while (mcp.parsePacket() == 0 ||
         mcp.read() != 4 ||         // correct length
         mcp.read() != 0x41 ||      // correct mode
         mcp.read() != 0x0c);       // correct PID
  // engine RPM = (A * 256 + B) / 4
  float rpm = ((mcp.read() * 256.0) + mcp.read()) / 4.0;
*/

/*
  while (mcp.parsePacket() == 0 ||
         mcp.read() != 3 ||         // correct length
         mcp.read() != 0x41 ||      // correct mode
         mcp.read() != 0x05);       // correct PID
  // engine coolant temp = A - 40
  float engineTemp = mcp.read() - 40;
*/

/*
while (mcp.available()) {
        Serial.print((char)mcp.read());
}
*/

int packetSize = mcp.parsePacket();
Serial.print("Size ");
Serial.println(packetSize);
if (packetSize) {
    // received a packet
    Serial.print("Received ");
}
  Serial.println("done");

  delay(10);

  /*
  // send extended packet: id is 29 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending extended packet ... ");

  mcp.beginExtendedPacket(0xabcdef);
  mcp.write('w');
  mcp.write('o');
  mcp.write('r');
  mcp.write('l');
  mcp.write('d');
  mcp.endPacket();

  Serial.println("done");

  delay(1000);
  */
  
  //return (int)engineTemp;
  return 80*packetSize;
}



void configureCAN() {
  Serial.begin(9600); //(115200);
  while(!Serial) delay(10);

  Serial.println("MCP2515 Sender test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }

  mcp.setClockFrequency(MCP2515_CLOCK_FREQUENCY);
  Serial.println("MCP2515 chip found");
}



void setup() {

  Serial.println('Setting CAN board');

  configureCAN();
}


void loop() {
  int tempCAN = 0;
  //tempCAN = readTemp();

  Serial.println(tempCAN);
  showTempBar(tempCAN);

  if(tempCAN>ALARM_TRIGGER_VALUE) {
    Serial.println(F("Limit safe temperature reached. Stop engine inmediatly."));
    playAlarm();
  }
  delay(500);
}