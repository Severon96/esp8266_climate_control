#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

const uint32_t kBaudRate = 115200;
const uint16_t kRecvPin = 14;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 15;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  // put your setup code here, to run once:
  Serial.printf("Setting up everything...");
#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#elif ARDUINO_USB_CDC_ON_BOOT
  Serial.begin(kBaudRate);
#else  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif  // ESP8266
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  // Perform a low level sanity checks that the compiler performs bit field
  // packing as we expect and Endianness is as we expect.
  assert(irutils::lowLevelSanityCheck() == 0);
  
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver
}

void loop() {
  // put your main code here, to run repeatedly:
if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    // Display the basic output of what we found.
    Serial.println("### BEGIN HUMAN READABLE BASIC BEGIN ###");
    Serial.print(resultToHumanReadableBasic(&results));
    Serial.println("### BEGIN HUMAN READABLE BASIC END ###");

    Serial.println("### BEGIN HEXADECIMAL BEGIN ###");
    Serial.println(resultToHexidecimal(&results));
    Serial.println("### BEGIN HEXADECIMAL END ###");
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    Serial.println("### BEGIN LEGACY TIMING INFO BEGIN ###");
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    Serial.println("### END LEGACY TIMING INFO END ###");
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    Serial.println("### BEGIN NO LEGACY TIMING INFO BEGIN ###");
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println("### END NO LEGACY TIMING INFO END ###");
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}
