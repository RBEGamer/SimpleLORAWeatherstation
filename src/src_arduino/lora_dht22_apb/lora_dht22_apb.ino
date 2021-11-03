/*
 * FH Aachen Makerspace
 * The FH Aachen makerspace lora based dht22 solar powered sensor node 
 * Initial Version: Marcel Ochsendorf 15.10.2021 https://github.com/RBEGamer/MSFHAC_Lora_DHT22_Node/
 * 
 * 
 * COMPONENTS:
 * Arduio Nano
 * DHT22
 * RFM95(W)
 * Step-Up
 * 3.7.V 1S Lipo Cell
 * Solar Charing Board
 * 
 * 
 */



#define SENSOR_TYPE_DHT22
#define SENSOR_TYPE_IKEA_VINDRIKTNING



#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "DHT.h"

//--------------------- USER CONFIG ------------------------ //


#define BATT_ADC_PIN A0 //BATTERY VOLTAGE ADC PIN
#define BATT_ADC_MAX_RANGE 5.0 //ADC VALUE * (BATT_ADC_MAX_RANGE*1023) => ON 3.3V MCU SET THIS TO 3.3V
#define CHARHING_OK_INPUT 7 //PIN FOR GETTING CHARGING STATE FROM SOLAR CHARGER LOW IF CHARGING IS ACTIVE




#ifdef SENSOR_TYPE_DHT22
#define DHTPIN 6 //DHT SENSOR PIN   
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); //Der Sensor wird ab jetzt mit „dth“ angesprochen
#endif

#ifdef SENSOR_TYPE_IKEA_VINDRIKTNING
#include "IkeaVindriktningSerialCom.h" 
#include "IkeaVindriktningTypes.h"
particleSensorState_t state;
#endif



static uint8_t payload[256] = { 0 }; //INCREASE FOR LARGER PAYLOADS 


#define RFM95_RST_PIN 5 //RESET
#define RFM95_NSS_PIN 10 //NSS / CS
#define RFM95_DIO0_PIN 2 //DIO0
#define RFM95_DIO1_PIN 3 //DIO1
#define RFM95_DIO2_PIN 4 //DIO2

//-------------------- LORA CONFIG ----------------------- //

//#ifdef SENSOR_TYPE_DHT22
//static const PROGMEM u1_t NWKSKEY[16] = {0xCC, 0x46, 0x1B, 0x83, 0xF1, 0x79, 0xAB, 0x6D, 0x73, 0x0E, 0x7D, 0xC4, 0x9F, 0x54, 0x18, 0xDA };
//static const u1_t PROGMEM APPSKEY[16] = { 0x4B, 0x51, 0x95, 0xA5, 0x45, 0x1F, 0xA9, 0x76, 0xD9, 0x50, 0xB4, 0xDA, 0x29, 0xAF, 0x01, 0x6E };
//static const u4_t DEVADDR = 0x260B1A42;
//#endif

#ifdef SENSOR_TYPE_IKEA_VINDRIKTNING
static const PROGMEM u1_t NWKSKEY[16] = {0x66, 0x1F, 0x14, 0x94, 0xC2, 0xA7, 0x7C, 0x0F, 0xE6, 0x50, 0x75, 0xDA, 0xDA, 0x73, 0x33, 0x00 };
static const u1_t PROGMEM APPSKEY[16] = { 0xB8, 0xC7, 0x43, 0x72, 0x2A, 0xD5, 0x60, 0x41, 0x1D, 0x6E, 0xEA, 0x57, 0xEA, 0x33, 0x14, 0xFB };
static const u4_t DEVADDR = 0x260B478B;
#endif



//--------------------- END CONFIG ---------------------- //

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


static osjob_t sendjob;

const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = RFM95_NSS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RFM95_RST_PIN,
    .dio = {RFM95_DIO0_PIN, RFM95_DIO0_PIN, RFM95_DIO0_PIN}, //DIO0 DIO1 DIO2 CONNECT ALL OF THEM
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
         default:
            Serial.println(F("Unknown event"));
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
    }
}


String payload_str = "";


#ifdef SENSOR_TYPE_DHT22
void prepare_payload_dht22(){
  payload_str += "f=TEMP=";
  float Luftfeuchtigkeit = dht.readHumidity(); //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  float Temperatur = dht.readTemperature();//die Temperatur auslesen und unter „Temperatur“ speichern

  // ADD TEMPERATURE
  if(isnan(Temperatur)){
    payload_str.concat("null");
  }else{
   payload_str.concat(Temperatur);
  }
  payload_str += ";f=HUM=";

  //ADD HUMIDITY
  if(isnan(Luftfeuchtigkeit)){
    payload_str.concat("null");
  }else{
   payload_str.concat(Luftfeuchtigkeit);
  }

  payload_str += ";";

  }
#endif

#ifdef SENSOR_TYPE_IKEA_VINDRIKTNING
void prepare_payload_vindriking(){
      payload_str += "f=PM25=";
      for(int i = 0; i < 1000; i++){
        IkeaVindriktningSerialCom::handleUart(state);
        delay(10);
      } 
    payload_str += String(state.lastPM25); 
    payload_str += ";";

  }
#endif




int create_payload_buffer(){
  payload_str = "";
  
  #ifdef SENSOR_TYPE_DHT22
  prepare_payload_dht22();
  #endif

  #ifdef SENSOR_TYPE_IKEA_VINDRIKTNING
  prepare_payload_vindriking();
  #endif

  payload_str += "f=BATT=";
  //ADD BATTERY AND CHARGING STATE
  float batt = analogRead(BATT_ADC_PIN);
  batt = batt * (BATT_ADC_MAX_RANGE/1023);
  payload_str.concat(batt);
  payload_str += ";i=CHG=";

  const bool ch_ok= !digitalRead(CHARHING_OK_INPUT);
  payload_str.concat(ch_ok);
  payload_str += ";";


  


    
   payload_str += "s=FW=42;";
   //CUT DOWN LENGTH
  int sz = payload_str.length();
  if(payload_str.length()> sizeof(payload)){
    sz = sizeof(payload);
  }

  for(int i = 0; i < sizeof(payload); i++){
    payload[i] = 0;  
  }
  
  //COPY INTO CHAR ARRAY
  payload_str.toCharArray(payload, sz);
  Serial.println(payload_str);
  Serial.println((char*)payload);

  
  return sz;
  
  }


void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {

        //CREATE PAYLOAD BUFFER
        int buffer_len = create_payload_buffer();

        if(buffer_len > sizeof(payload)){
          buffer_len = sizeof(payload);
        }
        
        
        LMIC_setTxData2(1, payload, buffer_len-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));

    pinMode(CHARHING_OK_INPUT, INPUT_PULLUP);
    
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    #if defined(CFG_eu868)
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
   
    #elif defined(CFG_us915)
    LMIC_selectSubBand(1);
    #endif

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);




    #ifdef SENSOR_TYPE_DHT22
    //SETUP DHT22 
    dht.begin(); //DHT11 Sensor starten
    #endif

    #ifdef SENSOR_TYPE_IKEA_VINDRIKTNING
     IkeaVindriktningSerialCom::setup();
    
      for(int i = 0; i < 10000; i++){
        IkeaVindriktningSerialCom::handleUart(state);
        delay(1);
      }
     
     
    #endif


    
    // Start job
    do_send(&sendjob);
}


unsigned long prev = 0;
unsigned long curr = 0;
const int interval = 1000*60;


void loop() {
    os_runloop_once();

    curr = millis();
    if(curr - prev >= interval){
      prev = curr;
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }

      
}
