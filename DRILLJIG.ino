/* =========================================================
   ESP32 AUTOMATIC MULTI PCD DRILL JIG
   PROFESSIONAL INDUSTRIAL WEB DASHBOARD
   ========================================================= */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* ================= PIN CONFIG ================= */

#define STEP_PIN        18
#define DIR_PIN         19
#define EN_PIN          23

#define LIMIT_SW        27
#define MANUAL_SW       14

#define DWELL_UP_BTN    26
#define DWELL_DOWN_BTN   4

#define SDA_PIN         21
#define SCL_PIN         22

/* ================= LCD ================= */

LiquidCrystal_I2C lcd(0x27,16,2);

/* ================= WIFI ================= */

const char* ssid     = "udayakumar";
const char* password = "udaya123";

WebServer server(80);

/* ================= MOTOR ================= */

const int stepsPerRev = 200;

/* ================= VARIABLES ================= */

int setAngleValue = 60;

int currentAngle = 0;

bool autoMode = true;

bool waiting = false;

unsigned long dwellTime = 500;

unsigned long dwellStart = 0;

int targetPieces = 5;

int completedPieces = 0;

/* ================= WEB PAGE ================= */

String webpage = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>PSG DRILL JIG</title>

<style>

*{
margin:0;
padding:0;
box-sizing:border-box;
font-family:'Segoe UI',sans-serif;
}

body{

background:
linear-gradient(
135deg,
#020617,
#0f172a,
#111827
);

min-height:100vh;

padding:20px;

color:white;
}

/* ================= HEADER ================= */

.topbar{

background:
rgba(255,255,255,0.05);

backdrop-filter:blur(15px);

border-radius:25px;

padding:25px;

text-align:center;

box-shadow:
0 8px 30px rgba(0,0,0,0.5);

margin-bottom:25px;
}

.topbar h1{

font-size:34px;

font-weight:bold;

color:#60a5fa;
}

.topbar p{

margin-top:10px;

font-size:18px;

color:#cbd5e1;
}

/* ================= DASHBOARD ================= */

.dashboard{

display:grid;

grid-template-columns:
repeat(3,1fr);

gap:20px;
}

/* ================= CARD ================= */

.card{

background:
rgba(255,255,255,0.05);

backdrop-filter:blur(12px);

border-radius:25px;

padding:25px;

box-shadow:
0 8px 30px rgba(0,0,0,0.4);

transition:0.3s;
}

.card:hover{

transform:
translateY(-5px);
}

.card h3{

margin-bottom:20px;

color:#60a5fa;

font-size:22px;

text-align:center;
}

/* ================= VALUE ================= */

.bigvalue{

font-size:48px;

font-weight:bold;

text-align:center;

color:#facc15;

margin:20px 0;
}

/* ================= BUTTON ================= */

button{

padding:14px 20px;

font-size:16px;

font-weight:bold;

border:none;

border-radius:14px;

cursor:pointer;

margin:5px;

color:white;

transition:0.3s;
}

button:hover{

transform:scale(1.05);
}

.green{

background:
linear-gradient(
135deg,
#16a34a,
#22c55e
);
}

.red{

background:
linear-gradient(
135deg,
#dc2626,
#ef4444
);
}

.blue{

background:
linear-gradient(
135deg,
#2563eb,
#3b82f6
);
}

/* ================= INPUT ================= */

input{

width:100%;

padding:15px;

font-size:18px;

border:none;

border-radius:14px;

background:
rgba(255,255,255,0.08);

color:white;

margin-bottom:15px;

text-align:center;

outline:none;
}

/* ================= DIAL ================= */

canvas{

display:block;

margin:auto;

background:
rgba(255,255,255,0.03);

border-radius:50%;

padding:10px;
}

/* ================= MOBILE ================= */

@media(max-width:900px){

.dashboard{

grid-template-columns:
1fr;
}
}

</style>

</head>

<body>

<!-- ================= HEADER ================= -->

<div class="topbar">

<h1>PSG COLLEGE OF TECHNOLOGY</h1>

<p>AUTOMATIC MULTI PCD DRILL JIG</p>

</div>

<!-- ================= DASHBOARD ================= -->

<div class="dashboard">

<!-- 1 MODE -->

<div class="card">

<h3>MODE CONTROL</h3>

<div class="bigvalue"
id="mode">
AUTO
</div>

<center>

