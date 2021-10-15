/*
 * FH Aachen Makerspace
 * The FH Aachen makerspace lora based dht22 solar powered sensor node 
 * Initial Version: Marcel Ochsendorf marcelochsendorf.com 15.10.2021
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


#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "DHT.h"

//--------------------- USER CONFIG ------------------------ //


#define BATT_ADC_PIN A0 //BATTERY VOLTAGE ADC PIN
#define BATT_ADC_MAX_RANGE 5.0 //ADC VALUE * (BATT_ADC_MAX_RANGE*1023) => ON 3.3V MCU SET THIS TO 3.3V

#define DHTPIN 6 //DHT SENSOR PIN   
#define DHTTYPE DHT22
//#define DHTTYPE DHT11

#define CHARHING_OK_INPUT 7 //PIN FOR GETTING CHARGING STATE FROM SOLAR CHARGER LOW IF CHARGING IS ACTIVE


//-------------------- LORA CONFIG ----------------------- //
// NwkSKey MSB
static const PROGMEM u1_t NWKSKEY[16] = {0xCC, 0x46, 0x1B, 0x83, 0xF1, 0x79, 0xAB, 0x6D, 0x73, 0x0E, 0x7D, 0xC4, 0x9F, 0x54, 0x18, 0xDA };
// AppSKey MSB
static const u1_t PROGMEM APPSKEY[16] = { 0x4B, 0x51, 0x95, 0xA5, 0x45, 0x1F, 0xA9, 0x76, 0xD9, 0x50, 0xB4, 0xDA, 0x29, 0xAF, 0x01, 0x6E };
// LoRaWAN address
static const u4_t DEVADDR = 0x260B1A42; //UNIQUE LORA DEVICE ID



//--------------------- END CONFIG ---------------------- //
DHT dht(DHTPIN, DHTTYPE); //Der Sensor wird ab jetzt mit „dth“ angesprochen


void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t payload[] = "..................."; //INCREASE FOR LARGER PAYLOADS 
static osjob_t sendjob;

const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 5,
    .dio = {2, 3, 4}, //DIO0 DIO1 DIO2 CONNECT ALL OF THEM
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
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
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
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
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}


String payload_str = "";
void create_payload_buffer(){
  float Luftfeuchtigkeit = dht.readHumidity(); //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  float Temperatur = dht.readTemperature();//die Temperatur auslesen und unter „Temperatur“ speichern
  payload_str = "";

  // ADD TEMPERATURE
  if(isnan(Temperatur)){
    payload_str.concat("null");
  }else{
   payload_str.concat(Temperatur);
  }
  payload_str += ";";

  //ADD HUMIDITY
  if(isnan(Luftfeuchtigkeit)){
    payload_str.concat("null");
  }else{
   payload_str.concat(Luftfeuchtigkeit);
  }
  payload_str += ";";

  float batt = analogRead(BATT_ADC_PIN);
  batt = batt * (BATT_ADC_MAX_RANGE/1023);
  payload_str.concat(batt);
  payload_str += ";";

  const bool ch_ok= !digitalRead(CHARHING_OK_INPUT);
  payload_str.concat(ch_ok);
  payload_str += ";";

  
   //CUT DOWN LENGTH
  int sz = payload_str.length();
  if(sizeof(payload) > payload_str.length()){
    sz = sizeof(payload);
  }
  //COPY INTO CHAR ARRAY
  payload_str.toCharArray(payload, sz);
  Serial.println(payload_str);
  Serial.println((char*)payload);


  
  }


void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {

        //CREATE PAYLOAD BUFFER
        create_payload_buffer();
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payload, sizeof(payload)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
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





    //SETUP DHT22 
    dht.begin(); //DHT11 Sensor starten
    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
