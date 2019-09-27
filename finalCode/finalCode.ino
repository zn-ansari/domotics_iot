
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
IPAddress server(192, 168, 1, 101);
EthernetClient ethClient;
PubSubClient client(ethClient);

//ultrasonic
int trig = 9;
int echo = 8;
int LED =7;
float t = 0, h = 0, hp = 0;
//energy
float sample1=0; // for voltage
float sample2=0; // for current
float voltage=0.0;
float val; // current callibration
float actualval; // read the actual current from ACS 712
float amps=0.0;
float totamps=0.0; 
float avgamps=0.0;
float amphr=0.0;
float watt=0.0;
float enrgy=0.0;

char j[5];

void setup()
{
    Serial.begin(9600);
    client.setServer(server,1883);
    client.setCallback(callback);  
    Ethernet.begin(mac);
    pinMode(5,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(2,OUTPUT);
    if(client.connect("catenzClient"))
     {
      Serial.println("connected");
     }
    else 
    {
    Serial.println("not connected");
    Serial.println(client.state());
    }
    client.subscribe("12");
    client.subscribe("31"); 
    client.subscribe("32");
    client.subscribe("33");
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT); 
    pinMode(LED,OUTPUT);
    pinMode(9,OUTPUT);
} 

void loop()
{
     client.loop();
     water();
     energy();
     parking();
     
}

void callback(char* topic, byte* payload, unsigned int length)
{
  int mytopic = tpc2int(topic);
  if(mytopic == 12)
  {
    int msg = pyld2int(payload, length);
    if(msg==121)
    {
    digitalWrite(5, HIGH);
    }
    else
    if(msg==120)
    digitalWrite(5, LOW);
  }
  else if(mytopic == 31)
  {
    int msg = pyld2int(payload, length);
    if(msg==311)
    {
    digitalWrite(4, HIGH);
    }
    else
    if(msg==310)
    digitalWrite(4, LOW);
  }
  else if(mytopic == 32)
  {
    int msg = pyld2int(payload, length);
    if(msg==321)
    {
    digitalWrite(3, HIGH);
    }
    else
    if(msg==320)
    digitalWrite(3, LOW);
  }
  else if(mytopic == 33)
  {
    int msg = pyld2int(payload, length);
    if(msg==331)
    {
    digitalWrite(2, HIGH);
    }
    else
    if(msg==330)
    digitalWrite(2, LOW);
  }
  
}

void water() { 
  // Transmitting pulse
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  // Waiting for pulse
  t = pulseIn(echo, HIGH);
  // Calculating distance 
  h = t/58;
  h = h -0;  // offset correction
  h = 18- h;  // water height, 0 - 50 cm
  hp =  (h/18)*100;  // distance in %, 0-100 %
  // Sending to computer
  char* w = data2char(hp);
  client.publish("11",w);
}

void energy()
{
      for(int i=0;i<150;i++)
  {
    sample1+=analogRead(A2);  //read the voltage from the sensor
    sample2+=analogRead(A3); //read the current from sensor
    delay(2);
  }
   sample1=sample1/150; 
   sample2=sample2/150;
   voltage=4.669*2*sample1/1000;
   val =(5.0*sample2)/1024.0; 
   actualval =val-2.5; // offset voltage is 2.5v 
   long milisec = millis(); // calculate time in milliseconds
   long time=milisec/1000; // convert milliseconds to seconds

   amps =actualval*10; // 100mv/A from data sheet 
   totamps=totamps+amps; // total amps 
   avgamps=totamps/time; // average amps
   amphr=(avgamps*time)/3600;  // amphour
   watt =voltage*amps;    // power=voltage*current
   enrgy=(watt*time)/3600;   //energy in watt hour
   char* e = data2char(enrgy); 
   client.publish("21",e);
}

void parking()
{
  int sensorValue1 = analogRead(A4);
  float voltage1 = sensorValue1 * (5.0 / 1023.0);
  Serial.println(voltage1);
  int sensorValue2 = analogRead(A5);
  float voltage2 = sensorValue2 * (5.0 / 1023.0);
  Serial.println(voltage2);  
  if(voltage1>2&&voltage2>2)
    {
        client.publish("41","both the");
    }
   else if(voltage1>2)
    {
        client.publish("41","A");
    }
    
   else if(voltage2>2)
    {
        client.publish("41","B"); 
    }
   
   else if(voltage1>2)
    {
        client.publish("41","null"); 
    }

}

int tpc2int(char* topic)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int mytopic = atoi (topic);
  Serial.print(mytopic);
  return(mytopic);
}

int pyld2int (byte* payload, unsigned int length)
{
  payload[length] = '\0';
  int msg = atoi( (const char *) payload);
  Serial.print(" = ");
  Serial.println(msg);
  return(msg);
}

char* data2char(float x)
{
  String i = String(x);
  Serial.println(x);
  i.toCharArray(j,i.length()+1);
  return(j);
}
