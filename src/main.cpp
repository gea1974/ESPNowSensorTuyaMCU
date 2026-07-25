#include <configuration.h>

#include <lib/logging.h>

#include <lib/EspNowSensor.h>

uint8_t   dataBatteryLevel = 0;

#if ( (defined ESP32 && defined ESP32C3) || (defined ESP32 && defined ESP32C2))
HardwareSerial SerialTuya(SERIAL_TUYA_PORT);
#endif   


//=============================Tuya
uint8_t   startupSeq[]                  = TUYA_STARTSEQ;
uint8_t   startupSeqCounter             = 255;


#define   QUERY_IDLE                    0
#define   QUERY_REQUEST                 1
#define   QUERY_SEND                    2
#define   QUERY_RECEIVED                3 

uint8_t   tuyaHeartBeatQuery            = QUERY_IDLE;
uint8_t   tuyaProductQuery              = QUERY_IDLE;
uint8_t   tuyaWorkingModeQuery          = QUERY_IDLE;
uint8_t   tuyaQueryDpStatus             = QUERY_IDLE;
uint8_t   tuyaHeartbeatRestart          = 0;

#define   NETWORKSTATE_DISCONNECTED     0
#define   NETWORKSTATE_CONFIG           1
#define   NETWORKSTATE_CONNECTING       2
#define   NETWORKSTATE_CONNECTED        3
uint8_t   tuyaNetworkState              = NETWORKSTATE_DISCONNECTED;

uint8_t   tuyaSettingsModeReply         = 0;
uint8_t   tuyaSendDataRecordReply       = 0;
uint8_t   tuyaSendDPCacheReply          = 0;

unsigned long tuyaQueryTime             = millis();
unsigned long tuyaHeartbeatTime         = 0;

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


const char sendHeartbeat[]                        = {0x00};                     //Send Heartbeat
const char queryProductInfo[]                     = {0x01};                     //Query product information
const char queryWorkingMode[]                     = {0x02};                     //Query working mode
const char queryDpStatus[]                        = {0x08};                     //Query dp status

const char reportNetworkStatusConfig[4][2]        = { {0x02, 0x00}, {0x02, 0x00}, {0x02, 0x00}, {0x03, 0x01} };               //Network Status: Configuration
const char reportNetworkStatusConnecting[4][2]    = { {0x02, 0x02}, {0x02, 0x00}, {0x02, 0x00}, {0x03, 0x02} };               //Network Status: Connecting
const char reportNetworkStatusConnected[4][2]     = { {0x02, 0x04}, {0x02, 0x00}, {0x02, 0x01}, {0x03, 0x04} };               //Network Status: Connected

const char confirmSettingMode3[]                  = {0x03};                     //Confirm Reset Wifi = enter configuration mode
const char confirmSettingMode4[]                  = {0x04};                     //Confirm Reset Wifi = enter configuration mode
const char confirmReportRecord5[]                 = {0x05, 0x00};               //Confirm Report the real-time status success
const char confirmReportRecord6[]                 = {0x06, 0x01};               //Confirm Report status (proactively)
const char confirmReportRecord8[]                 = {0x08, 0x00};               //Confirm Report the real-time status with storage function success
const char confirmReportRecord22[]                = {0x23, 0x01};               //Confirm Report status (sync)
const char confirmObtainCache[]                   = {0x10, 0x01, 0x00};         //Confirm Obtain DP cache command



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
//                      Header  Version   Sequence (Protocol Version 2)                 Command  Datalen    data      Check
  uint8_t dataLen = size -1;
  uint8_t telegramLen = 2   +   1     +   2 * (uint8_t)(TUYA_PROTOCOL_VERSION==0x02)  + 1     +  2     +    dataLen  + 1; 
