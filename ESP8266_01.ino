#include <ESP8266WiFi.h>

#define Stop 1
#define Forward 2
#define Reverse 3
#define Straight 4
#define Right 5
#define Left 6
#define BuzzOn 7
#define BuzzOff 8
#define TakeMeasure 9
#define Run 10
#define GAS 2
#define READY 0

// change these values to match your network
//char ssid[] = "OTE007599";       //  your network SSID (name)
//char pass[] = "8040624812924736";          //  your network password

char ssid[] = "MultiSensor";       //  your network SSID (name)
char pass[] = "12345678";          //  your network password


WiFiServer server(80);

float sound = 0;
float light = 0;
float gas = 0;
float humidity = 0;
float temperature = 0;
float battery = 0;
byte signals = Stop;
byte prevSignal = Stop;
byte lowBat = 0;


String request = "";
String header = "";
String html_1 = "";
String html_2 = "";
int LED_Pin = 2;


void sites();



/*
   Sketch: ESP8266_LED_CONTROL_AJAX_02
   Intended to be run on an ESP8266
*/






void setup()
{
  Serial.begin(115200);
  pinMode(READY, INPUT);
  pinMode(GAS, OUTPUT);
  digitalWrite(GAS, LOW);

  WiFi.softAP(ssid, pass);

  //    Serial.println("");
  //    Serial.println(F("[CONNECTED]"));
  //    Serial.print("[IP ");
  //    Serial.print(WiFi.localIP());
  //    Serial.println("]");

  // start a server
  server.begin();
  sites();

}



void loop()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)  {
    return;
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');
  //  Serial.println(request);

  if ( request.indexOf("BP") > 0 )  {
    signals = BuzzOn;
  }
  else if ( request.indexOf("SB") > 0 ) {
    signals = BuzzOff;
  }
  else if ( request.indexOf("FW") > 0 )  {
    signals = Forward;
  }
  else if ( request.indexOf("RV") > 0 ) {
    signals = Reverse;
  }
  else if ( request.indexOf("STOP") > 0 ) {
    signals = Stop;
  }
  else if ( request.indexOf("RT") > 0 )  {
    signals = Right;
  }
  else if ( request.indexOf("LT") > 0 ) {
    signals = Left;
  }
  else if ( request.indexOf("STR") > 0 ) {
    signals = Straight;
  }

  else if (request.indexOf("measuring") > 0)
  {
    if (!lowBat) {
      digitalWrite(GAS, HIGH);
    }
    else {
      digitalWrite(GAS, LOW);
    }
    prevSignal = TakeMeasure;
    signals = TakeMeasure;
    Serial.write(TakeMeasure);
    while (1) {
      if (Serial.available()) {
        temperature = Serial.read();
        humidity = Serial.read();
        light = Serial.read();
        sound = Serial.read();
        gas = Serial.read();
        battery = Serial.read();
        if (battery == 0) {
          lowBat = 1;
        }
        else {
          lowBat = 0;
        }
        while (Serial.available()) {
          Serial.read();
        }
        temperature /= 5;
        humidity /= 2.55;
        light /= 2.55;
        sound /= 2.55;
        gas /= 2.55;
        battery /= 2.55;
        break;
      }
    }
    sites();
    client.flush();
    client.print( header );
    client.print(html_2);
  }
  else if (request.indexOf("running") > 0)
  {
    signals = Run;
    Serial.write(signals);
    digitalWrite(GAS, LOW);
    client.flush();
    client.print( header );
    client.print(html_1);
  }

  if (signals != prevSignal) {
    Serial.write(signals);
    prevSignal = signals;
  }

  //        boolean pinStatus = digitalRead(LED_Pin);
  //        if (pinStatus==HIGH) { html_1.replace("Turn on the LED","Turn off the LED");   }
  //        else                 { html_1.replace("Turn off the LED","Turn on the LED");   }




  // The client will actually be disconnected when the function returns and 'client' object is detroyed
}



void sites() {

  header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

  html_1 = R"=====(
<!DOCTYPE html>
<html>
 <head>
 <meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'/>
 <meta charset='utf-8'>

<div style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;">


 <style>
  body {font-size:140%;} 
  #main {display: table; margin: auto;  padding: 0 10px 0 10px; } 
  h2 {text-align:center; } 
  #BP {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; left: 10px; top: 20px;}
  #FW {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; left: 50px; bottom: 150px;}
  #RV {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; left: 50px; bottom: 50px;}
  #RT {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; right: 50px; bottom: 100px;}
  #LT {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; right: 200px; bottom: 100px;}
  #Measure {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; right: 10px; top: 20px;} 
 </style>



<script>
  function Beep()
  {
    ajaxLoad('BP'); 
  }
  function StopBeep()
  {
    ajaxLoad('SB'); 
  }
  function RunF()
  {
    ajaxLoad('FW'); 
  }
  function RunR()
  {
    ajaxLoad('RV'); 
  }
  function Stop()
  {
    ajaxLoad('STOP'); 
  }
  function TurnR()
  {
    ajaxLoad('RT'); 
  }
  function TurnL()
  {
    ajaxLoad('LT'); 
  }
  function TurnS()
  {
    ajaxLoad('STR'); 
  }
  function Measure()
  {
    window.location.href ='/measuring'; 
  }

  
//   function switchLED1() 
//  {
//       var button_text = document.getElementById("LED_button").value;
//     if (button_text=="Turn on the LED") //PF
//     {
//       document.getElementById("LED_button").value = "Turn off the LED";
//       ajaxLoad('LEDON'); 
//     }


var ajaxRequest = null;
if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }


