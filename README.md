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


#### OPTINALE KOMPONENTEN

* 3D Gedrucktes Gehäuse - Alle Dateien befinden sich in `./src/src_stl`
* `Abzweigdose IP54 aufputz 150x110x70mm`

* `Adafruit Universal USB DC Solar Lithium Ion Polymer Lipo Charger` - Solar Support
* `10W Solarpanel 12V ` - Solar Support
* `iPo Akku Lithium-Ion Polymer Batterie 3,7V 2000mAh` - Solar Support

#### TOOLS

* Loetkolben
* Loetzinn

#### SCHALTUNG

Der Aufbau der Schaltung ist relative einfach. Jedoch kann das `RFM95` Modul nicht direkt auf das Steckbrett gesteckt werden, da dies ein anderes Rastermass von 2.0mm verwendet und das Steckbrett 2.54mm. Somit gibt es hier zwei Möglichkeiten:

* Jumperwires direkt an das `RFM95` Modul anloeten.
* Eine Adapter-Platine verwenden - `./src/src_rfmbreadboardadapter` 


Der Arduino Nano und das RFM95 Module kommunizieren ueber den SPI Bus miteinander. Zusaetzlich verwendet das RFM95 Modul 4 weitere GPIO Pins des Arduino, um mit diesen weitere Statusinformationen auszutauschen. Hierzu gehoeren unter anderem ein RESET Signal und ein Empfangs-Event-Signal.

Der SPI Buse besteht in dieser Konfiguration aus den Folgenden Signalen:

* SCK - Takt
* MOSI - Master Out Slave In 
* MISO - Master In Slave Out
* CS - Chip Select

Somit wird das RFM95 Modul folgendermassen mit dem Arduino Nano verbunden:

| Arduino Nano  	| RFM95      	|
|---------------	|------------	|
| GND           	| GND        	|
| 3V3           	| 3.3V / VCC 	|
| D13           	| SCK        	|
| D12           	| MISO       	|
| D11           	| MOSI       	|
| D10           	| NSS        	|
| D5            	| RST        	|
| D2            	| DIO0       	|
| D3            	| DIO1       	|
| D4            	| DIO4       	|


**Bei moeglichen Problemen bitte alle! GND Anschluessen des RFM95 Moduls mit dem entsprechenden GND Pins des Arduinos verbinden!**


Nachdem das RFM Modul verbunden wurde, kann anschliessend der DHT22 Sensor verbunden werden.
Dieser stellt jedoch nur ein Beipiel dar und es können später auch andere Sensoren verwendet werden.
Er verwendet zur Kommunikation eines Bus-Protokoll welches über einen GPIO die Sensordaten an den Arduino schicken kann.
Somit wird nur ein Pin am Arduino belegt.
Der Bus wir in einer Pull-Up Konfiguration betrieben, somit muss ein zusaetzlicher 10k Widerstand gegen 3.3V und dem Daten-Pin vorhanden sein.






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
