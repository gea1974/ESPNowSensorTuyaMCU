#include <configuration.h>

#include <lib/logging.h>

#include <lib/EspNowSensor.h>

uint8_t   dataBatteryLevel = 0;

#if ( (defined ESP32 && defined ESP32C3) || (defined ESP32 && defined ESP32C2))
HardwareSerial SerialTuya(SERIAL_TUYA_PORT);
#endif   


//=============================Tuya
bool      tuyaProductRequest            = false;
bool      tuyaProductReply              = false;
bool      tuyaNetworkStateSend          = false;
bool      tuyaNetworkStateDisconnected  = false;
bool      tuyaNetworkStateConnected     = false;
bool      tuyaSendDataRecordConfirm5    = false;
bool      tuyaSendDataRecordConfirm6    = false;
bool      tuyaSendDataRecordConfirm8    = false;
bool      tuyaSendDPCacheConfirm        = false;
uint8_t   tuyaSettingsModeRequest       = 0;
unsigned long tuyaProductRequestedTime  = 0;
uint8_t   tuyaProductRequestRetry       = 0;
uint16_t  tuyaTelegramSeq               = 0;

#ifdef TUYA_MCU_WAKEUP_PIN
unsigned long tuyaMcuWakeupTime         = 0;
#endif

char tuyaReceivedBytes[128]{0};
uint8_t tuyaRxByteCount{0};
bool tuyaDataReceived=false;
/*
const char queryProductInfo[]       = {0x55, 0xAA, 0x00, 0x01, 0x00, 0x00, 0x00};                     //Query product information
const char reportNetworkStatus1[]   = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02};               //smartconfig configuration status
const char reportNetworkStatus2[]   = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x01, 0x03};               //AP configuration status 
const char reportNetworkStatus3[]   = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x02, 0x04};               //Wifi has been configured, but not connected to the router
const char reportNetworkStatus4[]   = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x03, 0x05};               //Wi-Fi has been configured, and connected to the router
const char reportNetworkStatus5[]   = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x04, 0x06};               //Wi-Fi has been connected to the router and the cloud
const char confirmSettingMode3[]    = {0x55, 0xAA, 0x00, 0x03, 0x00, 0x00, 0x02};                     //Confirm Reset Wifi = enter configuration mode
const char confirmSettingMode4[]    = {0x55, 0xAA, 0x00, 0x04, 0x00, 0x00, 0x03};                     //Confirm Reset Wifi = enter configuration mode
const char confirmReportRecord5[]   = {0x55, 0xAA, 0x00, 0x05, 0x00, 0x01, 0x00, 0x05};               //Confirm Report the real-time status success
const char confirmReportRecord8[]   = {0x55, 0xAA, 0x00, 0x08, 0x00, 0x01, 0x00, 0x08};               //Confirm Report the real-time status with storage function success
const char confirmObtainCache[]     = {0x55, 0xAA, 0x00, 0x10, 0x00, 0x02, 0x01, 0x00, 0x12};         //Confirm Obtain DP cache command
*/

