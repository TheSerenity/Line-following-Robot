/*
 * Valid test data "100000001000000000010010";
 */
// Definitions
#define CHECKSUM_SIZE 6
#define DATA_MIN 0
#define DATA_MAX 18
#define DATA_TOTAL 24 // DATA_MAX + CHECKSUM_SIZE
#define DATA_ID_MIN 0
#define DATA_ID_MAX 7
#define DATA_DISTANCE_MIN 8
#define DATA_DISTANCE_MAX 17

// Vars
String inputString = "";
String ChecksumInput = "";
String Checksum = "";
float SourceID, Distance;
bool stringComplete = 0;
bool Polynomial[7] = {1, 0, 1, 1, 0, 0, 1}; // CRC-6-DARC, polynomial 0x19 (1 011001, x^6 + x^4 + x^3 + 1)
bool XOR[DATA_TOTAL];
bool Tmp[DATA_TOTAL];

// Function prototype
void CRC_6_DARC(String data);

void setup() {
  Serial.begin(4800);
}

void loop() {
  
  // If received info passed basic validation
  if(stringComplete) {

    // Calculate checksum of inputString
    CRC_6_DARC(inputString.substring(DATA_MIN, DATA_MAX));

    // Read the checksum of inputString
    for(int i = 0; i < CHECKSUM_SIZE; i++) ChecksumInput += inputString[i + DATA_MAX];

    // Compare calculated checksum with read checksum from inputString
    if(Checksum == ChecksumInput)
    {
      // Assign values to SourceID and Distance, converted from binary string to decimal
      for(int i = DATA_ID_MIN;       i<= DATA_ID_MAX;       i++) SourceID += (inputString[i] == '1' ?pow(2, DATA_ID_MAX - i)       :0);
      for(int i = DATA_DISTANCE_MIN; i<  DATA_DISTANCE_MAX; i++) Distance += (inputString[i] == '1' ?pow(2, DATA_DISTANCE_MAX - i) :0);
      
      Serial.println("Received: " + inputString);                           // Print info received (binary)
      Serial.println("SourceID: " + String((int)round(SourceID)));          // Print Source ID
      Serial.println("Distance: " + String((int)round(Distance)) + " cm\n");  // Print Distance
    }
    // Checksums do not match
    else Serial.println("CRC-6-DARC failed: Invalid data\n" + Checksum + " is NOT equal to " + ChecksumInput + "\n");
  
    // Reset vars for new string
    inputString = "", Checksum = "", ChecksumInput = "";
    SourceID = 0, Distance = 0, stringComplete = 0;
  }
}

void serialEvent() {
  
  // Wait for serial data
  while(Serial.available())
  {
    // Read a byte
    char Received = (char)Serial.read();
    
    // Add byte to string
    // Must be boolean or '\n'
    if ((Received == '0') || (Received == '1') || (Received == '\n')); // This char is OK
    else return;
    
    inputString += String(Received);
    //if(inputString.length() > DATA_TOTAL) return;

    // If byte is a newline '\n', set stringComplete to 1
    //if(Received == '\n') stringComplete = 1;
    if(inputString.length() == DATA_TOTAL) stringComplete = 1;
  }
}

void CRC_6_DARC(String data) {

  // Assign data to XOR[]
  memset(XOR, 0, sizeof(XOR));
  for(int d = DATA_MIN; d < DATA_MAX; d++) XOR[d] = (data[d] == '0' ?0:1);

  // For each character in data string
  for(int c = DATA_MIN; c < DATA_MAX; c++)
  {
    if(XOR[c])
    {
      // Clear Tmp and set it to Polynomial at the first 1 in XOR from left to right
      memset(Tmp, 0, sizeof(Tmp));
      for(int p = 0; p < sizeof(Polynomial); p++) Tmp[p + c] = Polynomial[p];

      // XOR Polynomial with XOR at each 1's
      for(int x = 0; x < sizeof(XOR); x++) XOR[x] = Tmp[x] ^ XOR[x];
    }
  }

  // Assign checksum of XOR to Checksum
  for(int i = 0; i < CHECKSUM_SIZE; i++) Checksum += (XOR[i + data.length()] == 1 ?1:0);
}

