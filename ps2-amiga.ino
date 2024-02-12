
#include "ps2.hh"

void setup()
{
    Serial.begin(115200);
    ps2_fsm.begin();
}

void loop()
{
    char s[80];
    if (ps2_frame.available)
    {
        uint8_t key = ps2_frame.key;
        ps2_frame.available = false;

        sprintf(s, "Received key: 0x%x\n", key);
        Serial.println(s);
    }
}