<button class="green"
onclick="setMode('auto')">
AUTO
</button>

<button class="red"
onclick="setMode('manual')">
MANUAL
</button>

</center>

</div>

<!-- 2 ANGLE -->

<div class="card">

<h3>SET ANGLE</h3>

<div class="bigvalue">

<span id="setangle">
60
</span>°

</div>

<input type="number"
id="angval"
placeholder="0-360 Degree">

<button class="blue"
onclick="setAngle()">
SET ANGLE
</button>

</div>

<!-- 3 TARGET -->

<div class="card">

<h3>TARGET PIECES</h3>

<div class="bigvalue"
id="target">
5
</div>

<input type="number"
id="tval"
placeholder="Target Pieces">

<button class="blue"
onclick="setTarget()">
SET TARGET
</button>

</div>

<!-- 4 PRODUCTION -->

<div class="card">

<h3>COMPLETED PIECES</h3>

<div class="bigvalue"
id="donepcs">
0
</div>

</div>

<!-- 5 DWELL -->

<div class="card">

<h3>DWELL TIME</h3>

<div class="bigvalue">

<span id="dwell">
500
</span> ms

</div>

<center>

<button class="green"
onclick="send('inc')">
+
</button>

<button class="red"
onclick="send('dec')">
-
</button>

</center>

</div>

<!-- 6 LIVE ANGLE -->

<div class="card">

<h3>LIVE ANGLE</h3>

<div class="bigvalue">

<span id="liveangle">
0
</span>°

</div>

<canvas id="dial"
width="220"
height="220">
</canvas>

</div>

</div>

<script>

/* ================= COMMAND ================= */

function send(cmd){

fetch("/"+cmd);
}

function setMode(mode){

fetch("/"+mode);
}

function setTarget(){

let v =
document.getElementById("tval").value;

fetch("/setTarget?val="+v);
}

function setAngle(){

let v =
document.getElementById("angval").value;

fetch("/setAngle?val="+v);
}

/* ================= DIAL ================= */

const c =
document.getElementById("dial");

const x =
c.getContext("2d");

function drawDial(a){

x.clearRect(0,0,220,220);

const cx=110;
const cy=110;
const r=90;

/* OUTER RING */

x.beginPath();

x.arc(cx,cy,r,0,2*Math.PI);

x.strokeStyle="#2563eb";

x.lineWidth=10;

x.stroke();

/* ANGLE ARC */

let end=
(a-90)*Math.PI/180;

x.beginPath();

x.arc(
cx,
cy,
r,
-Math.PI/2,
end
);

x.strokeStyle="#22c55e";

x.lineWidth=10;

x.stroke();

/* NEEDLE */

x.beginPath();

x.moveTo(cx,cy);

x.lineTo(
cx+Math.cos(end)*(r-20),
cy+Math.sin(end)*(r-20)
);

x.strokeStyle="#facc15";

x.lineWidth=5;

x.stroke();
}

/* ================= UPDATE ================= */

function update(){

fetch("/status")

.then(r=>r.json())

.then(d=>{

document.getElementById("mode")
.innerHTML=d.mode;

document.getElementById("setangle")
.innerHTML=d.setangle;

document.getElementById("liveangle")
.innerHTML=d.liveangle;

document.getElementById("target")
.innerHTML=d.target;

document.getElementById("donepcs")
.innerHTML=d.donepcs;

document.getElementById("dwell")
.innerHTML=d.dwell;

drawDial(d.liveangle);

});
}

setInterval(update,500);

</script>

</body>
</html>

)rawliteral";

/* ================= ROTATE FUNCTION ================= */

void rotateAngle(float angle){

  if(angle <= 0) return;

  int steps =
  (angle * stepsPerRev) / 360.0;

  digitalWrite(EN_PIN, LOW);

  for(int i=0;i<steps;i++){

    digitalWrite(STEP_PIN,HIGH);
    delayMicroseconds(800);

    digitalWrite(STEP_PIN,LOW);
    delayMicroseconds(800);
  }

  digitalWrite(EN_PIN,HIGH);
}

/* ================= SETUP ================= */

