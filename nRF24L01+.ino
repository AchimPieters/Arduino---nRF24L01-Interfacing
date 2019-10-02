#include <SPI.h>
#include "RF24.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 (CE & CS)

RF24 radio(9,10);

// sets the role of this unit in hardware.  Connect to GND to be the 'led' board receiver
// Leave open to be the 'remote' transmitter
const int role_pin = A4;

const uint8_t button_pins[] = { 2,3,4,5,6,7 };
const uint8_t num_button_pins = sizeof(button_pins);

const uint8_t led_pins[] = { 2,3,4,5,6,7 };
const uint8_t num_led_pins = sizeof(led_pins);

const uint64_t pipe = 0xE8E8F0F0E1LL;

typedef enum { role_remote = 1, role_led } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Remote", "LED Board"};

role_e role;

uint8_t button_states[num_button_pins];
uint8_t led_states[num_led_pins];

void setup(void)
{

        pinMode(role_pin, INPUT);
        digitalWrite(role_pin,HIGH);
        delay(20);

        if ( digitalRead(role_pin) )
                role = role_remote;
        else
                role = role_led;

        Serial.begin(115200);
        printf_begin();
        printf("\n\rRF24/examples/led_remote/\n\r");
        printf("ROLE: %s\n\r",role_friendly_name[role]);

        radio.begin();
        radio.setChannel(100);
        radio.setPALevel(RF24_PA_MIN);
        radio.setDataRate(RF24_250KBPS);

        if ( role == role_remote )
        {
                radio.openWritingPipe(pipe);
        }
        else
        {
                radio.openReadingPipe(1,pipe);
        }
        if ( role == role_led )
                radio.startListening();
        radio.printDetails();
        if ( role == role_remote )
        {
                int i = num_button_pins;
                while(i--)
                {
                        pinMode(button_pins[i],INPUT);
                        digitalWrite(button_pins[i],HIGH);
                }
        }
        if ( role == role_led )
        {
                int i = num_led_pins;
                while(i--)
                {
                        pinMode(led_pins[i],OUTPUT);
                        led_states[i] = HIGH;
                        digitalWrite(led_pins[i],led_states[i]);
                }
        }
}
void loop(void)
{
        if ( role == role_remote )
        {
                int i = num_button_pins;
                bool different = false;
                while(i--)
                {
                        uint8_t state = !digitalRead(button_pins[i]);
                        if ( state != button_states[i] )
                        {
                                different = true;
                                button_states[i] = state;
                        }
                }
                if ( different )
                {
                        printf("Now sending...");
                        bool ok = radio.write( button_states, num_button_pins );
                        if (ok)
                                printf("ok\n\r");
                        else
                                printf("failed\n\r");
                }
                delay(20);
        }
        if ( role == role_led )
        {
                if ( radio.available() )
                {
                        while (radio.available())
                        {
                                radio.read( button_states, num_button_pins );
                                printf("Got buttons\n\r");
                                int i = num_led_pins;
                                while(i--)
                                {
                                        if ( button_states[i] )
                                        {
                                                led_states[i] ^= HIGH;
                                                digitalWrite(led_pins[i],led_states[i]);
                                        }
                                }
                        }
                }
        }
}
