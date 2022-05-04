/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 2 (connect to TX of other device)
 * TX is digital pin 3 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

SoftwareSerial mySerial(D5, D6); // RX, TX
float avgTmp[10];
int cnt = 0;

const char* ssid = "CMCC-xu97";
const char* password = "";
const char* mqtt_server = "192.168.1.2";
const int mqttPort = 1884;
const char* mqttUser = "sonoff";
const char* mqttPassword = "sonoff";

WiFiClient espClient;
PubSubClient client(espClient);

double preTemp = 0;

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 68);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("temp no callback needed");
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  double preTemp = 0;
  for (; cnt < 20; cnt++)
  {
    avgTmp[cnt % 10] = getTemp(preTemp);
    preTemp = avgTmp[cnt % 10];
    Serial.println(preTemp);
    delay(10);
  }
  Serial.println("init done");

  setup_wifi();
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      // client.subscribe("inTopic");
      Serial.print("connect success");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

double getTemp(double inTemp)
{
  mySerial.write("AT+T\r\n");
  delay(10);
  while(mySerial.available() > 0) {
    String c_tmp = mySerial.readString();
    int c_len = c_tmp.length();
    int e_idx = c_len - 3;
    inTemp = c_tmp.substring(3, c_len).toDouble();
  }
  return inTemp;
}

void loop() // run over and over
{
  delay(3000);
  cnt %= 10;
  int pre_cnt = (cnt - 1+ 10) % 10;
  avgTmp[cnt] = getTemp(avgTmp[pre_cnt]);
  double t_temp = 0;
  int i = 0;
  for (; i < 10; i++) t_temp += avgTmp[i];
  Serial.println(t_temp/10);
  cnt++;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if ((t_temp/10 - preTemp > 0.5) || (preTemp - t_temp/10 > 0.5)) {
    String l_tmp_data = String(t_temp/10,2);
    client.publish("kitchen/dhw_temp", l_tmp_data.c_str(), true);
    preTemp = t_temp/10;
  }
}
