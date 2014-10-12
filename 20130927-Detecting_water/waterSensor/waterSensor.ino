int analogPin = 1; // level sensor connected to an analog port
int led = 12; // Piranha LED connected to digital port 12
int val = 0; // define the variable val initial value is 0
int data = 0; // define a variable data initial value is 0
void setup ()
{
	pinMode(led, OUTPUT); // define led pin as an output
	Serial.begin(9600); // set the baud rate to 9600
}
void loop ()
{
	val = analogRead(analogPin); // read the analog value to give the variable val
	if (val> 600) { // determine the variable val is greater than 700
		digitalWrite(led, HIGH); // variable val is greater than 700, the light Piranha LED
	}
	else {
		digitalWrite(led, LOW); // variable val is less than 700, the light goes off piranha
	}
	data = val; // variable val assigned to the variable data
	Serial.println(data); // serial print variable data
	delay(100);
}
