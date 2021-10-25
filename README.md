# MSFHAC_Lora_DHT22_Node

Einfacher Temperatur und rel. Luftfeuchtigkeits Lora-Wan Sensor basierend auf dem DHT22 und RFM95/RFM96 im zusammenspiel mit einem Arduino Nano.


## BEISPIELHAFTER AUFBAU



## SETUP

### HARDWARE SETUP

#### KOMPONENTEN

* Arduino Nano
* DHT22
* RFM95, RFM95W oder RFM95
* Steckbrett
* Jumperwires
* 

#### OPTINALE KOMPONENTEN

* 3D Gedrucktes Gehäuse - Alle Dateien befinden sich in `./src/src_stl`
* `Abzweigdose IP54 aufputz 150x110x70mm`

* `Adafruit Universal USB DC Solar Lithium Ion Polymer Lipo Charger` - Solar Support
* `10W Solarpanel 12V ` - Solar Support
* `iPo Akku Lithium-Ion Polymer Batterie 3,7V 2000mAh` - Solar Support


#### SCHALTUNG






### ChirpStack

#### Device-Profile

Create a new Device Profile and use this for each sensor of this type later on.

##### GENERAL

* `Device-profile name` => `abp_dht22_sensor_node_profile`
* `LoRaWAN MAC version` => `1.0.3` 
* `LoRaWAN Regional Parameters revision *` => `A`
* `ADR algorithm` => `Default ADR algorithm`
* `Max EIRP *` => `0` 
* `Uplink interval (seconds)` => `5` 



##### CODEC: Javascript Decode Function

```js
// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}
function Decode(fPort, bytes) {
  var result = "";
  for (var i = 0; i < bytes.length; i++) {
    result += (String.fromCharCode(bytes[i]));
  }
  
  var sp = String(result).split(';');
  return {temperature: parseFloat(sp[0]), humidity: parseFloat(sp[1]), battery: parseFloat(sp[2]),charging: parseFloat(sp[3])}
    
}
```

#### Application