const char queryProductInfo[]                     = {0x01, 0x00, 0x00};                     //Query product information
const char reportNetworkStatusWifi3[]             = {0x02, 0x00, 0x01, 0x02};               //Network Status 3: Wifi has been configured, but not connected to the router
const char reportNetworkStatusWifi5[]             = {0x02, 0x00, 0x01, 0x04};               //Network Status 5: Wi-Fi has been connected to the router and the cloud
const char reportNetworkStatusZigbeeNotPaired[]   = {0x02, 0x00, 0x01, 0x00};               //Network Status of the Zigbee module: 0x00: not paired
const char reportNetworkStatusZigbeeBeingPaired[] = {0x02, 0x00, 0x01, 0x03};               //Network Status of the Zigbee module: 0x03: being paired
const char reportNetworkStatusZigbeePaired[]      = {0x02, 0x00, 0x01, 0x01};               //Network Status of the Zigbee module: 0x01: paired
const char confirmSettingMode3[]                  = {0x03, 0x00, 0x00};                     //Confirm Reset Wifi = enter configuration mode
const char confirmSettingMode4[]                  = {0x04, 0x00, 0x00};                     //Confirm Reset Wifi = enter configuration mode
const char confirmReportRecord5[]                 = {0x05, 0x00, 0x01, 0x00};               //Confirm Report the real-time status success
const char confirmReportRecord6[]                 = {0x06, 0x00, 0x01, 0x01};               //Confirm Report status (proactively)
const char confirmReportRecord8[]                 = {0x08, 0x00, 0x01, 0x00};               //Confirm Report the real-time status with storage function success
const char confirmObtainCache[]                   = {0x10, 0x00, 0x02, 0x01, 0x00};         //Confirm Obtain DP cache command
//=============================ESP!Now
void OnDataSent(
    #ifdef ESP8266 
    uint8_t *mac, uint8_t sendStatus
    #endif
    #ifdef ESP32
    const uint8_t *mac_addr, esp_now_send_status_t status
    #endif
)
{
    #ifdef ESP32
    uint8_t sendStatus = (uint8_t)status;
    uint8_t mac[6];
    mac[0] = mac_addr[0];
    mac[1] = mac_addr[1];
    mac[2] = mac_addr[2];
    mac[3] = mac_addr[3];
    mac[4] = mac_addr[4];
    mac[5] = mac_addr[5];
    #endif

    EspNowSensor.OnDataSent(mac,sendStatus);
}
void OnDataRecv(
                #ifdef ESP8266
                uint8_t * mac, uint8_t *incomingData, uint8_t len
                #endif
                #ifdef ESP32
                const esp_now_recv_info *esp_now_info, const uint8_t *incomingData, int len
                #endif
)
{
  #ifdef ESP32
  uint8_t mac[6];
  mac[0] = esp_now_info->src_addr[0];
  mac[1] = esp_now_info->src_addr[1];
  mac[2] = esp_now_info->src_addr[2];
  mac[3] = esp_now_info->src_addr[3];
  mac[4] = esp_now_info->src_addr[4];
  mac[5] = esp_now_info->src_addr[5];
  #endif
      
  EspNowSensor.OnDataRecv(mac,incomingData,len);
}

//=============================Tuya
void TuyaSerialBegin(unsigned long baud) {
  #ifdef ESP8266
  Serial.begin(baud);
  #endif
  #if ( (defined ESP32 && defined ESP32C3) || (defined ESP32 && defined ESP32C2))
  SerialTuya.begin(baud, SERIAL_8N1, SERIAL_TUYA_RX_PIN, SERIAL_TUYA_TX_PIN);
  #endif   
}

uint16_t tuyaTelegramSequence(uint16_t seq, bool set, bool increment)
{
  if (set) tuyaTelegramSeq=seq;
  if (increment) tuyaTelegramSeq++;
  if (tuyaTelegramSeq>0xFF0) tuyaTelegramSeq = 0;
  return tuyaTelegramSeq;
}

void TuyaSerialWrite(const char *buffer, size_t size) {
  #ifdef TUYA_MCU_WAKEUP_PIN
    tuyaMcuWakeupTime = millis();
    delay(TUYA_MCU_WAKEUP_SEND_DELAY);
    digitalWrite(TUYA_MCU_WAKEUP_PIN, LOW);
  #endif

  #ifdef ESP8266
  Serial.write(buffer, size);
  Serial.flush();
  #endif
  #if ( (defined ESP32 && defined ESP32C3) || (defined ESP32 && defined ESP32C2))
  SerialTuya.write(buffer, size);
  SerialTuya.flush();
  #endif
}

