#include <Arduino.h>
#include <hmac.h>

static char g_msg[11] = {0};
static char g_key[16] = {0};

#define SAMPLE_SIZE 5

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println("Starting test");
    Serial.flush();

    unsigned long durationSum = 0;
    for (int i = 0; i < SAMPLE_SIZE; ++i)
    {
        const unsigned long startTime = millis();
        hmac::compute(g_msg, sizeof(g_msg), g_key, sizeof(g_key));
        const unsigned long duration = millis() - startTime;
        Serial.print("Trial ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(duration);
        durationSum += duration;
    }

    // compute average
    const unsigned long avg = durationSum / SAMPLE_SIZE;
    Serial.print("Average: ");
    Serial.println(avg);
}