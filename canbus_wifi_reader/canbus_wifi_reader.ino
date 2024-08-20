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

struct can_frame canMsgPrev1;
struct can_frame canMsgPrev2;
struct can_frame canMsgPrev3;
struct can_frame canMsgPrev4;
struct can_frame canMsgPrev5;


// CAN Bus data types

#define ENGINE_RPM 2
#define FUELP_PUMP_ON 10
#define FUELP_PUMP_ON_1 11
#define FUELP_PUMP_ON_2 12
#define FUELP_PUMP_ON_3 13
#define FUELP_PUMP_ON_4 14
#define FUELP_PUMP_ON_5 15



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
    case FUELP_PUMP_ON_1:
      canMsg.can_id  = 0x7eb;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x10;
      canMsg.data[1] = 0x0A;
      canMsg.data[2] = 0x67;
      canMsg.data[3] = 0x37;
      canMsg.data[4] = 0x00;
      canMsg.data[5] = 0x00;
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00;
      break;
      case FUELP_PUMP_ON_2:
      canMsg.can_id  = 0x7eb;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x21;
      canMsg.data[1] = 0x00;
      canMsg.data[2] = 0x00;
      canMsg.data[3] = 0x00;
      canMsg.data[4] = 0x00;
      canMsg.data[5] = 0xAA;
      canMsg.data[6] = 0xAA;
      canMsg.data[7] = 0xAA;
      break;
      case FUELP_PUMP_ON_3:
      canMsg.can_id  = 0x7eb;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x03;
      canMsg.data[1] = 0x7F;
      canMsg.data[2] = 0x27;
      canMsg.data[3] = 0x24;
      canMsg.data[4] = 0xAA;
      canMsg.data[5] = 0xAA;
      canMsg.data[6] = 0xAA;
      canMsg.data[7] = 0xAA;
      break;
      case FUELP_PUMP_ON_4:
      canMsg.can_id  = 0x7eb;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x05;
      canMsg.data[1] = 0x6F;
      canMsg.data[2] = 0xDB;
      canMsg.data[3] = 0x06;
      canMsg.data[4] = 0x03;
      canMsg.data[5] = 0x28;
      canMsg.data[6] = 0xAA;
      canMsg.data[7] = 0xAA;
      break;
      case FUELP_PUMP_ON_5:
      canMsg.can_id  = 0x7eb;
      canMsg.can_dlc = 8;
      canMsg.data[0] = 0x04;
      canMsg.data[1] = 0x62;
      canMsg.data[2] = 0x07;
      canMsg.data[3] = 0x27;
      canMsg.data[4] = 0x08;
      canMsg.data[5] = 0xAA;
      canMsg.data[6] = 0xAA;
      canMsg.data[7] = 0xAA;
      break;
    default:
      Serial.println("CAN Value type not supported");
      break;
  }
  return canMsg;
}



float printFormatedHEX(int n, int d) {
  if (d==2 and n<16) Serial.print("0");
  if (d==3 and n<0x10) Serial.print("00");
  if (d==3 and n<0x100 and n>=0x10) Serial.print("0");
  //Serial.print(n < 16 ? "0" : ""); // <10 Hex , add "0" to align numbers
  Serial.print(n, HEX);
}



void printCANData(struct can_frame canMsg) {
 
  printFormatedHEX(canMsg.can_id,3);
  Serial.print(" ");
  printFormatedHEX(canMsg.can_dlc,2);
  Serial.print(" ");
  for (int i=0; i<canMsg.can_dlc; i++) {
    printFormatedHEX(canMsg.data[i],2);
  }
  for (int i=canMsg.can_dlc; i<=8; i++) {
    Serial.print("  ");
  }
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


void turnFuelPumpOn(void) {

  struct can_frame canMsgSent;

  Serial.println("Turning fuel pump ON...");

  setCANValueType(FUELP_PUMP_ON_1);
  struct can_frame canMsgSent1 = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent1);

  setCANValueType(FUELP_PUMP_ON_2);
  struct can_frame canMsgSent2 = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent2);

  setCANValueType(FUELP_PUMP_ON_3);
  struct can_frame canMsgSent3 = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent3);

  setCANValueType(FUELP_PUMP_ON_4);
  struct can_frame canMsgSent4 = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent4);

  setCANValueType(FUELP_PUMP_ON_5);
  struct can_frame canMsgSent5 = setCANMsgData();;
  mcp2515.sendMessage(&canMsgSent5);

  
}



void sniffCAN(void) {
  
  struct can_frame canMsgRead;
  if (mcp2515.readMessage(&canMsgRead) == MCP2515::ERROR_OK) {

    // Show messages from this range
    gotReading=true;
    //if ( (canMsgRead.can_id >= 0x7e8 and canMsgRead.can_id <= 0x7ef) or
    //     (canMsgRead.can_id >= 0x7d8 and canMsgRead.can_id <= 0x7df)
    //   ) gotReading=true;
    if (canMsgRead.can_id==0x7eb) {
      //Serial.print("*");
      gotReading=true;
    }
  


    if (gotReading) {
      
      /*
      printCANData(canMsgPrev1);
      Serial.print("  |  ");
      printCANData(canMsgPrev2);
      Serial.print("  |  ");
      printCANData(canMsgPrev3);
      Serial.print("  |  ");
      printCANData(canMsgPrev4);
      Serial.print("  |  ");
      printCANData(canMsgPrev5);
      Serial.print("  |  ");
      */

      printCANData(canMsgRead);
      
      if (canMsgRead.can_id >= 0x7e8 and canMsgRead.can_id <= 0x7ef) {
        Serial.println("  --Answer");
      }
      else if (canMsgRead.can_id >= 0x7d8 and canMsgRead.can_id <= 0x7df) {
        Serial.println("  --Command");
      }
      else {
        Serial.println("");
      }

      if (canMsgRead.can_id >= 0x7e8 and canMsgRead.data[0] == 0x10) {
        Serial.println("  ***********************************");
      }


    }


    canMsgPrev5 = canMsgPrev4;
    canMsgPrev4 = canMsgPrev3;
    canMsgPrev3 = canMsgPrev2;
    canMsgPrev2 = canMsgPrev1;
    canMsgPrev1 = canMsgRead;
    
  } 
  
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
  //Serial.begin(500000);
  configureCAN();
  setCANValueType(ENGINE_RPM);
  
}


void loop() {

  sniffCAN();

  unsigned long currentMillis = millis();
  if(currentMillis - lastChangeTime >= CHANGE_CAN_TYPE_INTERVAL) {
    lastChangeTime = currentMillis;
    //turnFuelPumpOn();
  }


  /*
  float canValue;
  Serial.println("aw");
  canValue = readCANValue();
  
  //gotReading = true;
  //canValue = random(0, 100);
  
  delay(1000); // Wait to data transfer from device to complete
  
  if (gotReading == true) {
    Serial.print("Value from CAN Bus:");
    Serial.println(canValue);

    delay(500);
    
    unsigned long currentMillis = millis();
    if(currentMillis - lastChangeTime >= CHANGE_CAN_TYPE_INTERVAL) {
      lastChangeTime = currentMillis;
      setCANValueType(nextCANType());
      redraw=true;
    }
    
  }
  */
}