//  printLogMsgTime("Tuya: Telegram: Write: Telegram length: %d Data lenght: %d\n",telegramLen, dataLen);
  char sendbuffer[telegramLen];
  uint8_t tuyaChecksum = 0;
  sendbuffer[telegramLen-1] = 0x00;
  for (int i = 0; i < (telegramLen-1); i++) {
    //Header
    if       (i==0)                                     sendbuffer[i] = TUYA_PROTOCOL_HEADER >> 8;
    else if  (i==1)                                     sendbuffer[i] = TUYA_PROTOCOL_HEADER & 0xFF;
    //Protocol version
    else if ((i==2) && (TUYA_PROTOCOL_VERSION==0x00) )  sendbuffer[i] = TUYA_PROTOCOL_VERSION;    
    else if ((i==2) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = TUYA_PROTOCOL_VERSION; 
    else if ((i==2) && (TUYA_PROTOCOL_VERSION==0x03) )  sendbuffer[i] = 0x00;   
    //Sequence (only protocol version 0x02)
    else if ((i==3) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = seq >> 8;
    else if ((i==4) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = seq & 0xFF;
    //Command (first byte of buffer)
    else if  (i==3)                                     sendbuffer[i] = buffer[0];
    else if ((i==5) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = buffer[0];
    //Data length
    else if  (i==4)                                     sendbuffer[i] = dataLen >> 8;
    else if  (i==5)                                     sendbuffer[i] = dataLen & 0xFF;
    else if ((i==5) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = dataLen >> 8;
    else if ((i==6) && (TUYA_PROTOCOL_VERSION==0x02) )  sendbuffer[i] = dataLen & 0xFF;
    //Data bytes
    else sendbuffer[i] = buffer[i-(telegramLen - size - 1)];
/*
    String tuyaTxMessage;
    char dataChar[2];
    for (int i = 0; i < telegramLen; i++) {
      sprintf (dataChar, "%02X", sendbuffer[i]);
      tuyaTxMessage += dataChar;
    }
    printLogMsgTime("Tuya: Telegram: Write: Loop: %d Message: %s\n",i,tuyaTxMessage.c_str());*/
    tuyaChecksum += sendbuffer[i];
  }
  sendbuffer[telegramLen-1] = tuyaChecksum;

  String tuyaTxMessage;
  char dataChar[2];
  for (int i = 0; i < telegramLen; i++) {
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
      else if (tuyaVersion==0x03) {
        tuyaMinMessageLen = 7;
        tuyaTelegramSequence(0,true,false);
        tuyaCommand = tuyaReceivedBytes[3];
        tuyaDataLen = (tuyaReceivedBytes[4] << 8 | tuyaReceivedBytes[5]);
        tuyaTelegramLen = 6 + tuyaDataLen + 1;   //Header + Data + Checksum
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
    else if ( (tuyaVersion!=0x00) && (tuyaVersion!=0x02) && (tuyaVersion!=0x03)) {             
      printLogMsgTime("Tuya: Message: Receive:  Protocol version (%02X): not implemented\n", tuyaVersion);
    }
    else if (tuyaVersion!=TUYA_PROTOCOL_VERSION) {             
      printLogMsgTime("Tuya: Message: Receive:  Protocol version unexpected: %02X <> %02X\n", tuyaVersion, TUYA_PROTOCOL_VERSION);
    }
    else if (!tuyaChecksumOk) {               // Message check sum wrong
      printLogMsgTime("Tuya: Message: Receive: Checksum invalid: %02X <> %02X\n", tuyaReceivedBytes[tuyaRxByteCount-1], tuyaChecksum);
    }
    else if (tuyaCommand == 0x00)             // MCU: Heartbeat Response
    {
      String tuyaHeartbeatRestartStr;
      if (tuyaReceivedBytes[6]==0x00) tuyaHeartbeatRestartStr = "Restart";
      else if (tuyaReceivedBytes[6]==0x01) tuyaHeartbeatRestartStr = "Regular";
      else 
      {
        tuyaHeartbeatRestartStr = "(";
        tuyaHeartbeatRestartStr += tuyaReceivedBytes[6];
        tuyaHeartbeatRestartStr += ")";
      }
      printLogMsgTime("Tuya: Message: Receive: Heartbeat %s\n" , tuyaHeartbeatRestartStr.c_str() );
      tuyaHeartbeatRestart = tuyaReceivedBytes[6];
      tuyaHeartBeatQuery = QUERY_RECEIVED;
    }
    else if (tuyaCommand == 0x01)                                       // MCU: system information.
    {
      String tuyaMcuVersion;
      for (int i = 0; i < tuyaDataLen; i++) {
        tuyaMcuVersion += tuyaReceivedBytes[6+i];
      }
      printLogMsgTime("Tuya: Message: Receive: Product information: %s\n" , tuyaMcuVersion.c_str() );
      tuyaProductQuery = QUERY_RECEIVED;
    }
    else if ( ( (tuyaCommand == 0x02) && (tuyaVersion==0x00) )          // MCU: network status confirmed
          ||  ( (tuyaCommand == 0x02) && (tuyaVersion==0x02) )          
          ||  ( (tuyaCommand == 0x03) && (tuyaVersion==0x03) )  )           
    {
      printLogMsgTime("Tuya: Message: Receive: Network status confirmed (%02X) (Tuya version %02X)\n",tuyaCommand ,tuyaVersion);
    }
    else if ( ( (tuyaCommand == 0x02) && (tuyaVersion==0x03) )  )       // MCU: Query working mode response
    {
      if       (tuyaDataLen==0)  printLogMsgTime("Tuya: Message: Receive: Query working mode response: MCU\n");  //The module works with the MCU to process network events
      else  if (tuyaDataLen==2)  printLogMsgTime("Tuya: Message: Receive: Query working mode response: Module (%02X %02X)\n", tuyaReceivedBytes[6],tuyaReceivedBytes[7]);
      else  if (tuyaDataLen==3)  printLogMsgTime("Tuya: Message: Receive: Query working mode response: Module (%02X %02X %02X)\n", tuyaReceivedBytes[6],tuyaReceivedBytes[7],tuyaReceivedBytes[8]);
      else                      printLogMsgTime("Tuya: Message: Receive: Query working mode response: Unknown\n");
      tuyaWorkingModeQuery = QUERY_RECEIVED;
    }
    else if ( ( (tuyaCommand == 0x03) && (tuyaVersion==0x00) )          // MCU: setting mode.
          ||  ( (tuyaCommand == 0x03) && (tuyaVersion==0x02) )
          ||  ( (tuyaCommand == 0x04) && (tuyaVersion==0x00) )
          ||  ( (tuyaCommand == 0x04) && (tuyaVersion==0x03) )  )
    {
      printLogMsgTime("Tuya: Message: Receive: Settings mode (%02X)\n" ,tuyaCommand );
      tuyaSettingsModeReply = tuyaCommand;
    }
    else if ( ( (tuyaCommand == 0x05) && (tuyaVersion==0x00) )         // MCU: Report record type status
           || ( (tuyaCommand == 0x06) && (tuyaVersion==0x02) )          
           || ( (tuyaCommand == 0x08) && (tuyaVersion==0x00) ) 
           || ( (tuyaCommand == 0x22) && (tuyaVersion==0x03) ) )     
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
      else if (tuyaCommand==0x22) dpidAdr = 6;
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

      tuyaSendDataRecordReply = tuyaCommand;
    }
    else if (tuyaCommand == 0x10)             // MCU: Obtain DP cache command
    {
      printLogMsgTime("Tuya: Message: Receive: Obtain DP cache\n" );
      tuyaSendDPCacheReply = tuyaCommand;
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



  uint8_t startupSeqCounterOld = startupSeqCounter;
  if      (startupSeqCounter==255) startupSeqCounter=0;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DONE) ) ;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DELAY) && (millis()>TUYA_SEND_STARTUP_DELAY) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_HEARTBEAT) && (tuyaHeartBeatQuery==QUERY_RECEIVED) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_PRODUCT) && (tuyaProductQuery==QUERY_RECEIVED) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_WORKINGMODE) && (tuyaWorkingModeQuery==QUERY_RECEIVED) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_NETWORKSTATE) && (tuyaNetworkState==NETWORKSTATE_CONNECTED) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DP_STATUS) && (tuyaQueryDpStatus==QUERY_SEND) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_HEARTBEAT_RESTART_DONE) && (tuyaHeartbeatRestart!=0x00) ) startupSeqCounter++;
  else if ( (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_NOP) ) startupSeqCounter++;

  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DONE) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Done\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DELAY) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Delay\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_HEARTBEAT) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Heartbeat\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_PRODUCT) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Product query\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_WORKINGMODE) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Working mode query\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_NETWORKSTATE) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Network state\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_NOP) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  No operation\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_HEARTBEAT_RESTART_DONE) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  Heartbeat restart\n",startupSeqCounter);
  if ( (startupSeqCounterOld != startupSeqCounter) && (startupSeq[startupSeqCounter]== TUYA_STARTSEQ_DP_STATUS) ) printLogMsgTime("Tuya: Startup Sequence: Counter=%d:  DP state query\n",startupSeqCounter);

  if (startupSeqCounterOld != startupSeqCounter) tuyaQueryTime = millis();
  else if (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_DONE) ;
  else if (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_HEARTBEAT_RESTART_DONE) ;
  else if (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_DELAY) ;
  else if ( (millis()-tuyaQueryTime) > TUYA_QUERY_TIMEOUT)
  {
    printLogMsgTime("Tuya: Startup Sequence: Query response not received .. Reboot.\n");
    #ifdef ESP8266
    system_restart(); 
    #endif 
    #ifdef ESP32 
    esp_restart(); 
    #endif
    delay(500);
  }



  if      (TUYA_PRODUCT_QUERY!=TUYA_QUERY_ONCE) ;                                                                                                           //TUYA: Query Product info
  else if ( (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_PRODUCT) && (tuyaProductQuery==QUERY_IDLE)  ) tuyaProductQuery = QUERY_REQUEST;                                                                                              
  else if (tuyaProductQuery==QUERY_RECEIVED) ;
  else if (tuyaProductQuery==QUERY_REQUEST)
  {
    printLogMsgTime("Tuya: Message: Send: Query Product Info\n");
    TuyaSerialWriteTelegram(queryProductInfo, sizeof(queryProductInfo),tuyaTelegramSequence(0,false,true));
    tuyaProductQuery = QUERY_SEND;
  }

  if      (TUYA_WORKINGMODE_QUERY!=TUYA_QUERY_ONCE) ;                                                                                                       //TUYA: Query working mode
  else if ( (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_WORKINGMODE) && (tuyaWorkingModeQuery==QUERY_IDLE)  ) tuyaWorkingModeQuery = QUERY_REQUEST;                                                       
  else if (tuyaWorkingModeQuery==QUERY_RECEIVED) ;
  else if (tuyaWorkingModeQuery==QUERY_REQUEST)
  {
    printLogMsgTime("Tuya: Message: Send: Query Working Mode Info\n");
    TuyaSerialWriteTelegram(queryWorkingMode, sizeof(queryWorkingMode),tuyaTelegramSequence(0,false,true));
    tuyaWorkingModeQuery = QUERY_SEND;
  }

  if      (TUYA_DP_STATUS_QUERY!=TUYA_QUERY_ONCE) ;                                                                                                       //TUYA: Query working mode
  else if ( (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_DP_STATUS) && (tuyaQueryDpStatus==QUERY_IDLE)  ) tuyaQueryDpStatus = QUERY_REQUEST;                                                       
  else if (tuyaQueryDpStatus==QUERY_RECEIVED) ;
  else if (tuyaQueryDpStatus==QUERY_SEND) ;
  else if (tuyaQueryDpStatus==QUERY_REQUEST)
  {
    printLogMsgTime("Tuya: Message: Send: Query DP Status\n");
    TuyaSerialWriteTelegram(queryDpStatus, sizeof(queryDpStatus),tuyaTelegramSequence(0,false,true));
    tuyaQueryDpStatus = QUERY_SEND;
  }

  if     ( (TUYA_HEARTBEAT!=TUYA_QUERY_ONCE) && (TUYA_HEARTBEAT!=TUYA_QUERY_CYCLIC)  ) ;                                                                    //TUYA: Send Heartbeats
  else if ( (startupSeq[startupSeqCounter]==TUYA_STARTSEQ_HEARTBEAT) && (tuyaHeartBeatQuery==QUERY_IDLE)  ) tuyaHeartBeatQuery = QUERY_REQUEST;                                           
  else if ( (TUYA_HEARTBEAT==TUYA_QUERY_CYCLIC) && (tuyaHeartBeatQuery==QUERY_RECEIVED) && (millis()-tuyaHeartbeatTime) > TUYA_HEARTBEAT_INTERVAL ) tuyaHeartBeatQuery=QUERY_REQUEST;
  else if   (tuyaHeartBeatQuery==QUERY_RECEIVED) ;
  else if   (tuyaHeartBeatQuery==QUERY_REQUEST)
  {
    printLogMsgTime("Tuya: Message: Send: Heartbeat\n");
    TuyaSerialWriteTelegram(sendHeartbeat, sizeof(sendHeartbeat),tuyaTelegramSequence(0,false,false));
    tuyaHeartBeatQuery = QUERY_SEND;
    tuyaHeartbeatTime = millis();
  }
  else if ( (tuyaHeartBeatQuery==QUERY_SEND) && ((millis()-tuyaHeartbeatTime) > TUYA_HEARTBEAT_RETRY) ) tuyaHeartBeatQuery=QUERY_REQUEST;

  if      ( (TUYA_SEND_NETWORK_STATE!=TUYA_QUERY_ONCE) && (TUYA_SEND_NETWORK_STATE!=TUYA_QUERY_ON)  ) ;                                                     //TUYA: Send networkstate
  else if ( (TUYA_SEND_NETWORK_STATE==TUYA_QUERY_ONCE) && (tuyaNetworkState==NETWORKSTATE_CONNECTED) ) ;
  else if ( (startupSeq[startupSeqCounter]!= TUYA_STARTSEQ_NETWORKSTATE) &&  (startupSeq[startupSeqCounter]!= TUYA_STARTSEQ_DONE) ) ;
  else if (!(tuyaNetworkState==NETWORKSTATE_CONFIG) && EspNowSensor.configmode)                                                                             //TUYA: Send Network status config
  {                         
    printLogMsgTime("Tuya: Message: Send: Network status Configuration\n");
    if  ( (TUYA_PROTOCOL_VERSION==0x00) || 
          (TUYA_PROTOCOL_VERSION==0x02) || 
          (TUYA_PROTOCOL_VERSION==0x03) ) 
        TuyaSerialWriteTelegram(reportNetworkStatusConfig[TUYA_PROTOCOL_VERSION], sizeof(reportNetworkStatusConfig[TUYA_PROTOCOL_VERSION]),tuyaTelegramSequence(0,false,true));
    tuyaNetworkState = NETWORKSTATE_CONFIG;
  }
  else if (!(tuyaNetworkState==NETWORKSTATE_CONNECTING) && !EspNowSensor.readyToSend && !EspNowSensor.configmode)                                           //TUYA: Send Network status not connecting
  {                         
    printLogMsgTime("Tuya: Message: Send: Network status Connecting\n");
    if  ( (TUYA_PROTOCOL_VERSION==0x00) || 
          (TUYA_PROTOCOL_VERSION==0x02) || 
          (TUYA_PROTOCOL_VERSION==0x03) ) 
        TuyaSerialWriteTelegram(reportNetworkStatusConnecting[TUYA_PROTOCOL_VERSION], sizeof(reportNetworkStatusConnecting[TUYA_PROTOCOL_VERSION]),tuyaTelegramSequence(0,false,true));
    tuyaNetworkState = NETWORKSTATE_CONNECTING;
  }
  else if (!(tuyaNetworkState==NETWORKSTATE_CONNECTED) && EspNowSensor.readyToSend && !EspNowSensor.configmode)                                             //TUYA: Send Network status connected
  {                         
    printLogMsgTime("Tuya: Message: Send: Network status Connected\n");
    if  ( (TUYA_PROTOCOL_VERSION==0x00) || 
          (TUYA_PROTOCOL_VERSION==0x02) || 
          (TUYA_PROTOCOL_VERSION==0x03) ) 
        TuyaSerialWriteTelegram(reportNetworkStatusConnected[TUYA_PROTOCOL_VERSION], sizeof(reportNetworkStatusConnected[TUYA_PROTOCOL_VERSION]),tuyaTelegramSequence(0,false,true));
    tuyaNetworkState = NETWORKSTATE_CONNECTED;
  }


  if   (tuyaSettingsModeReply!=0){                                                                                                //TUYA: Request configuration mode
    printLogMsgTime("Tuya: Message: Send: Confirm settings mode (%d)\n", tuyaSettingsModeReply);                                                                      
    if (tuyaSettingsModeReply==3) TuyaSerialWriteTelegram(confirmSettingMode3, sizeof(confirmSettingMode3),tuyaTelegramSequence(0,false,false));
    if (tuyaSettingsModeReply==4) TuyaSerialWriteTelegram(confirmSettingMode4, sizeof(confirmSettingMode4),tuyaTelegramSequence(0,false,false));

    if (!EspNowSensor.configmode) EspNowSensor.configmodeEnter();
    tuyaSettingsModeReply = 0;
  }
  if ( (tuyaSendDataRecordReply!=0) && !EspNowSensor.broadcastSending){                                                         //TUYA: Send Data record confirm
    printLogMsgTime("Tuya: Message: Send: Confirm report record (%02X)\n",tuyaSendDataRecordReply);
    if      (tuyaSendDataRecordReply==0x05)  TuyaSerialWriteTelegram(confirmReportRecord5, sizeof(confirmReportRecord5),tuyaTelegramSequence(0,false,false));
    else if (tuyaSendDataRecordReply==0x06)  TuyaSerialWriteTelegram(confirmReportRecord6, sizeof(confirmReportRecord6),tuyaTelegramSequence(0,false,false));
    else if (tuyaSendDataRecordReply==0x08)  TuyaSerialWriteTelegram(confirmReportRecord8, sizeof(confirmReportRecord8),tuyaTelegramSequence(0,false,false));
    else if (tuyaSendDataRecordReply==0x22) TuyaSerialWriteTelegram(confirmReportRecord22, sizeof(confirmReportRecord22),tuyaTelegramSequence(0,false,false));
    tuyaSendDataRecordReply = 0;
  }
  if ( (tuyaSendDPCacheReply!=0)  && (tuyaSendDataRecordReply==0) && !EspNowSensor.broadcastSending )                           //TUYA: Send DP cache confirm 
  {
    delay(50);
    if      (tuyaSendDataRecordReply=10)  printLogMsgTime("Tuya: Message: Send: Confirm Obtain DP cache command (%d)\n",tuyaSendDPCacheReply);
    TuyaSerialWriteTelegram(confirmObtainCache, sizeof(confirmObtainCache),tuyaTelegramSequence(0,false,false));
    tuyaSendDPCacheReply = 0;
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
