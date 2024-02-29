#include <heltec.h>

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "logo.h"

#include "lora-keys.h"
#include "hex-string.h"

SSD1306Wire dspl(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

class CollarState {
  public:
    enum eDeviceState_LoraWan state;
    uint16_t txCount;
    CollarState()
      : state(DEVICE_STATE_INIT), txCount(0)
      {

      }
      CollarState &operator=(const CollarState &v) {
        txCount = v.txCount;
        state = v.state;
        return *this;
      }
      bool operator==(const CollarState &v) {
        return (state == v.state) && (txCount == v.txCount);
      }

      bool operator!=(const CollarState &v) {
        return (state != v.state) || (txCount != v.txCount);
      }
};

CollarState state;
CollarState oldState;

void logo(){
	dspl.clear();
	dspl.drawXbm(32, 5, logoWidth,logoHeight, (const unsigned char *) logoBits);
	dspl.display();
}

void updateState()
{
  state.state = deviceState;
  if (state == oldState)
    return;

  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);

  dspl.clear();

  String s;
  if (overTheAirActivation) {
    s = "OTAA "; // + hexString(appKey, 16);
  } else {
    s =  "ABP "; // + hexString(appSKey, 16) + " N: " + hexString(nwkSKey, 16);
  }
  s += String(devAddr, HEX); // + hexString(devEui, 8); // + " A: " + hexString(appEui, 8);
  
  dspl.drawString(0, 0, s); 

  s =  "Tx: " + String(state.txCount, DEC);
  dspl.drawString(0, 10, s);

  switch (deviceState) {
    case DEVICE_STATE_INIT:
        dspl.drawString(0, 50, "Init");
        break;
    case DEVICE_STATE_JOIN:
        dspl.drawString(0, 50, "Join");
        break;
    case DEVICE_STATE_SEND:
        dspl.drawString(0, 50, "Send");
        break;
    case DEVICE_STATE_CYCLE:
        dspl.drawString(0, 50, "Cycle");
        break;
    case DEVICE_STATE_SLEEP:
        dspl.drawString(0, 50, "Sleep");
        break;
    default:
        break;
  }
	dspl.display();
  oldState = state;
}

void setup() {
  Serial.begin(115200);
	pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
	dspl.init();

	logo();
  delay(100);
  Mcu.setlicense(license);
  Mcu.begin();
  delay(100);
	dspl.clear();
  deviceState = DEVICE_STATE_INIT;
}

// Prepares the payload of the frame 
static void prepareTxFrame(
  uint8_t port
)
{
  appDataSize = 4;
  appData[0] = 0xAA;
  appData[1] = 0x11;
  auto puc = (unsigned char *)(&state.txCount);
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
      txDutyCycleTime = appTxDutyCycle;// appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
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
}
