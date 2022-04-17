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

SoftwareSerial mySerial(D5, D6); // RX, TX
float avgTmp[10];
int cnt = 0;

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
  
}
