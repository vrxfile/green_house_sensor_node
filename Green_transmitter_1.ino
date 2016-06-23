#include <VirtualWire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Sleep_n0m1.h>

#define trx_gnd_pin 2
#define trx_vcc_pin 4
#define transmit_pin 3

#define m_gnd_pin A1
#define m_vcc_pin A2
#define m_sig_pin A3

#define t_gnd_pin 10
#define t_vcc_pin A5
#define t_sig_pin 5
OneWire oneWire(t_sig_pin);
DallasTemperature ds_sensor(&oneWire);

#define led_pin 13

#define MY_ADDR 0x01

#define SEND_INTERVAL 55000

#define BUFF_SIZE 32
char buf1[BUFF_SIZE] = {0};
char buf2[BUFF_SIZE] = {0};
char str_tmp1[8] = {0};
char str_tmp2[8] = {0};

Sleep sleep;

void setup()
{
  // Init serial port
  Serial.begin(9600);
  while (!Serial) {}

  // Init power for transmitter
  pinMode(trx_gnd_pin, OUTPUT);
  digitalWrite(trx_gnd_pin, LOW);
  pinMode(trx_vcc_pin, OUTPUT);
  digitalWrite(trx_vcc_pin, HIGH);

  // Init power for moisture sensor
  pinMode(m_gnd_pin, OUTPUT);
  digitalWrite(m_gnd_pin, LOW);
  pinMode(m_vcc_pin, OUTPUT);
  digitalWrite(m_vcc_pin, LOW);

  // Init power for DS18B20 sensor
  pinMode(t_gnd_pin, OUTPUT);
  digitalWrite(t_gnd_pin, LOW);
  pinMode(t_vcc_pin, OUTPUT);
  digitalWrite(t_vcc_pin, HIGH);

  // Init DS18B20 sensor
  ds_sensor.begin();

  // Init onboard LED
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Init analog input for moisture sensor
  pinMode(m_sig_pin, INPUT);

  // Init transmitter
  vw_set_tx_pin(transmit_pin);
  vw_setup(1024);
}

void loop()
{
  Serial.println("Measuring...");

  // Measure moisture sensor
  digitalWrite(m_vcc_pin, HIGH); delay(250);
  float m = analogRead(m_sig_pin) / 1023.0 * 100.0;
  digitalWrite(m_vcc_pin, LOW);
  Serial.print("Moisture: ");
  Serial.print(m);
  Serial.println(" %");

  // Measure DS18B20 sensor
  digitalWrite(t_vcc_pin, HIGH); delay(250);
  ds_sensor.begin();
  ds_sensor.requestTemperatures();
  float t = ds_sensor.getTempCByIndex(0);
  digitalWrite(t_vcc_pin, LOW);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  Serial.println();

  // Preparing network packet in JSON format
  dtostrf(m, 3, 1, str_tmp1);
  dtostrf(t, 3, 1, str_tmp2);
  sprintf(buf1, "{\"A\":%d,\"M\":%s}\n", MY_ADDR, str_tmp1);
  sprintf(buf2, "{\"A\":%d,\"T\":%s}\n", MY_ADDR, str_tmp2);
  Serial.print("JSON1 length: ");
  Serial.println(strlen(buf1));
  Serial.print("JSON1 packet: ");
  Serial.println(buf1);
  Serial.print("JSON2 length: ");
  Serial.println(strlen(buf2));
  Serial.print("JSON2 packet: ");
  Serial.println(buf2);

  Serial.println("Sending data...");

  // Sending data to server
  digitalWrite(led_pin, HIGH);
  digitalWrite(trx_vcc_pin, HIGH); delay(250);
  vw_send((uint8_t *)buf1, strlen(buf1));
  vw_wait_tx();
  vw_send((uint8_t *)buf2, strlen(buf2));
  vw_wait_tx();
  digitalWrite(trx_vcc_pin, LOW);
  digitalWrite(led_pin, LOW);

  Serial.println("Data sent OK!");
  Serial.println();

  // Sleep mode
  delay(250);
  sleep.pwrDownMode();
  sleep.sleepDelay(SEND_INTERVAL);
}
