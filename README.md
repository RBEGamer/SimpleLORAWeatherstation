# MSFHAC_Lora_DHT22_Node
The FH Aachen makerspace lora based dht22 solar powered sensor node

## SETUP


### ChirpStack

#### Device-Profile


#### CODEC: Javascript Decode Function

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