void TuyaSerialWriteTelegram(const char *buffer, size_t size, uint16_t seq) {
//                      Header  Version   Sequence (Protocol Version 2)                   data      Check
  uint8_t telegramLen = 2   +   1     +   2 * (uint8_t)(TUYA_PROTOCOL_VERSION==0x02)  +   size  +   1; 
  char sendbuffer[telegramLen];
  uint8_t tuyaChecksum = 0;
  sendbuffer[telegramLen-1] = 0x00;
  for (int i = 0; i < (telegramLen-1); i++) {
    if      (i==0)                                      sendbuffer[i] = TUYA_PROTOCOL_HEADER >> 8;
    else if (i==1)                                      sendbuffer[i] = TUYA_PROTOCOL_HEADER & 0xFF;
    else if (i==2)                                      sendbuffer[i] = TUYA_PROTOCOL_VERSION;    
    else if ((i==3) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = seq >> 8;
    else if ((i==4) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = seq & 0xFF;
    else sendbuffer[i] = buffer[i-(telegramLen - size - 1)];
    tuyaChecksum += sendbuffer[i];
  }
  sendbuffer[telegramLen-1] = tuyaChecksum;

  String tuyaTxMessage;
  char dataChar[2];
  for (int i = 0; i < telegramLen-1; i++) {
    sprintf (dataChar, "%02X", sendbuffer[i]);
    tuyaTxMessage += dataChar;
  }
  printLogMsgTime("Tuya: Message: Transmit: %s\n", tuyaTxMessage.c_str());

  TuyaSerialWrite(sendbuffer, telegramLen);
}

void TuyaSerialRead() {
  #ifdef ESP8266
  while (Serial.available()) {
    tuyaReceivedBytes[tuyaRxByteCount++] = (char)Serial.read();
    delay(2);
  }
  #endif
  #if ( (defined ESP32 && defined ESP32C3) || (defined ESP32 && defined ESP32C2))
  while (SerialTuya.available()) {
    tuyaReceivedBytes[tuyaRxByteCount++] = (char)SerialTuya.read();
    delay(2);
  }
  #endif   
  tuyaDataReceived = tuyaRxByteCount>0;
}


//=============================Main
void setup() {
  EspNowSensor.begin();
  EspNowSensor.registerSendCallback(OnDataSent);
  EspNowSensor.registerRecvCallback(OnDataRecv);

  TuyaSerialBegin(TUYA_BAUD_RATE);

  #ifdef TUYA_MCU_WAKEUP_PIN
      pinMode(TUYA_MCU_WAKEUP_PIN, OUTPUT);
      digitalWrite(TUYA_MCU_WAKEUP_PIN, HIGH);
  #endif

  if (TUYA_SEND_STARTUP_DELAY>0) {
    printLogMsgTime("Info: Startup delay: %dms\n" , TUYA_SEND_STARTUP_DELAY);
  }

}

void loop() {

  EspNowSensor.configmodeHandle();
  if (!EspNowSensor.configmode) EspNowSensor.espnowAuthCheck();

  TuyaSerialRead();

  if (tuyaDataReceived)                     //TUYA: handle received message
  {
    String tuyaRxBuffer;
    String tuyaRxMessage;
    uint8_t tuyaChecksum = 0;
    char dataChar[2];
    bool tuyaChecksumOk = false;
    bool tuyaHeaderOk = false;
    uint16_t tuyaTelegramLen = 0;
    uint16_t tuyaDataLen = 0;
    uint8_t tuyaCommand = 0;
    uint8_t tuyaVersion = 0;
    uint8_t tuyaMinMessageLen = 7;

    if (tuyaRxByteCount>=7){                  // Analyze Message
      for (int i = 0; i < tuyaRxByteCount; i++) {
        sprintf (dataChar, "%02X", tuyaReceivedBytes[i]);
        tuyaRxBuffer += dataChar;
      }
      printLogMsgTime("Tuya: Message: Buffer: %s Lenght: %d\n", tuyaRxBuffer.c_str(), tuyaRxByteCount);
      tuyaHeaderOk = (tuyaReceivedBytes[0] << 8 | tuyaReceivedBytes[1]) == TUYA_PROTOCOL_HEADER;
      tuyaVersion = tuyaReceivedBytes[2];
      if (tuyaVersion==0x00) 
      {
        tuyaMinMessageLen = 7;
        tuyaTelegramSequence(0,true,false);
        tuyaCommand = tuyaReceivedBytes[3];
        tuyaDataLen = (tuyaReceivedBytes[4] << 8 | tuyaReceivedBytes[5]);
        tuyaTelegramLen = 6 + tuyaDataLen + 1;   //Header + Data + Checksum
      }
      else if (tuyaVersion==0x02) {
        tuyaMinMessageLen = 9;
        tuyaTelegramSequence((tuyaReceivedBytes[3] << 8 | tuyaReceivedBytes[4]),true,false);
        tuyaCommand = tuyaReceivedBytes[5];
        tuyaDataLen = (tuyaReceivedBytes[6] << 8 | tuyaReceivedBytes[7]);
        tuyaTelegramLen = 6 + 2 + tuyaDataLen + 1;   //Header + Sequence + Data + Checksum
      }
      else 
      {
        printLogMsgTime("Tuya: Message: Protocol version: unknown (%02X)\n", tuyaVersion); 
        tuyaMinMessageLen = 7;
        tuyaTelegramSequence(0,true,false);
        tuyaCommand = tuyaReceivedBytes[3];
        tuyaDataLen = (tuyaReceivedBytes[4] << 8 | tuyaReceivedBytes[5]);
        tuyaTelegramLen = 6 + tuyaDataLen + 1;   //Header + Data + Checksum
      }

      if (tuyaTelegramLen<=tuyaRxByteCount) {
        for (int i = 0; i < tuyaTelegramLen-1; i++) {
          sprintf (dataChar, "%02X", tuyaReceivedBytes[i]);
          tuyaRxMessage += dataChar;
          tuyaChecksum += tuyaReceivedBytes[i];
        }
        sprintf (dataChar, "%02X", tuyaReceivedBytes[tuyaRxByteCount-1]);
        tuyaRxMessage += dataChar;
        printLogMsgTime("Tuya: Message: Receive: %s\n", tuyaRxMessage.c_str());
        tuyaChecksumOk = tuyaChecksum==tuyaReceivedBytes[tuyaTelegramLen-1];
      }
      else {
        tuyaChecksumOk = false;
        printLogMsgTime("Tuya: Message: Receive: Lenght invalid %d<%d\n", tuyaRxByteCount, tuyaTelegramLen);
      }
    }
    if (tuyaRxByteCount<tuyaMinMessageLen){   // Message too short
      printLogMsgTime("Tuya: Message: Receive: Message too short\n" );
    }
    else if (!tuyaHeaderOk){                  // Message header wrong
      printLogMsgTime("Tuya: Message: Receive: Header invalid: %02X%02X\n",  tuyaReceivedBytes[0], tuyaReceivedBytes[1]);
    }
    else if ( (tuyaVersion!=0x00) && (tuyaVersion!=0x02)) {             
      printLogMsgTime("Tuya: Message: Receive:  Protocol version (%02X): not implemented\n", tuyaVersion);
    }
    else if (tuyaVersion!=TUYA_PROTOCOL_VERSION) {             
      printLogMsgTime("Tuya: Message: Receive:  Protocol version unexpected: %02X <> %02X\n", tuyaVersion, TUYA_PROTOCOL_VERSION);
    }
    else if (!tuyaChecksumOk) {               // Message check sum wrong
      printLogMsgTime("Tuya: Message: Receive: Checksum invalid: %02X <> %02X\n", tuyaReceivedBytes[tuyaRxByteCount-1], tuyaChecksum);
    }
    else if (tuyaCommand == 0x01)             // MCU: system information.
    {
      String tuyaMcuVersion;
      for (int i = 0; i < tuyaDataLen; i++) {
        tuyaMcuVersion += tuyaReceivedBytes[6+i];
      }
      printLogMsgTime("Tuya: Message: Receive: Product information: %s\n" , tuyaMcuVersion.c_str() );
      tuyaProductReply = true;
    }
    else if (tuyaCommand == 0x02)             // MCU: network status confirmed
    {
      printLogMsgTime("Tuya: Message: Receive: Network status confirmed\n" );
    }
    else if (   (tuyaCommand == 0x03)         // MCU: setting mode.
          ||  ( (tuyaCommand == 0x04) && (tuyaVersion==0x00) ) )
    {
      printLogMsgTime("Tuya: Message: Receive: Settings mode (%02X)\n" ,tuyaCommand );
      tuyaSettingsModeRequest = tuyaCommand;
    }
    else if (   (tuyaCommand == 0x05)         // MCU: Report record type status
           || ( (tuyaCommand == 0x06) && (tuyaVersion==0x02) )          
           || ( (tuyaCommand == 0x08) && (tuyaVersion==0x00) ) )     
    {
      uint8_t dpidAdr = 0;
      uint8_t dpid = 0;
      uint8_t dpidType = 0;
      uint16_t dpidLen = 0;
      uint32_t dpidValue = 0;
      bool dataTypeImplemented=true;

      printLogMsgTime("Tuya: Message: Receive: Report record type status (%d)\n", tuyaCommand);

      if (tuyaCommand==0x05) dpidAdr = 6;
      else if (tuyaCommand==0x06) dpidAdr = 6;
      else if (tuyaCommand==0x08) dpidAdr = 13;
      if (tuyaVersion==0x02) dpidAdr += 2; //+ sequenz number

      while ( (dpidAdr+3)<(tuyaTelegramLen-1) ) { //DPID + Type + Len = dpidAdr + 1 + 2
        dpid = tuyaReceivedBytes[dpidAdr];
        dpidType = tuyaReceivedBytes[dpidAdr+1];
        dpidLen =  (uint32_t) tuyaReceivedBytes[dpidAdr+2] << 8 | tuyaReceivedBytes[dpidAdr+3];
        printLogMsgTime("Tuya: Message: Receive: Data: Adr: %d: DPID: %d, Type: %d, Len: %d\n", dpidAdr, dpid, dpidType, dpidLen);

        if ( (dpidAdr+3+dpidLen)>(tuyaTelegramLen-1) ) printLogMsgTime("Tuya: Message: Receive: DPID data Lenght invalid (%d:%d<%d)\n", dpidAdr, (3+dpidLen), tuyaTelegramLen);
        else if (dpidLen==1)  dpidValue = (uint32_t) (tuyaReceivedBytes[dpidAdr+4]);
        else if (dpidLen==2)  dpidValue = (uint32_t) (tuyaReceivedBytes[dpidAdr+4] << 8 | tuyaReceivedBytes[dpidAdr+5]);
        else if (dpidLen==4)  dpidValue = (uint32_t) (tuyaReceivedBytes[dpidAdr+4] << 24 | tuyaReceivedBytes[dpidAdr+5] << 16 | tuyaReceivedBytes[dpidAdr+6] << 8 | tuyaReceivedBytes[dpidAdr+7]);
        else {
          dataTypeImplemented=false;
          printLogMsgTime("Tuya: Message: Receive: Report record type status DPID lengh (%d): not implemented\n", dpidLen);
        }

        if (dataTypeImplemented) EspNowSensor.espnowMessageDataSetProgram(0xA0);
        
        if (!dataTypeImplemented) ;
        else if (dpidType==1) EspNowSensor.espnowMessageDataAddSensorValue(dpid,dpidValue);
        else if (dpidType==2) EspNowSensor.espnowMessageDataAddSensorValue(dpid,dpidValue);
        else if (dpidType==4) EspNowSensor.espnowMessageDataAddSensorValue(dpid,dpidValue);
        else printLogMsgTime("Tuya: Message: Receive: Report record type status DPID type (%d): not implemented\n", dpidType);

        dpidAdr += 4 + dpidLen;
      }
      #ifdef ESPNOW_SEND_TUYA_TELEGRAM
        printLogMsgTime("Tuya: Message: Data: Telegram received, sending ESP!Now message.\n");
        EspNowSensor.espnowMessageDataSetProgram(0xA0);
        EspNowSensor.espnowMessageDataSend();
      #endif //ESPNOW_SEND_TUYA_TELEGRAM

      tuyaSendDataRecordConfirm5 = (tuyaCommand==0x05);
      tuyaSendDataRecordConfirm6 = (tuyaCommand==0x06);
      tuyaSendDataRecordConfirm8 = (tuyaCommand==0x08);
    }
    else if (tuyaCommand == 0x10)             // MCU: Obtain DP cache command
    {
      printLogMsgTime("Tuya: Message: Receive: Obtain DP cache\n" );
      tuyaSendDPCacheConfirm = true;
    }
    else                                      // MCU: message not implemented
    {
      printLogMsgTime("Tuya: Message: Receive: message Command not implemented (%02X)\n" , tuyaCommand );
    }

    if (tuyaRxByteCount>tuyaTelegramLen){     // another message in buffer
      for (int i = 0; i < tuyaRxByteCount-tuyaTelegramLen; i++) {
        tuyaReceivedBytes[i] = tuyaReceivedBytes[tuyaTelegramLen+i];
      }
      tuyaRxByteCount = tuyaRxByteCount - tuyaTelegramLen;
      printLogMsgTime("Tuya: Message: Buffer: %d bytes left in buffer\n", tuyaRxByteCount);
    }
    else {                                    // Buffer empty
      tuyaDataReceived = false;
      tuyaRxByteCount = 0;
      memset(&tuyaReceivedBytes, 0, 128);
    }
  }

  if (!TUYA_PRODUCT_REQUEST) ;                                                                                                  //TUYA: Request Product info
  else if (EspNowSensor.configmode) ;
  else if (tuyaProductRequest && tuyaProductReply) ;
  else if (!tuyaProductRequest && (millis()>TUYA_SEND_STARTUP_DELAY))
  {
    printLogMsgTime("Tuya: Message: Send: Query Product Info\n");
    TuyaSerialWriteTelegram(queryProductInfo, sizeof(queryProductInfo),tuyaTelegramSequence(0,false,true));
    tuyaProductRequestedTime = millis();
    tuyaProductRequest = true;
  }
  else if (tuyaProductRequest && !tuyaProductReply && ((millis()-tuyaProductRequestedTime) > TUYA_PRODUCT_REQUEST_TIMEOUT))
  {
    printLogMsgTime("Tuya: Message: Product Info not received .. Reboot.\n",tuyaProductRequestRetry);
    #ifdef ESP8266
    system_restart(); 
    #endif 
    #ifdef ESP32 
    esp_restart(); 
    #endif
    delay(500);
  }

  if (!TUYA_SEND_NETWORK_STATE) ;                                                                                               //TUYA: Send networkstate
  else if (EspNowSensor.configmode) tuyaNetworkStateConnected = false;
  else if ( TUYA_PRODUCT_REQUEST && !tuyaProductRequest ) ;
  else if ( TUYA_PRODUCT_REQUEST && !tuyaProductReply ) ;
  else if (tuyaNetworkStateConnected) ;
  else if (EspNowSensor.broadcastSending || tuyaDataReceived) ;
  else if (!tuyaNetworkStateDisconnected && !EspNowSensor.readyToSend  && (TUYA_PROTOCOL_VERSION==0x00))                        //TUYA: Send Network status not connected
  {                         
    printLogMsgTime("Tuya: Message: Send: Network status Wifi not connected (3)\n");
    TuyaSerialWriteTelegram(reportNetworkStatusWifi3, sizeof(reportNetworkStatusWifi3),tuyaTelegramSequence(0,false,true));
    tuyaNetworkStateConnected = false;
    tuyaNetworkStateDisconnected  = true;
  }
  else if (!tuyaNetworkStateConnected && EspNowSensor.readyToSend  && (TUYA_PROTOCOL_VERSION==0x00))                            //TUYA: Send Network status connected
  {                         
    printLogMsgTime("Tuya: Message: Send: Network status Wifi connected (5)\n");
    TuyaSerialWriteTelegram(reportNetworkStatusWifi5, sizeof(reportNetworkStatusWifi5),tuyaTelegramSequence(0,false,true));
    tuyaNetworkStateConnected = true;
    tuyaNetworkStateDisconnected  = false;
  }

  if (tuyaSettingsModeRequest!=0){                                                                                               //TUYA: Request configuration mode
    printLogMsgTime("Tuya: Message: Send: Confirm settings mode (%d)\n", tuyaSettingsModeRequest);                                                                      
    if (tuyaSettingsModeRequest==3) TuyaSerialWriteTelegram(confirmSettingMode3, sizeof(confirmSettingMode3),tuyaTelegramSequence(0,false,false));
    if (tuyaSettingsModeRequest==4) TuyaSerialWriteTelegram(confirmSettingMode4, sizeof(confirmSettingMode4),tuyaTelegramSequence(0,false,false));

    if (!EspNowSensor.configmode) EspNowSensor.configmodeEnter();
    tuyaSettingsModeRequest = 0;
  }

  if (tuyaSendDataRecordConfirm5 && !EspNowSensor.broadcastSending){                                                            //TUYA: Send Data record confirm (5)
    printLogMsgTime("Tuya: Message: Send: Confirm report record (5)\n");
    TuyaSerialWriteTelegram(confirmReportRecord5, sizeof(confirmReportRecord5),tuyaTelegramSequence(0,false,false));
    tuyaSendDataRecordConfirm5 = false;
  }
  if (tuyaSendDataRecordConfirm6 && !EspNowSensor.broadcastSending){                                                            //TUYA: Send Data record confirm (6)
    printLogMsgTime("Tuya: Message: Send: Confirm report record (6)\n");
    TuyaSerialWriteTelegram(confirmReportRecord6, sizeof(confirmReportRecord6),tuyaTelegramSequence(0,false,false));
    tuyaSendDataRecordConfirm6 = false;
  }
  if (tuyaSendDataRecordConfirm8 && !EspNowSensor.broadcastSending){                                                            //TUYA: Send Data record confirm (8)
    printLogMsgTime("Tuya: Message: Send: Confirm report record (8)\n");
    TuyaSerialWriteTelegram(confirmReportRecord8, sizeof(confirmReportRecord8),tuyaTelegramSequence(0,false,false));
    tuyaSendDataRecordConfirm8 = false;
  }
  if (tuyaSendDPCacheConfirm && !EspNowSensor.broadcastSending && !tuyaSendDataRecordConfirm5 && !tuyaSendDataRecordConfirm8 )  //TUYA: Send DP cache confirm (10)
  {
    delay(50);
    printLogMsgTime("Tuya: Message: Send: Confirm Obtain DP cache command\n");
    TuyaSerialWriteTelegram(confirmObtainCache, sizeof(confirmObtainCache),tuyaTelegramSequence(0,false,false));
    tuyaSendDPCacheConfirm = false;
  }

  #ifdef TUYA_MCU_WAKEUP_PIN
    if ( ((millis()-tuyaMcuWakeupTime) > TUYA_MCU_WAKEUP_PULSE_LEN) && (tuyaMcuWakeupTime>0) )
    {
      tuyaMcuWakeupTime = 0;
      digitalWrite(TUYA_MCU_WAKEUP_PIN, HIGH);
    }
  #endif

  EspNowSensor.shutDownCheck();
  printLogMsgIdle();
  delay(10);    // Add a small delay to avoid overwhelming the CPU
}
