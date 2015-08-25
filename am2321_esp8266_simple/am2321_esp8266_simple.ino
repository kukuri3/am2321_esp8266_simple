#define kID (4)
#define kNAME "1F_HALL"

#include <Wire.h>
#include <AM2321.h>
#include <Weather.h>
#include <math.h>
#include <FreqCounter.h>
AM2321 ac;

#include "ESP8266.h"
#include <SoftwareSerial.h>

#define SSID        "yosemite"
#define PASSWORD    "ftnde201315"
#define HOST_NAME   "192.168.1.18"
#define HOST_PORT   (4126)

SoftwareSerial mySerial(3, 2); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

#define LED_PIN 13

float temperature;
float humidity;
float dewpoint;
long int freq;
long int lux;

void setup(void)
{
    Serial.begin(9600);  
        //LEDピンを出力に
    pinMode(LED_PIN, OUTPUT);
    
    Serial.print("setup begin\r\n");
      
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("softap ok\r\n");
    } else {
        Serial.print("softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("AP success\r\n");
        Serial.print("IP: ");
        Serial.println(wifi.getLocalIP().c_str());  
        Serial.println("ip got!");    
    } else {
        Serial.print("AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("s ok\r\n");
    } else {
        Serial.print("serr\r\n");
    }
    
        
    if (wifi.registerUDP(HOST_NAME, HOST_PORT)) {
        Serial.print(" udp ok\r\n");
    } else {
        Serial.print("udp err\r\n");
    }
    
    Serial.print("setup end\r\n");
    

}

	double dewPoint(double celsius, double humidity) {
		// RATIO was originally named A0, possibly confusing in Arduino context
		double RATIO = 373.15 / (273.15 + celsius);
		double SUM = -7.90298 * (RATIO - 1);
		SUM += 5.02808 * log10(RATIO);
		SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
		SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
		SUM += log10(1013.246);
		double VP = pow(10, SUM - 3) * humidity;
		double T = log(VP/0.61078);   // temp var
		return (241.88 * T) / (17.558 - T);
	}
void loop(void)
{
    uint8_t buffer[128] = {0};
  char buf[128];
    
  //LED点滅
  digitalWrite(LED_PIN, 1);
  delay(10);
  digitalWrite(LED_PIN, 0);

  //温度取得
  ac.read();
  temperature=((float)ac.temperature)/10.0;
  humidity=((float)ac.humidity)/10.0;
  dewpoint=dewPoint(temperature,humidity);
  
  
  //S9705から照度を取得
  
  FreqCounter::f_comp = 8;             // Set compensation to 12
  FreqCounter::start(1000);            // Start counting with gatetime of 1000ms
  while (FreqCounter::f_ready == 0);  // wait until counter ready
  freq = FreqCounter::f_freq;            // read result
  lux=freq/500;


  
  //シリアルに出力
//  Serial.print(millis()/1000);
  Serial.print(millis()/1000);
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",Humidity[%],");
  Serial.print(humidity);
  Serial.print(", Dewpoint[C],");
  Serial.print(dewpoint);
  Serial.print(", freq[Hz],");
  Serial.print(freq);
  Serial.print(",Lux,");
  Serial.print(lux);
  
  
  Serial.print("\r\n");
  
  //wifiに出力
  sprintf(buf,"id,%d,name,%s,time,%d,temp,%d,hum,%d,dew,%d,lux,%d\r\n",kID,kNAME,(int)(millis()/1000),(int)(temperature*10),(int)(humidity*10),(int)(dewpoint*10),lux);
  wifi.send((const uint8_t*)buf, strlen(buf));
  Serial.print(buf);
  

   delay(5000);
}
          
