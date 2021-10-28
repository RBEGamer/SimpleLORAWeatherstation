# MSFHAC_Lora_DHT22_Node

Einfacher Temperatur und rel. Luftfeuchtigkeits Lora-Wan Sensor basierend auf dem DHT22 und RFM95/RFM96 im zusammenspiel mit einem Arduino Nano.


## BEISPIELHAFTER AUFBAU

### FINALER SENSOR

![FINALER_SENSOR](documenation/images/finaler_sensor_beispiel.jpg)


### STECKBRETT MIT ADAPTERPLATINE

![PCB](documenation/images/finaler_sensor_pcb.jpg)




## SETUP

### HARDWARE SETUP

#### KOMPONENTEN

* Arduino Nano
* DHT22
* RFM95, RFM95W oder RFM95
* Steckbrett
* Jumperwires


#### OPTINALE KOMPONENTEN

* 3D Gedrucktes Gehäuse - Alle Dateien befinden sich in `src/src_stl`
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
* Eine Adapter-Platine verwenden - `src/src_rfmbreadboardadapter` 


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
Somit wird der 10K Widerstand zwischen DHT Pin 1 und DHT Pin 2 eingebaut.

| Arduino Nano 	| DHT22        	|
|--------------	|--------------	|
| GND          	| Pin 4 (GND)  	|
| 3V3          	| Pin 1 (VCC)  	|
| D6           	| Pin 2 (Data) 	|


![STECKBRETT_SCHALTUNG](src/src_schematic/sensor_fritzing/lora_sensor_schem_Steckplatine.png)











### SOFTWARE SETUP ARDUINO

Der Arduino Nano wird mit der Aruino IDE programmiert. Fuer die verwendeten Bauteile müssen jedoch noch bereits fertige Bibliotheken installiert werden, welche die einfache Ansteuerung des RFM95 Moduls, sowie des DHT22 ermöglichen. Auch das LoraWan-Protokoll welches hier verwendet wird, gibt es bereits in einer entsprechenden Bibliothek. Die folgenden Bibliotheken müssen installiert werden. Da kann entwieder durch den Arduino-Library Manager geschehen oder durch das kopieren der entsprechenden Dateien aus dem `src_arduino/required_libraries` in das `Dokumente/Arduino/library`Verzeichnis des PCs.


| Arduino Bibliothek Name 	|
|-------------------------	|
| Adafruit_Unified_Sensor 	|
| DHT_sensor_library      	|
| IBM_LMIC_framework      	|

Alle anderen Softwarekomponenten bringt die Arduino IDE schon mit der Installation mit, so z.B. auch fuer die SPI Kommunikation.

Nach der Installation oder des Kopierens der Bibliotheken, kann der Quellcode für diesen Sensor geöffnet werden.
Dieser befindet sich im Ordner `src_arduino/lora_dht22_apb/lora_dht22_apb.ino`.
Hier wurden schon die notwendigen Einstellungen für die oben genannten Pinbelegungen gesetzt.

Hier sind auch bereits die Einstellungen für die Lora Kommunikation gesetzt. Jeder Sensor besitzt seine eigene feste ID, mit der er sich im LoraWan-Netzwerk identifizieren kann. Diese muss zuerst im folgenden Abschnitt erstellt werden, bevor der Code auf den Arduino aufgespielt werden kann.







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
