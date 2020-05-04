#include <clickButton.h>
#include <neopixel.h>

#define LED_START_FOOD 0
#define LED_START_POOP 3
#define LED_START_WATER 6

#define PIXEL_PIN D8
#define PIXEL_COUNT 10
#define PIXEL_TYPE WS2812B

#define PIN_BUTTON_POOP 3
#define PIN_BUTTON_WATER 4
#define PIN_BUTTON_FOOD 5

#define WATER_SECONDS 24 * 7 * 1 * 60 * 60
#define FOOD_SECONDS 24 * 7 * 2 * 60 * 60
#define POOP_WARN_SECONDS 48 * 60 * 60
#define POOP_SECONDS 60 * 60 * 60

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
ClickButton buttonPoop(PIN_BUTTON_POOP, LOW, CLICKBTN_PULLUP);
ClickButton buttonFood(PIN_BUTTON_FOOD, LOW, CLICKBTN_PULLUP);
ClickButton buttonWater(PIN_BUTTON_WATER, LOW, CLICKBTN_PULLUP);

int timePoop;
int timeWater;
int timeFood;

// bool success = Particle.function("funcKey", funcName);

int setPoopTime(String timestamp) {
    timePoop = timestamp.toInt();
    return 0;
}

int setWaterTime(String timestamp) {
    timeWater = timestamp.toInt();
    return 0;
}

int setFoodTime(String timestamp) {
    timeFood = timestamp.toInt();
    return 0;
}

void set_led_color(uint8_t start, uint32_t color) {
    for (uint8_t p = 0; p < 3; p += 1) {
        strip.setPixelColor(start + p, color);
    }
    strip.show();
}

void handlePoopWarning() {
    set_led_color(LED_START_POOP, strip.Color(255, 255, 0));
}

void handlePoop() {
    set_led_color(LED_START_POOP, strip.Color(255, 0, 0));
}

void handleWater() {
    set_led_color(LED_START_WATER, strip.Color(0, 50, 255));
}

void handleFood() {
    set_led_color(LED_START_FOOD, strip.Color(0, 255, 0));
}

void setup() {
    Particle.variable("time.poop", timePoop);
    Particle.variable("time.water", timeWater);
    Particle.variable("time.food", timeFood);

    Particle.function("setPoopTime", setPoopTime);
    Particle.function("setWaterTime", setWaterTime);
    Particle.function("setFoodTime", setFoodTime);

    pinMode(PIN_BUTTON_POOP, INPUT_PULLUP);
    pinMode(PIN_BUTTON_WATER, INPUT_PULLUP);
    pinMode(PIN_BUTTON_FOOD, INPUT_PULLUP);

    strip.begin();
    strip.show();

    set_led_color(LED_START_POOP, strip.Color(255, 255, 255));
    set_led_color(LED_START_FOOD, strip.Color(255, 255, 255));
    set_led_color(LED_START_WATER, strip.Color(255, 255, 255));

    Particle.publish("boot", PRIVATE);
}

void loop() {
    int currentTime = Time.now();

    buttonPoop.Update();
    if (buttonPoop.clicks == 1) {
        Particle.publish("button/poop", PRIVATE);
        timePoop = currentTime;
    }

    buttonFood.Update();
    if (buttonFood.clicks == 1) {
        Particle.publish("button/food", PRIVATE);
        timeFood = currentTime;
    }

    buttonWater.Update();
    if (buttonWater.clicks == 1) {
        Particle.publish("button/water", PRIVATE);
        timeWater = currentTime;
    }

    if (currentTime - timePoop >= POOP_SECONDS) {
        handlePoop();
    } else if (currentTime - timePoop >= POOP_WARN_SECONDS) {
        handlePoopWarning();
    } else {
        set_led_color(LED_START_POOP, strip.Color(0, 0, 0));
    }

    if (currentTime - timeWater >= WATER_SECONDS) {
        handleWater();
    } else {
        set_led_color(LED_START_WATER, strip.Color(0, 0, 0));
    }

    if (currentTime - timeFood >= FOOD_SECONDS) {
        handleFood();
    } else {
        set_led_color(LED_START_FOOD, strip.Color(0, 0, 0));
    }

    delay(50);

}
