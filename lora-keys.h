// OTAA
uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x23 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };
// ABP para
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr =  (uint32_t) 0x007e6ae1;
// LoraWan channelsmask, default channels 0-7
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
uint32_t license[4] = { 0xAD8973C6, 0xEE5887FB, 0x8464951F, 0x180A219F };
// LoraWan region, select in arduino IDE tools
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
// LoraWan Class, Class A and Class C are supported
DeviceClass_t loraWanClass = CLASS_A;
// the application data transmission duty cycle.  value in [ms].
uint32_t appTxDutyCycle = 1000;
// OTAA or ABP
// bool overTheAirActivation = true; // OTAA
bool overTheAirActivation = false; // OTAA
// ADR enable
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

