

//#define kID (4)
//#define kNAME "1F_HALL"
//#define kID (5)
//#define kNAME "1F_YOUSITU"
#define kID (6)
#define kNAME "2F_HALL"
//#define kID (7)
//#define kNAME "2F_B"
/*
#define SSID        "yosemite"
#define PASSWORD    "ftnde201315"
#define HOST_NAME   "192.168.1.18"
#define HOST_PORT   (4126)
 */
#define SSID        "G808"
#define PASSWORD    "12345678"
#define HOST_NAME   "192.168.43.170"
#define HOST_PORT   (4126)


#include <Wire.h>
#include <AM2321.h>
#include <Weather.h>
#include <math.h>
#include <FreqCounter.h>
#include <LPS331.h>

LPS331 ps;
AM2321 ac;

#include "ESP8266.h"
#include <SoftwareSerial.h>



SoftwareSerial mySerial(3, 2); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

#define LED_PIN 13

float temperature;
float humidity;
float dewpoint;
int itemp;
int ihum;
int idew;
int ipress;
long int freq;
long int lux;
int psflag=0;
float pressure;
long loopcount=0;

void xWifiSetup(void);

void setup(void)
{

	Wire.begin();
	Serial.begin(9600);  
	if (!ps.init())
	{
		Serial.println("Failed to autodetect pressure sensor!");
	}else{
		Serial.println("LPS331 found!");
		ps.enableDefault();
		psflag=1;
	}

	//LEDピンを出力に
	pinMode(LED_PIN, OUTPUT);
	//xWifiSetup();

}


void xWifiSetup(void)
{

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
	if(wifi.setAutoConnect(1)){
		Serial.print("auto connect ok\r\n");
	}else{
		Serial.print("auto connect ng\r\n");
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
	char buf[188];


        //wifiへの接続/切断
        //一定回数ごとにAPへの接続と切断を行うことによってロバストにする
        if((loopcount&0xff)==0){
          xWifiSetup();
        }
        loopcount++;

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

	//気圧取得
	if(psflag==1){
		pressure = ps.readPressureMillibars();
	}

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
	Serial.print(",Presser[hp],");
	Serial.print(pressure);
	Serial.print("\r\n");

	//wifiに出力
	// sprintf(buf,"id,%d,name,%s,time,%d,temp,%d,hum,%d,dew,%d,lux,%d,press,%d\r\n"
	//   ,kID,kNAME,(int)(millis()/1000),(int)(temperature*10),(int)(humidity*10),(int)(dewpoint*10),lux,(int)(pressure*10));
	itemp=(int)(temperature*10);
	ihum=(int)(humidity*10);
	idew=(int)(dewpoint*10);
	ipress=(int)(pressure*10);

	sprintf(buf,"id,%d,name,%s,count,%ld,temp,%d,hum,%d,dew,%d,lux,%ld,press,%d\r\n"
			,kID,kNAME,loopcount,itemp,ihum,idew,lux,ipress);
	int txresult=wifi.send((const uint8_t*)buf, strlen(buf));
	Serial.print(buf);
	//Serial.println(wifi.getNowConecAp());


	delay(30000);


}
