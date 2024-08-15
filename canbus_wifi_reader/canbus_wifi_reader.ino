//#include <Arduino_BuiltIn.h>

/*
  Arduino CANBus WiFi Reader

  Read data from ODB2 CANBus and send it to a PC via Wifi


  created 14 Aug 2024
    by Ramon Izquierdo Cordova


  Explanations about the CAN Bus
  https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial

  ODB2 Codes
  https://en.wikipedia.org/wiki/OBD-II_PIDs


*/

// CAN bus libs -----------------------
#include <mcp2515.h>


// CAN Bus vars -----------------------

#define CS_PIN    10

MCP2515 mcp2515(CS_PIN);

// program vars -----------------------

// CAN Bus data types

#define ENGINE_RPM 2


int canValueType; // CAN Value type being read and shown

bool gotReading = false; // flag for reading valid value from the CAN bus


static const unsigned long CHANGE_CAN_TYPE_INTERVAL = 10000; // ms
static unsigned long lastChangeTime = 0;

/*************************************************
   Fucntions
 *************************************************/

// CAN Bus functions -----------------------


struct can_frame setCANMsgData(void) {
  struct can_frame canMsg;
  
  switch (canValueType) {
    case ENGINE_RPM:
      canMsg.can_id  = 0x7df;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x02;
      canMsg.data[1] = 0x01;
      canMsg.data[2] = 0x0C;
      canMsg.data[3] = 0x00;
      canMsg.data[4] = 0x00;
      canMsg.data[5] = 0x00;
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00;
      break;
    default:
      Serial.println("CAN Value type not supported");
      break;
  }
  return canMsg;
}


float decodeCANData(struct can_frame canMsg) {
  float canValue = 0.0;
  
  switch (canValueType) {
    case ENGINE_RPM:
      // Engine speed (rpm) = (A * 256 + B) / 4
      canValue = (canMsg.data[3] * 256 + canMsg.data[4]) / 4;
      break;
    default:
      Serial.println("CAN Value type not supported");
      break;
  }
  return canValue;
}


float readCANValue(void) {
  
  struct can_frame canMsgSent = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent);
  //mcp2515.sendMessage(&canMsgSent);

  float canValue = 0.0;
  int loopCount = 0;
  int maxLoopIterations = 500; //Trying to read answer n times
  struct can_frame canMsgRead;
  while (mcp2515.readMessage(&canMsgRead) == MCP2515::ERROR_OK) {
    if ((canMsgRead.can_id >= 0x7e8 and canMsgRead.can_id <= 0x7ef) or loopCount>maxLoopIterations){
      break;
    }
    loopCount = loopCount + 1;
  }
  if (canMsgRead.can_id >= 0x7e8 and canMsgRead.can_id <= 0x7ef and canMsgRead.data[1]==canMsgSent.data[1]+0x40 and canMsgRead.data[2]==canMsgSent.data[2] ) {
    canValue = decodeCANData(canMsgRead);
    gotReading = true;
  }
  else {
    gotReading = false;
  }
  
  return canValue;
}


void configureCAN() {
  Serial.println("Searching MCP2515...");
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  Serial.println("MCP2515 chip found");
}

void setCANValueType(int valueType) {
  canValueType = valueType;
}



// Main fucntions -----------------------

void setup() {

  Serial.begin(115200);
  configureCAN();
  setCANValueType(ENGINE_RPM);
}


void loop() {
  float canValue;
  
  canValue = readCANValue();
  
  //gotReading = true;
  //canValue = random(0, 100);
  
  delay(1000); // Wait to data transfer from device to complete
  
  if (gotReading == true) {
    Serial.print("Value from CAN Bus:");
    Serial.println(canValue);

    delay(500);
    /*
    unsigned long currentMillis = millis();
    if(currentMillis - lastChangeTime >= CHANGE_CAN_TYPE_INTERVAL) {
      lastChangeTime = currentMillis;
      setCANValueType(nextCANType());
      redraw=true;
    }
    */
  }
}