void setup(){

  Serial.begin(115200);

  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pinMode(EN_PIN,OUTPUT);

  pinMode(LIMIT_SW,INPUT_PULLUP);
  pinMode(MANUAL_SW,INPUT_PULLUP);

  pinMode(DWELL_UP_BTN,INPUT_PULLUP);
  pinMode(DWELL_DOWN_BTN,INPUT_PULLUP);

  digitalWrite(DIR_PIN,HIGH);

  digitalWrite(EN_PIN,HIGH);

  /* ================= LCD ================= */

  Wire.begin(SDA_PIN,SCL_PIN);

  lcd.init();

  lcd.backlight();

  /* ================= WIFI ================= */

  WiFi.softAP(ssid,password);

  lcd.setCursor(0,0);
  lcd.print("IP ADDRESS:");

  lcd.setCursor(0,1);
  lcd.print(WiFi.softAPIP());

  Serial.println(WiFi.softAPIP());

  delay(3000);

  lcd.clear();

  /* ================= ROUTES ================= */

  server.on("/",[](){

    server.send(200,
    "text/html",
    webpage);
  });

  server.on("/auto",[](){

    autoMode=true;

    server.send(200,
    "text/plain",
    "AUTO");
  });

  server.on("/manual",[](){

    autoMode=false;

    server.send(200,
    "text/plain",
    "MANUAL");
  });

  server.on("/inc",[](){

    if(dwellTime<5000)
      dwellTime+=100;

    server.send(200,
    "text/plain",
    "OK");
  });

  server.on("/dec",[](){

    if(dwellTime>200)
      dwellTime-=100;

    server.send(200,
    "text/plain",
    "OK");
  });

  server.on("/setTarget",[](){

    if(server.hasArg("val")){

      targetPieces =
      server.arg("val").toInt();
    }

    server.send(200,
    "text/plain",
    "OK");
  });

  server.on("/setAngle",[](){

    if(server.hasArg("val")){

      setAngleValue =
      server.arg("val").toInt();

      if(setAngleValue<0)
        setAngleValue=0;

      if(setAngleValue>360)
        setAngleValue=360;
    }

    server.send(200,
    "text/plain",
    "OK");
  });

  server.on("/status",[](){

    String json="{";

    json+="\"mode\":\"";
    json+=(autoMode?"AUTO":"MANUAL");
    json+="\",";

    json+="\"setangle\":";
    json+=String(setAngleValue);
    json+=",";

    json+="\"liveangle\":";
    json+=String(currentAngle);
    json+=",";

    json+="\"target\":";
    json+=String(targetPieces);
    json+=",";

    json+="\"donepcs\":";
    json+=String(completedPieces);
    json+=",";

    json+="\"dwell\":";
    json+=String(dwellTime);

    json+="}";

    server.send(200,
    "application/json",
    json);
  });

  server.begin();
}

/* ================= LOOP ================= */

void loop(){

  server.handleClient();

  /* ================= AUTO MODE ================= */

  if(autoMode &&
     digitalRead(LIMIT_SW)==LOW &&
     !waiting){

    waiting=true;

    dwellStart=millis();
  }

  if(waiting &&
     millis()-dwellStart>=dwellTime){

    waiting=false;

    rotateAngle(setAngleValue);

    currentAngle += setAngleValue;

    if(currentAngle>=360){

      currentAngle=0;

      completedPieces++;
    }
  }

  /* ================= MANUAL MODE ================= */

  if(!autoMode &&
     digitalRead(MANUAL_SW)==LOW){

    delay(50);

    rotateAngle(setAngleValue);

    currentAngle += setAngleValue;

    if(currentAngle>=360){

      currentAngle=0;

      completedPieces++;
    }

    while(digitalRead(MANUAL_SW)==LOW);
  }

  /* ================= DWELL BUTTONS ================= */

  if(digitalRead(DWELL_UP_BTN)==LOW){

    if(dwellTime<5000)
      dwellTime+=100;

    delay(200);
  }

  if(digitalRead(DWELL_DOWN_BTN)==LOW){

    if(dwellTime>200)
      dwellTime-=100;

    delay(200);
  }

  /* ================= LCD DISPLAY ================= */

  lcd.setCursor(0,0);

  lcd.print("A:");
  lcd.print(currentAngle);

  lcd.print(" D:");
  lcd.print(dwellTime/100);

  lcd.print("   ");

  lcd.setCursor(0,1);

  lcd.print(autoMode ?
  "AUTO":"MAN ");

  lcd.print(" P:");
  lcd.print(completedPieces);

  lcd.print("/");
  lcd.print(targetPieces);

  lcd.print("   ");
}