/**************************************************************************/
/*! 
    @file     readntag203.pde
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    This example will wait for any NTAG203 or NTAG213 card or tag,
    and will attempt to read from it.

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout 
      ----> https://www.adafruit.com/products/364
 
    Check out the links above for our tutorials and wiring diagrams 
    These chips use SPI or I2C to communicate.

    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_SS   (10)
// Use this line for a breakout with a hardware SPI connection. 
Adafruit_PN532 nfc(PN532_SS);
int tagdetected = 1;

void setup(void) {
  /*
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  */
  Serial.begin(115200);
  Serial.println("Hello! NFC READER HERE!");
  pinMode(4, OUTPUT);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  if (tagdetected > 0)
    digitalWrite(4, HIGH);
  else
    digitalWrite(4, LOW);
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  
  readTag();
  if (tagdetected > 0)
    digitalWrite(4, HIGH);
  else
    digitalWrite(4, LOW);
  delay(3000);
}

void readTag(){
  uint8_t success;
  uint8_t tagFound = 0;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  String content = "";
  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,5000);
  
  if (success) {
    tagFound = 1;
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 7)
    {
      uint8_t data[32];
      
      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");    
      
      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:
      
      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225      

      for (uint8_t i = 0; i < 42; i++) 
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        
        // Display the current page number
        //Serial.print("PAGE ");
        if (i < 10)
        {
          //Serial.print("0");
          //Serial.print(i);
        }
        else
        {
          //Serial.print(i);
        }
        //Serial.print(": ");

        // Display the results, depending on 'success'
        if (success) 
        {
          // Dump the page data
          //nfc.PrintHexChar(data, 4);
          content += printNFC(data);
        }
        else
        {
          Serial.println("Unable to read the requested page!");
        }
        delay(10);
      }      
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
    Serial.println(content);
    if (content.indexOf("AW11_MR2TAG_SW9913") > 0){
      tagdetected = 0;
      Serial.println("TAG FOUND!!!!!");
    }else{
      tagdetected = 1;
      Serial.println("WRONG TAG FOUND!!");
    }
    // Wait a bit before trying again
    Serial.println("\n\nSend a character to scan another tag!");
    /*
    Serial.flush();
    while (!Serial.available());
    while (Serial.available()) {
    Serial.read();
    }
    */
    Serial.flush();
  }

  if (tagFound<1){
    tagdetected = 1;
    Serial.println("No Tag Found!!");
  }


}

String printNFC(byte * data){
  uint32_t szPos;
  String inside = "";
  for (szPos=0; szPos < 4; szPos++)
  {
    if (!(data[szPos] <= 0x1F))
      inside += (char)data[szPos];
  }
  return inside;
  
}
