# MSFHAC_Lora_DHT22_Node

Einfacher Temperatur und rel. Luftfeuchtigkeits Lora-Wan Sensor basierend auf dem DHT22 und RFM95/RFM96 im Zusammenspiel mit einem Arduino Nano.

Die besonderheit an dieser Variante, ist die Verwendung des Lora Funkstandards im Zusammenspiel mit dem Lora-Wan Protokoll. 
Dies ermöglicht den Empfang der Sensordaten über viele Kilometer hinweg.
Um eine Übertragung der Sensordaten in das Internet zu ermöglichen, wir ein dezentrales Netzwerk aus Empfangsstationen (Gateways) verwendet.
Diese werden in der Regel von Privatpersonen oder Firmen bereitgestellt.

Ein weiterer Vorteil dieser Architektur ist, dass nicht jeder eine Gateway benötigt, welcher Daten mit seinem Lora-Wan Device senden möchte.
Natürlich nur solange, wie ein Gateway in Reichweite ist. Dafür gibt es fertige Karten, welche die Abdeckung an einem Ort darstellen.

Diese Gateways können ihre Daten an einen Server im Internet senden, welcher diese dann als z.B. REST-Api bereitstellt.
In diesem Beispiel verwenden wir das [The Things Network (TTN)](https://www.thethingsnetwork.org).







## BEISPIELHAFTER AUFBAU DES SENSORS

### FINALER SENSOR

![FINALER_SENSOR](documenation/images/finaler_sensor_beispiel.jpg)


### STECKBRETT MIT ADAPTERPLATINE

![PCB](documenation/images/finaler_sensor_pcb.jpg)




## SETUP

### HARDWARE SETUP

#### KOMPONENTEN

* Arduino Nano
* DHT22
* RFM95, RFM95W oder RFM96
* Steckbrett
* Jumperwires


#### OPTINALE KOMPONENTEN

* 3D-Druck Gehäuse - Alle Dateien befinden sich in `src/src_stl`
* `Abzweigdose IP54 aufputz 150x110x70mm`

* `Adafruit Universal USB DC Solar Lithium Ion Polymer Lipo Charger` - Solar Support
* `10W Solarpanel 12V ` - Solar Support
* `iPo Akku Lithium-Ion Polymer Batterie 3,7V 2000mAh` - Solar Support

#### TOOLS

* Lötkolben
* Lötzinn

#### SCHALTUNG

Der Aufbau der Schaltung ist relative einfach. Jedoch kann das `RFM95` Modul nicht direkt auf das Steckbrett gesteckt werden, da dies ein anderes Rastermaß von 2.0mm verwendet und das Steckbrett 2.54mm. Somit gibt es hier zwei Möglichkeiten:

* Jumperwires direkt an das `RFM95` Modul anlöten.
* Eine Adapter-Platine verwenden - `src/src_rfmbreadboardadapter` 


Der Arduino Nano und das RFM95 Module kommunizieren über den SPI Bus miteinander. Zusaetzlich verwendet das RFM95 Modul vier weitere GPIO Pins des Arduino, um mit diesen weitere Statusinformationen auszutauschen. Hierzu gehören unter anderem ein `RESET` Signal und ein Empfangs-Event-Signal.

Der SPI Bus besteht in dieser Konfiguration aus den folgenden Signalen:

* `SCK` - Clock
* `MOSI` - Master Out Slave In 
* `MISO` - Master In Slave Out
* `CS` - Chip Select

Somit wird das RFM95 Modul folgendermaßen mit dem Arduino Nano verbunden:

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



Nachdem das RFM Modul verbunden wurde, kann anschliessend der DHT22 Sensor verbunden werden.
Dieser stellt jedoch nur ein Beipiel dar und es können später auch andere Sensoren verwendet werden.
Er verwendet zur Kommunikation ein Bus-Protokoll welches über einen GPIO die Sensordaten an den Arduino schickt.
Somit wird nur ein Pin am Arduino belegt.
Der Bus wir in einer Pull-Up Konfiguration betrieben, somit muss ein zusätzlicher 10k Widerstand gegen 3.3V und dem Daten-Pin angeschlossen werden.
Somit wird der 10K Widerstand zwischen DHT Pin 1 und DHT Pin 2 eingebaut.

| Arduino Nano 	| DHT22        	|
|--------------	|--------------	|
| GND          	| Pin 4 (GND)  	|
| 3V3          	| Pin 1 (VCC)  	|
| D6           	| Pin 2 (Data) 	|

Die folgende Grafik zeigt einen exemplarischen Aufbau der Schaltung.

![STECKBRETT_SCHALTUNG](src/src_schematic/sensor_fritzing/lora_sensor_schem_Steckplatine.png)







### The Things Network

Um unseren Sensor im späteren Verlauf programmieren können, muss dieser zuerst getauft werden. Jedes Lora-Wan Gerät bekommt eine feste ID von dem zu verwendenen Netzwerk. In diesem Fall ist es das TTN und dort muss der Sensor registriert werden. Nach dieser bekommen wir zwei Schlüssel, welche für eine verschlüsselte Verbindung zwischen benötigt werden. Diese werden dann in das spätere Arduino Programm eingefügt.



### Application Registrieren

Nach der Registrierung eines Accounts, bietet das TTN in der TTN Console zwei möglichkeiten:

* Application
* Gateway

Das TTN gruppiert mehrere Lora-Devices in Applications, daher muss zuerst eine neue Application mit einem beliebigen Namen erstellt werden.


![Register Application](documenation/images/ttn_a.png)


![Application](documenation/images/ttn_b.png)

Nach der Erstellung kann über den **+ Add Enddevice** Button ein Lora-Wan-Device hinzugefügt werden.
Hier müssen einige Informationen angegeben werden. Da dies ein selbstbau Sensor ist, gibt es für diesen noch kein fertiges Template, welches verwendet werden kann.

Hier muss in den Reiter "Manually" gewechselt werden und die folgenden Daten eingegeben werden:

| Eintrag 	| Wert 	|
|-------------------------	|-------------------------	                  |
| Frequency plan	          | Europe 863-870MHz SF9 for RX2 - recommended |
| LoRaWAN Version      	    | MAC V1.0.3                                  |
| Activation mode      	    | Activation by personalisation (ABP)         |

Anschliessend auf die drei "Generate" Buttons klicken und final "Register Device".
Damit ist die Registrierung des Sensors abgeschlossen.


![End Device Register](documenation/images/ttn_c.png)


Auf der Übersicht, stehen nun einige Informationen. Hier sind drei Angaben zur Parametrisierung wichtig.
Diese können in verschiedenen Formaten angezeigt werden. Dazu auf das Auge klicken um den Key anzuzeigen und den "<>" Button um die Darstellung zu ändern.
In der Folgenden Teablle sind die benötigten Informationen aufgeführt.

| Information               | Darstellung              | Beispiel                 |
|-------------------------	|------------------------- |------------------------- |
| Device Adress	            |   HEX, MSB ohne 0x       | 260BA509                 |
| NwkSKey                   |   HEX, MSB               | 0xE4, 0x14, 0xC5, 0x65, 0x24, 0x2D, 0x7B, 0x60, 0x61, 0x43, 0xD3, 0xC1, 0x49, 0x87, 0x90, 0x45 |
| AppSKeys                  |   HEX, MSB               | 0x0D, 0x79, 0xB8, 0x87, 0x1D, 0xD4, 0x64, 0xFA, 0xCC, 0x5B, 0x3B, 0xE0, 0x08, 0x0E, 0x7A, 0x22 |



Der letzte Schritt ist hier nur bei der Entwicklung wichtig.
Das Netzwerk und der Sensor merken sich wie viele Pakete sie bereits ausgetauscht haben.
Wenn der Sensor neu Programmiert wird dieser Zähler nicht im EEProm gespeichert ist, unterscheiden sich diese und das Netzwerk nimmt keine Pakete des Sensors mehr an. Deswegen wird zu Testzwecken diese Überprüfung abgeschaltet.


![End Device Register](documenation/images/ttn_d.png)

Dazu auf der Sensor-Seite unter `General Settings -> Network Layer -> Advanced MAC settings -> Resets FRame Counter` auf `Enabled` setzten.


![End Device Register](documenation/images/ttn_e.png)



### SOFTWARE SETUP ARDUINO

Der Arduino Nano wird mit der Aruino IDE programmiert. Für die verwendeten Komponenten müssen jedoch noch bereits fertige Bibliotheken installiert werden, welche die einfache Ansteuerung des RFM95 Moduls, sowie des DHT22 ermöglichen.

Auch das LoraWan-Protokoll welches hier verwendet wird, gibt es bereits in einer entsprechenden Bibliothek.

Dies kann entwieder durch den Arduino-Library Manager geschehen oder durch das kopieren der entsprechenden Ordner aus dem `src_arduino/required_libraries` Verzeichnis in den `Dokumente/Arduino/library` Ordner des PCs.

Die folgenden Bibliotheken müssen somit installiert werden:

| Arduino Bibliothek Name 	|
|-------------------------	|
| Adafruit_Unified_Sensor 	|
| DHT_sensor_library      	|
| IBM_LMIC_framework      	|

Alle anderen Softwarekomponenten bringt die Arduino IDE schon mit der Installation mit, so z.B. auch für die SPI Kommunikation.

Nach der Installation der Bibliotheken, kann der Quellcode für den Sensor geöffnet werden.

Dieser befindet sich im Ordner `src_arduino/lora_dht22_apb/lora_dht22_apb.ino`.
Hier wurden schon die notwendigen Einstellungen für die oben genannten Pinbelegungen gesetzt.

Final muss der Sensor parametrisiert werden und mit dem im TTN Netzwerk angelegten gekoppelt werden.
Hierzu wurden bei der Registrierung des Sensors im TTN drei verschiedene Keys angelegt.

Diese müssen jetzt im Programm wie folgt eingegeben werden:

```c++
41 | //-------------------- LORA CONFIG ----------------------- //
42 | // NwkSKey MSB
43 | static const PROGMEM u1_t NWKSKEY[16] = {0xE4, 0x14, 0xC5, 0x65, 0x24, 0x2D, 0x7B, 0x60, 0x61, 0x43, 0xD3, 0xC1, 0x49, 0x87, 0x90, 0x45 };
44 | // AppSKey MSB
45 | static const u1_t PROGMEM APPSKEY[16] = {0x0D, 0x79, 0xB8, 0x87, 0x1D, 0xD4, 0x64, 0xFA, 0xCC, 0x5B, 0x3B, 0xE0, 0x08, 0x0E, 0x7A, 0x22 };
46 | // LoRaWAN address = '0x' + Device Address 
47 | static const u4_t DEVADDR = 0x260BA509; //UNIQUE LORA DEVICE ID
```


Anschließend kann das Programm kompiliert und Hochgeladen werden.
Hierzu muss zuvor noch das passende Arduino Board ausgewählt werden.

Im Menü unter Werkzeug kann dies eingestellt werden:

| Arduino Nano Board-Settings 	|      Wert                         |
|-------------------------	|-------------------------	            |
| Board                   	| Arduino Nano                          |
| Prozessor               	| ATmega380p (Old Bootloader)           | 
| Port                     	| COMxx (WIN), /dev/ttyXX (Linux, Mac)  |




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
//UNCOMMENT FOR TTN DOWNLINK DECODER
//function decodeDownlink(input) {
//const bytes = input.bytes
//const TTN_COMPABILITY = true;

// UNCOMMENT FOR CHIRPSTACK CODEC
// function Decode(fPort, bytes) { 
//const TTN_COMPABILITY = false;

    var result = "";
    var resobj = {
        warnings: [],
        errors: [],
        data: {

        }
    };

    var entryobj = {};

    for (var i = 0; i < bytes.length; i++) {
        result += (String.fromCharCode(bytes[i]));
    }
    //SPLIT COMPLETE STRING BY ; TO GET INDIVIDUAL ENTRIES
    const sp = String(result).split(';');
    if(sp){
        //FOR EACH ENTRY
        for(var i = 0; i < sp.length;i++){
            if(sp[i].includes('=')){
                //SPLIT ENTRY FOR =
                const spe = String(sp[i]).split('=');
                if(spe.length === 3){
                    //PARSE EACH ENTRY <type>=<key>=<value>
                    //s => STRING
                    //f => float
                    //i => int
                    if(spe[0] === "s"){
                        entryobj[String(spe[1])] =spe[2];
                    }else if(spe[0] === "f"){
                        entryobj[String(spe[1])] = parseFloat(spe[2]);
                        //CHECK FOR Nan not a number and set to null
                        if(isNaN(entryobj[String(spe[1])])){
                            entryobj[String(spe[1])] = null;
                        }
                    }else if(spe[0] === "i"){
                        entryobj[String(spe[1])] = parseInt(spe[2], 10);
                        //CHECK FOR Nan not a number and set to null
                        if(isNaN(entryobj[String(spe[1])])){
                            entryobj[String(spe[1])] = null;
                        }
                    }
                }
            }
        }
    }
    if(TTN_COMPABILITY){
        resobj["data"] = entryobj;
        return resobj;
    }else{
        return entryobj;
    }


}
```

#### Application
