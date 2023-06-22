
#include <WiFi.h>
#include "delayCommand.h"

char ssid[] = "LT-4 FM";           // SSID of your home WiFi
char pass[] = "mobitell1234";

const char* server_ip = "192.168.0.100";
WiFiClient client;

//parsing data dari esp1
String dataIn = "";
String dt[10];
int i;
boolean parsing = false;

//setting pin
int flip;
int alarm_pin;
int alarm_wifi;
int led_wifi;
int jumlah_pin = 6;
int sw[] = {12, 0, 0, 0, 0, 0};
int led[] = {13, 0, 0, 0, 0, 0};

//data sw
int d_sw[] = {1, 1, 1, 1, 1, 1};
int d_sw1[] = {1, 1, 1, 1, 1, 1};

//state flip-flop
int kondisi_flip = 0;

//delay
const unsigned long onTime_flip = 20;
const unsigned long offTime_flip = 300;
const unsigned long u_status = 2000;
const unsigned long delay_data = 100;
const unsigned long delay_button = 1000;
int count;

void setup() {
  Serial.begin(9600);               // only for debug
  WiFi.mode(WIFI_STA);
  connectToWiFi();

  //inisialisasi pin
  for (int a = 0; a <= 0; a++) {
    Serial.println(a);
    pinMode(sw[a], INPUT_PULLUP);
    pinMode(led[a], OUTPUT);
  }
  //pinMode(flip, OUTPUT);
  //pinMode(alarm_pin, OUTPUT);
  //pinMode(alarm_wifi, OUTPUT);
  //pinMode(led_wifi, OUTPUT);

  Serial.println("Connected to wifi");
  Serial.print("Status: "); Serial.println(WiFi.status());    // Network parameters
  Serial.print("IP: ");     Serial.println(WiFi.localIP());
  Serial.print("Subnet: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  Serial.print("Signal: "); Serial.println(WiFi.RSSI());

  //fungsi
  attachDelayCommand_ms(check_data_masuk, delay_data, 0);
  attachDelayCommand_ms(check_tombol, delay_button, 1);
  attachDelayCommand_ms(check_status, u_status, 2);
  //attachDelayCommand_ms(flip_flop_on, onTime_flip, 3);
  //attachDelayCommand_ms(flip_flop_off, offTime_flip, 4);

}

void loop () {
  delayEvent();
}

void check_data_masuk() {
  if (client.available()) {
    char inChar = (char)client.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
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
  for (int a = 0; a <= jumlah_pin - 1; a++) {
    d_sw[a] = dt[a].toInt();
    //Serial.println(d_sw[a]);
  }
}

void check_tombol() {
  String sendData = "#";
  String sendData2 = "#";
  for (int a = 0; a <= jumlah_pin - 1; a++) {
    d_sw1[a] = digitalRead(sw[a]);
    sendData += String(d_sw1[a]) + ',';
    sendData2 += String(d_sw[a]) + ',';
  }
  sendData += "\n";
  Serial.print("switch esp1=");
  Serial.print(sendData);
  Serial.print("switch esp2=");
  Serial.println(sendData2);
  Serial.println();

  int arraySize = sizeof(d_sw) / sizeof(d_sw[1]);
  int arraySize1 = sizeof(d_sw1) / sizeof(d_sw[1]);

  int zeroCount = countZeros(d_sw, arraySize);
  int zeroCount1 = countZeros(d_sw1, arraySize1);

  if (zeroCount1 == 6 && zeroCount == 6) {
    //digitalWrite(alarm,LOW);
    return;
  }

  client.write(sendData.c_str());
  for (int a = 0; a <= jumlah_pin - 1; a++) {
    if (d_sw[a] == 0 && d_sw1[a] == 0 ) {
      Serial.printf("Led Nyla : %d\n", led[a]);
      //digitalWrite(led[a],HIGH);
      //digitalWrite(alarm,LOW);

    }
    else if (d_sw[a] == 0 && d_sw1[a] == 1) {
      Serial.println("Alarm nyala");
      //digitalWrite(alarm,HIGH);
    }
    else if (d_sw[a] == 1 && d_sw1[a] == 0) {
      Serial.println("Alarm nyala");
      //digitalWrite(alarm,HIGH);
    }
    else {
      count = 0;
      Serial.printf("Led Mati : %d\n", led[a]);
      //digitalWrite(led[a],LOW);
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
  if (!client.connected())
  {
    if (WiFi.status() != WL_CONNECTED) {
      //digitalWrite(led_wifi, LOW);
      //digitalWrite(alarm_wifi, HIGH);
      Serial.println("hubungkan kembali");
      connectToWiFi();
    }

    if (client.connect(server_ip, 80)) {
      Serial.println("connected");
      //digitalWrite(led_wifi, HIGH);
      //digitalWrite(alarm_wifi, LOW);
    }
    else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
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
  //digitalWrite(flip, HIGH);
  Serial.println("flip nyala");
  kondisi_flip = 1;
}

void flip_flop_off() {
  if (kondisi_flip == 1) {
    //digitalWrite(flip,LOW);
    Serial.println("flip mati");
    kondisi_flip = 0;
  }
}
