#include <heltec.h>

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "logo.h"

// OTAA
uint8_t devEui[] = {0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x23};
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// ABP para
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;
// LoraWan channelsmask, default channels 0-7
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
uint32_t license[4] = { 0xAD8973C6, 0xEE5887FB, 0x8464951F, 0x180A219F };
// LoraWan region, select in arduino IDE tools
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
// LoraWan Class, Class A and Class C are supported
DeviceClass_t loraWanClass = CLASS_A;
// the application data transmission duty cycle.  value in [ms].
uint32_t appTxDutyCycle = 15000;
//OTAA or ABP
bool overTheAirActivation = true;
//ADR enable
bool loraWanAdr = true;
// Indicates if the node is sending confirmed or unconfirmed messages
bool isTxConfirmed = true;
// Application port
uint8_t appPort = 2;

/*!
  Number of trials to transmit the frame, if the LoRaMAC layer did not
  receive an acknowledgment. The MAC performs a datarate adaptation,
  according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
  to the following table:

  Transmission nb | Data Rate
  ----------------|-----------
  1 (first)       | DR
  2               | DR
  3               | max(DR-1,0)
  4               | max(DR-1,0)
  5               | max(DR-2,0)
  6               | max(DR-2,0)
  7               | max(DR-3,0)
  8               | max(DR-3,0)

  Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
  the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

class CollarState {
  public:
    uint16_t cycleCount;
    uint16_t txCount;
    CollarState()
      : cycleCount(0), txCount(0)
      {

      }
      CollarState &operator=(const CollarState &v) {
        cycleCount = v.cycleCount;
        txCount = v.txCount;
        return *this;
      }
      bool operator==(const CollarState &v) {
        return (cycleCount == v.txCount) && (txCount == v.txCount);
      }

      bool operator!=(const CollarState &v) {
        return (cycleCount != v.cycleCount) || (txCount != v.txCount);
      }
};

CollarState state;
CollarState oldState;

SSD1306Wire dspl(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void logo(){
	dspl.clear();
	dspl.drawXbm(32, 5, logoWidth,logoHeight, (const unsigned char *) logoBits);
	dspl.display();
}

void updateState()
{
  if (state == oldState)
    return;
  dspl.clear();

  String cscnt =  "Tx: " + String(state.txCount, DEC) + " " + String(state.cycleCount, DEC);
  dspl.drawString(0, 0, cscnt);

  switch (deviceState) {
    case DEVICE_STATE_INIT:
        dspl.drawString(0, 10, "Init");
        break;
    case DEVICE_STATE_JOIN:
        dspl.drawString(0, 10, "Join");
        break;
    case DEVICE_STATE_SEND:
        dspl.drawString(0, 10, "Send");
        break;
    case DEVICE_STATE_CYCLE:
        dspl.drawString(0, 10, "Cycle");
        break;
    case DEVICE_STATE_SLEEP:
        dspl.drawString(0, 50, "Sleep");
        break;
    default:
        break;
  }
	dspl.display();
  digitalWrite(LED, (state.txCount % 2 ? HIGH : LOW));
  oldState = state;
}

void setup() {
  Serial.begin(115200);
	dspl.init();
	logo();
  delay(100);
	pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  Mcu.setlicense(license);
  Mcu.begin();
	delay(100);
	dspl.clear();
  deviceState = DEVICE_STATE_INIT;
  oldState.cycleCount = 42;
}

// Prepares the payload of the frame 
static void prepareTxFrame(
  uint8_t port
)
{
  appDataSize = 4;
  unsigned char * puc = (unsigned char *)(&state.cycleCount);
  appData[0] = puc[0];
  appData[1] = puc[1];
  puc = (unsigned char *)(&state.txCount);
  appData[2] = puc[0];
  appData[3] = puc[1];
}

void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT:
      updateState();
#if(LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass, loraWanRegion);
      updateState();
      break;
    case DEVICE_STATE_JOIN:
      updateState();
      LoRaWAN.join();
      break;
    case DEVICE_STATE_SEND:
      prepareTxFrame(appPort);
      updateState();
      LoRaWAN.send();
      state.txCount++;
      deviceState = DEVICE_STATE_CYCLE;
      updateState();
      break;
    case DEVICE_STATE_CYCLE:
      updateState();
      txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      updateState();
      break;
    case DEVICE_STATE_SLEEP:
      updateState();
      LoRaWAN.sleep(loraWanClass);
      break;
    default:
      deviceState = DEVICE_STATE_INIT;
      updateState();
      break;
  }
  state.cycleCount++;
}
