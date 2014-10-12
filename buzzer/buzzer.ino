int light = 0;
int buzzer = 9;
int lowHz = 500;
int highHz = 125;

void setup() {
  pinMode(light, INPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
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
  Serial.println(d);
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
  while (1) {
    int lightVal = analogRead(light);
    buzz(lightVal); 
  }
}
