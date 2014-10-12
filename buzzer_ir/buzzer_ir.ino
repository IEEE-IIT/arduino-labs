#include <IRremote.h>

int RECV_PIN = 11;
int light = 0;
int buzzer = 9;
int lowHz = 500;
int highHz = 125;
int freq = 100;
boolean on = true;
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A

long remote[][3] = {
{0xFFA25D, 0xFF629D, 0xFFE21D},
{0xFF22DD, 0xFF02FD, 0xFFC23D},
{0xFFE01F, 0xFFA857, 0xFF906F},
{0xFF6897, 0xFF9867, 0xFFB04F},
{0xFF30CF, 0xFF18E7, 0xFF7A85},
{0xFF10EF, 0xFF38C7, 0xFF5AA5},
{0xFF42BD, 0xFF4AB5, 0xFF52AD}
};

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

float get_delay(int hz) {
  return (1000 / hz) / 2;
}

void buzz(int v) {
  int h, i;
  if (v == HIGH) {
    h = highHz;
  } else {
    h = lowHz;
  }
  
  float d = get_delay(v);
  //Serial.println(d);
  if (d > 0) {
    digitalWrite(buzzer, HIGH);
    delay(d);
    digitalWrite(buzzer, LOW);
    delay(d);
  } else {
    digitalWrite(buzzer, LOW);
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    //Serial.println(remote[0][0],HEX);
    if (results.value == remote[2][1]) freq -= 10;
    else if (results.value == remote[2][2]) freq += 10;
    else if (results.value == remote[0][0]) on = !on;
    else if (results.value == remote[3][0]) freq = tones[0];
    else if (results.value == remote[4][0]) freq = tones[1];
    else if (results.value == remote[4][1]) freq = tones[2];
    else if (results.value == remote[4][2]) freq = tones[3];
    else if (results.value == remote[5][0]) freq = tones[4];
    else if (results.value == remote[5][1]) freq = tones[5];
    else if (results.value == remote[5][2]) freq = tones[6];
    else if (results.value == remote[6][0]) freq = tones[7];
    else if (results.value == remote[6][1]) freq = tones[8];
    else if (results.value == remote[6][2]) freq = tones[9];
    irrecv.resume(); // Receive the next value
  }
  Serial.println(freq);
  if (on) buzz(freq);
  else buzz(0);
}
