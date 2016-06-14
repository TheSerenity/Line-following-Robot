// Settings
#define Source_ID 10000000
#define EveryXHour 6
#define TIMEOUT 5000 // 5 ms

// Do NOT edit
#define TO_MSEC 1000
#define TO_SEC 1000
#define TO_MIN 60
#define TO_HOUR 60
#define AN_HOUR 1
#define A_DAY 24
#define TO_CM 1000
#define ON 1
#define OFF 0
#define SPEED_OF_SOUND 340 // 340 m/s at 15°C
#define DELAY_US 100

// Pins
const char Receiver = 2;
const char Transmitter = 3;

// Vars
long Timer = OFF;
long TimerTransmitReceive = OFF;
bool TimerTRStart = OFF;
long TimePrevious = OFF;
long TimePassed = OFF;
bool Transmitting = OFF;

// CRC-6-DARC vars, polynomial 0x19 (1 011001, x^6 + x^4 + x^3 + 1)
bool Polynomial[7] = {1, 0, 1, 1, 0, 0, 1};
bool Checksum[6];
bool XOR[24];
bool Tmp[24];

// Function prototype
void CRC_6_DARC(bool data);

void setup() {

  pinMode(Receiver, INPUT);
  pinMode(Transmitter, OUTPUT);
  Serial.begin(4800);

}

void loop() {
  
  // Time passed [seconds]
  // Hours: long TimePassed = (Timer / TO_MSEC / TO_SEC / TO_MIN / TO_HOUR);
  long TimePassed = (Timer / TO_MSEC / TO_SEC);
  
  /* 
   * Transmit at given time (EveryXHour) 
   */
  // If one hour has passed
  if(TimePassed >= AN_HOUR) 
  {
    long Remainder = TimePassed % EveryXHour;
    
    // If Remainder is 0, and TimePassed is different than TimePrevious
    if((TimePrevious != TimePassed) && !Remainder)
    {
      // Start transmitting
      Transmitting = ON;
      TimerTRStart = ON;
      digitalWrite(Transmitter, HIGH);
      TimePrevious = TimePassed;
    }
  }
  
  /* 
   * Wait until receiving a signal 
   */
  if(Transmitting && digitalRead(Receiver))
  {
    // Calculate distance [cm] to object
    long Distance = ((SPEED_OF_SOUND * TimerTransmitReceive) / 2) / TO_CM;
    
    // Assign data to Data
    String Data = String(Source_ID) + String(Distance);
    
    // Print source ID, distance and checksum
    CRC_6_DARC(Data); // Calculate checksum of Data
    Serial.print(Source_ID, BIN);
    Serial.print(Distance, BIN);
    for(int i=0; i<sizeof(Checksum); i++) Serial.print(Checksum[i]);
    Serial.print("\n");

    // Reset vars
    digitalWrite(Transmitter, LOW);
    TimerTransmitReceive = OFF;
    Transmitting = OFF;
    TimerTRStart = OFF;
  }
  
  // Time of day
  // Hours: Timer / TO_SEC / TO_MIN / TO_HOUR
  if(Timer / TO_MSEC / TO_SEC == A_DAY) Timer = OFF; // Reset
  else Timer += DELAY_US;

  // Timer between transmitting and receiving
  if(TimerTRStart) TimerTransmitReceive += DELAY_US;

  // Timeout for transmitting, if nothing is returned within given time.
  if(TimerTransmitReceive >= TIMEOUT)
  {    
    // Reset vars
    digitalWrite(Transmitter, LOW);
    TimerTRStart = OFF;
    TimerTransmitReceive = OFF;
    Transmitting = OFF;
  }
  
  delayMicroseconds(DELAY_US);
}

void CRC_6_DARC(String data) {

  // Assign data to XOR[]
  memset(XOR, 0, sizeof(XOR));
  for(int d=0; d<sizeof(data); d++) XOR[d] = data[d];

  // For each number in var data
  for(int i=0; i<sizeof(data); i++)
  {
    // If the number is 1
    if(XOR[i]) {

      // Clear Tmp and set it to Polynomial at the first 1 in XOR from left to right
      memset(Tmp, 0, sizeof(Tmp));
      for(int d=0; d<sizeof(Polynomial); d++) Tmp[d+i] = Polynomial[d];

      // XOR Polynomial with XOR
      for(int d=0; d<sizeof(XOR); d++) XOR[d] = Tmp[d] ^ XOR[d];
    }
  }
  
  // Assign last 6 bits of XOR to Checksum
  for(int i=0; i<sizeof(Checksum); i++) Checksum[i] = XOR[i + sizeof(data)];
}
