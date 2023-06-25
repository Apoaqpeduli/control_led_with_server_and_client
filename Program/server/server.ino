
#include <WiFi.h>
#include "delayCommand.h"


#define MAX_SRV_CLIENTS 1
#define PORT  80

char ssid[] = "LOGIC";           // SSID of your home WiFi
char pass[] = "kwato1234";

//setting agar ip menjadi static
#define staticIP "192.168.1.100"     // Desired static IP address
#define gate "192.168.1.1"       // IP address of your gateway/router
#define subnet "255.255.255.0"    // Subnet mask

//setting pin
int flip= 22;
int alarm_pin= 23;
int alarm_wifi= 18;
int led_wifi= 19;
int jumlah_pin = 6;
int sw[] = {15, 12, 4, 16, 17, 5};
int led[] = {35, 32, 33, 26, 27, 14};

//data sw
int d_sw[6] = {1, 1, 1, 1, 1, 1};
int d_sw1[6] = {1, 1, 1, 1, 1, 1};

WiFiServer server(PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];

//parsing data esp1
String dataIn = "";
String dt[10];
int i;
boolean parsing = false;

//delay
const unsigned long onTime_flip = 200;
const unsigned long offTime_flip = 1000;
const unsigned long u_status = 2000;
const unsigned long delay_data = 100;
const unsigned long delay_button = 1000;
int count;

//state flip-flop
int kondisi_flip = 0;


void setup() {
  Serial.begin(9600);

  for (int a = 0; a <= jumlah_pin-1; a++) {
    Serial.println(a);
    pinMode(sw[a], INPUT_PULLUP);
    pinMode(led[a], OUTPUT);
  }
  pinMode(flip, OUTPUT);
  pinMode(alarm_pin, OUTPUT);
  pinMode(alarm_wifi, OUTPUT);
  pinMode(led_wifi, OUTPUT);

  IPAddress staticIPAddr, gatewayIPAddr, subnetMaskAddr;
  staticIPAddr.fromString(staticIP);
  gatewayIPAddr.fromString(gate);
  subnetMaskAddr.fromString(subnet);

  if (!WiFi.config(staticIPAddr, gatewayIPAddr, subnetMaskAddr)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  server.begin();
  server.setNoDelay(true);

  Serial.print("SSID: ");           Serial.println(ssid);
  Serial.print("Password: ");       Serial.println(pass);

  //Serial.print("AP IP address: ");  Serial.println(myIP);
  Serial.print("Port: ");           Serial.println(PORT);
  Serial.print("IP: ");     Serial.println(WiFi.localIP());

  attachDelayCommand_ms(check_data_masuk, delay_data, 0);
  attachDelayCommand_ms(check_tombol, delay_button, 1);
  attachDelayCommand_ms(check_status, u_status, 2);
  attachDelayCommand_ms(flip_flop_on, 1, 3);
  //attachDelayCommand_ms(flip_flop_off, offTime_flip, 4); //tidak terpakai
}

void loop() {
  delayEvent();
}

void check_data_masuk() {
  //check clients for data
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        while (serverClients[i].available()) {
          char inChar = (char)serverClients[i].read();
          dataIn += inChar;
          if (inChar == '\n') {
            parsing = true;
          }
        }
      }
    }
  }
  if (parsing)
  {
    parsingData();
    parsing = false;
    dataIn = "";
  }
}

void parsingData()
{
  int j = 0;
  //kirim data yang telah diterima sebelumnya
  //Serial.print("data masuk : ");
  //Serial.print(dataIn);
  //Serial.print("\n");
  //inisialisasi variabel, (reset isi variabel)
  dt[j] = "";
  //proses parsing data
  for (i = 1; i < dataIn.length(); i++)
  {
    //pengecekan tiap karakter dengan karakter (#) dan (,)
    if ((dataIn[i] == '#') || (dataIn[i] == ','))
    {
      //increment variabel j, digunakan untuk merubah index array penampung
      j++;
      dt[j] = ""; //inisialisasi variabel array dt[j]
    }
    else
    {
      //proses tampung data saat pengecekan karakter selesai.
      dt[j] = dt[j] + dataIn[i];
    }
  }
  //kirim data hasil parsing
  for (int a = 0; a <= jumlah_pin-1; a++) {
    d_sw[a] = dt[a].toInt();
    //Serial.println(d_sw[a]);
  }
}

void check_tombol() {
  uint8_t i;
  String sendData = "#";
  String sendData2 = "#";
  for (int a = 0; a <= jumlah_pin - 1; a++) {
    d_sw1[a] = digitalRead(sw[a]);
    sendData += String(d_sw1[a]) + ',';
    sendData2 += String(d_sw[a]) + ',';
  }
  sendData += "\n";
  Serial.print("switch esp2=");
  Serial.print(sendData);
  Serial.print("switch esp1=");
  Serial.println(sendData2);
  Serial.println();

  int arraySize = sizeof(d_sw) / sizeof(d_sw[1]);
  int arraySize1 = sizeof(d_sw1) / sizeof(d_sw[1]);

  int zeroCount = countZeros(d_sw, arraySize);
  int zeroCount1 = countZeros(d_sw1, arraySize1);

  if (zeroCount1 == 6 && zeroCount == 6) {
    digitalWrite(alarm_pin,LOW);
    for (int a = 0; a <= jumlah_pin - 1; a++) {
      digitalWrite(led[a],LOW);
    }
  
    return;
  }

  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      serverClients[i].write(sendData.c_str());
    }
  }

  for (int a = 0; a <= jumlah_pin - 1; a++) {
    if (d_sw[a] == 0 && d_sw1[a] == 0 ) {
      Serial.printf("Led Nyla : %d\n", led[a]);
      digitalWrite(led[a],HIGH);
      digitalWrite(alarm_pin,LOW);

    }
    else if (d_sw[a] == 0 && d_sw1[a] == 1) {
      Serial.println("Alarm mati");
      digitalWrite(alarm_pin,LOW);
    }
    else if (d_sw[a] == 1 && d_sw1[a] == 0) {
      Serial.println("Alarm nyala");
      digitalWrite(alarm_pin,HIGH);
    }
    else {
      count = 0;
      Serial.printf("Led Mati : %d\n", led[a]);
      //digitalWrite(alarm, LOW);
      digitalWrite(led[a],LOW);
    }
  }
}

int countZeros(int array[], int size) {
  int zeroCount = 0;

  for (int i = 0; i < size; i++) {
    if (array[i] == 1) {
      zeroCount++;
    }
  }

  return zeroCount;
}

void check_status() {
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.println(i);
        digitalWrite(led_wifi, HIGH);
        digitalWrite(alarm_wifi, LOW);
        break;
      }
    }
    //no free/disconnected spot so reject
    if (i == MAX_SRV_CLIENTS) {
      WiFiClient serverClient = server.available();
      serverClient.stop();
      Serial.println("Connection rejected ");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led_wifi, LOW);
    digitalWrite(alarm_wifi, HIGH);
    Serial.println("hubungkan kembali");
    connectToWiFi();
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("Wi-Fi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void flip_flop_on() {
  digitalWrite(flip, HIGH);
  delay(onTime_flip);
  digitalWrite(flip, LOW);
  delay(offTime_flip);
  //Serial.println("flip nyala");
  kondisi_flip = 1;
}

void flip_flop_off() {
  if (kondisi_flip == 1) {
    digitalWrite(flip,LOW);
    //Serial.println("flip mati");
    kondisi_flip = 0;
  }
}
