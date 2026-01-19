
#define PRODUCT                             "ESP-NOW Tuya Sensor"
#define PRODUCT_FAMILY_KEY                  0x01
#define VERSION                             0x001000
#define OWNER                               "gea"

#define ESPNOW_TELEGRAM_EXTENDED
#define ESPNOW_TELEGRAM_PROGRAM             0xA0

    #ifdef D06_WINDOW_SENSOR
        //Product
        #define PRODUCT_ID                      "D06"
        #define DESCRIPTION                     "D06 Window Sensor"
        #define PRODUCT_KEY                      0x01
        //Data points
        #define DPID_STATE                      1
        #define DPID_BATTERY                    3


        //MCU specific definitions
        #if  (defined TYWE3S || defined ESP12)
            #define ACTIVE_PIN                  2
            #define ACTIVE_PIN_POLARITY         LOW
            #define SETUP_PIN                   0
            #define SETUP_PIN_POLARITY          LOW
            #define SERIAL_DEBUG_SW
            #define SERIAL_TUYA_PORT            0
        #endif
        #ifdef ESP01F_CBU
            #define ACTIVE_PIN                  2
            #define ACTIVE_PIN_POLARITY         LOW
            #define SETUP_PIN                   0
            #define SETUP_PIN_POLARITY          LOW
            #define SERIAL_DEBUG_SW 
            #define SERIAL_TUYA_PORT            0
        #endif
        #ifdef ESP32C3WROOM06
            #define ACTIVE_PIN                  5
            #define ACTIVE_PIN_POLARITY         HIGH
            #define SERIAL_TUYA_TX_PIN          18
            #define SERIAL_TUYA_RX_PIN          19
            #define SERIAL_TUYA_PORT            1 
        #endif
        #ifdef ESP32C2WROOM06
            #define ACTIVE_PIN                  5
            #define ACTIVE_PIN_POLARITY         HIGH
            #define SERIAL_TUYA_TX_PIN          18
            #define SERIAL_TUYA_RX_PIN          10
            #define SERIAL_TUYA_PORT            1 
        #endif

        #define ESPNOW_SEND_TUYA_TELEGRAM
        #define TUYA_BAUD_RATE                  9600
    #endif  

    #ifdef TH01_TEMP_HUM_SENSOR
        #define PRODUCT_ID                  "TH01"
        #define DESCRIPTION                 "TH01 Temperature & Humidity Sensor"
        #define PRODUCT_KEY                  0x02

        #define DPID_BATTERY                3
        #define DPID_VALUE1                 1
        #define DPID_VALUE2                 2  

        #define ACTIVE_PIN                  2   
        #define ACTIVE_PIN_POLARITY         LOW
        #define SERIAL_DEBUG_SW
        #define SERIAL_TUYA_PORT            0

        #define ESPNOW_SEND_DATA_COMPLETE
        #define TUYA_BAUD_RATE              9600
    #endif  


//Tuya default configuration
#ifndef TUYA_BAUD_RATE
    #define TUYA_BAUD_RATE                  9600
#endif
#ifndef TUYA_SEND_STARTUP_DELAY
    #define TUYA_SEND_STARTUP_DELAY             0
#endif
#ifndef TUYA_PRODUCT_REQUEST_TIMEOUT
    #define TUYA_PRODUCT_REQUEST_TIMEOUT        500
#endif

#include <lib/configuration_defaults.h>