function ajaxLoad(ajaxURL)
{
  if(!ajaxRequest){ alert("AJAX is not supported."); return; }
  
  ajaxRequest.open("GET",ajaxURL,true);
  ajaxRequest.onreadystatechange = function()
  {
    if(ajaxRequest.readyState == 4 && ajaxRequest.status==200)
    {
      var ajaxResult = ajaxRequest.responseText;
    }
  }
  ajaxRequest.send();
}
   
</script>

 <title>RUN!</title>
</head>

<body>

  <h2>RUN!</h2>

  <input type="button" id = "Measure" onclick="Measure()" value="Measure"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "BP" ontouchstart="Beep()" ontouchend="StopBeep()" value="BP"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "FW" ontouchstart="RunF()" ontouchend="Stop()" value="FW"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "RV" ontouchstart="RunR()" ontouchend="Stop()" value="RV"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "RT" ontouchstart="TurnR()" ontouchend="TurnS()" value="RIGHT"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "LT" ontouchstart="TurnL()" ontouchend="TurnS()" value="LEFT"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 

 </div>
</body>
</html>
</html>


)====="; 










html_2 = R"=====(

<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  #boxRed {
    background-color: lightgrey;
    width: 300px;
    border: 35px solid red;
    padding: 10px;
    margin: 20px;
  }
  #boxGreen {
    background-color: lightgrey;
    width: 300px;
    border: 35px solid green;
    padding: 10px;
    margin: 20px;
  }

  body {font-size:140%;} 
  #main {display: table; margin: auto;  padding: 0 10px 0 10px; } 
  #Run {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; right: 10px; top: 10px;}
  #Refresh {padding:10px 10px 10px 10px; width:20%;  background-color: #50FF50; font-size: 250%; position: absolute; left: 10px; top: 10px;}

  </style>
</head>
<body>
  <h2>Measure</h2>
  <p>
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">)=====";

  html_2 += temperature;

  html_2 += R"=====(
    </span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="dht-labels">Humidity</span>
    <span id="humidity">)=====";

  html_2 += humidity;

  html_2 += R"=====(
    </span>
    <sup class="units">%</sup>
  </p>
  <p>
    <span class="dht-labels">Light</span> 
    <span id="lignt">)=====";

  html_2 += light;

  html_2 += R"=====(
    </span>
    <sup class="units">%</sup>
  </p>
  <p>
    <span class="dht-labels">Sound</span> 
    <span id="sound">)=====";

  html_2 += sound;

  html_2 += R"=====(
    </span>
    <sup class="units">%</sup>
  </p>
  <p>
    <span class="dht-labels">Gas</span> 
    <span id="gas">)=====";

  html_2 += gas;

  html_2 += R"=====(
    </span>
    <sup class="units">%</sup>
  </p>
  <p>
    <span class="dht-labels">Battery</span> 
    <span id="battery">)=====";

  html_2 += battery;

  html_2 += R"=====(
    </span>
    <sup class="units">%</sup>
  </p>

  <input type="button" id = "Run" onclick="Run()" value="Run!"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 
  <input type="button" id = "Refresh" onclick="Refresh()" value="Refresh"  style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"unselectable="on"onselectstart="return false;" onmousedown="return false;"     /> 

)=====";
  if(digitalRead(READY)){
    html_2 += "<boxGreen>Ready!</boxGreen>";
  }
  else{
    html_2 += "<boxRed>Not Ready</boxRed>";
  }
  html_2 += R"=====(
  <form name="addtext" onsubmit="download(this['name'].value, this['text'].value)">
  <input type="hidden" name="text" value="Temperature = )=====";

  html_2 += temperature;

  html_2 += R"=====(
Humidity = )=====";

  html_2 += humidity;

  html_2 += R"=====(
Light = )=====";

  html_2 += light;

  html_2 += R"=====(
Sound = )=====";

  html_2 += sound;

  html_2 += R"=====(
Gas = )=====";

  html_2 += gas;

  html_2 += R"=====(
  
  ">
  <input type="text" name="name" value="" placeholder="File Name">
  <input type="submit" onClick="addTexttxt();" value="Save As TXT">
  </form>
  
</body>
<script>

  var temperature = )=====";
  html_2 += temperature;
  html_2 += R"=====(
  ;
  var humidity =)=====";
  html_2 += humidity;
  html_2 += R"=====(
  ;
  var light =)=====";
  html_2 += light;
  html_2 += R"=====(
  ;
  var sound =)=====";
  html_2 += sound;
  html_2 += R"=====(
  ;
  var gas = )=====";
  html_2 += gas;
  html_2 += R"=====(
  ;
  
  function Run()
  {
    window.location.href ='/running';
  }
  function Refresh()
  {
    window.location.href ='/measuring';
  }

</script>

<script language="Javascript" >
function download(filename, text) {
  var pom = document.createElement('a');
  pom.setAttribute('href', 'data:text/plain;charset=utf-8,' + 

encodeURIComponent(text));
  pom.setAttribute('download', filename);

  pom.style.display = 'none';
  document.body.appendChild(pom);

  pom.click();

  document.body.removeChild(pom);
}

function addTextTXT()
{
    document.addtext.name.value = document.addtext.name.value + ".txt"
}
</script>


</html>

)====="; 

}
