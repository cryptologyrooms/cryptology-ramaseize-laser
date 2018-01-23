/* Arduino Library Includes */

#include "TaskAction.h"
#include "Adafruit_NeoPixel.h"

static const uint8_t RELAY_PIN = 2;

static const uint8_t DETECTOR_1_PIN = 4;
static const uint8_t DETECTOR_2_PIN = 5;

static const uint8_t NEOPIXEL_PIN = 8;
static const uint8_t NUMBER_OF_PIXELS = 5;

static Adafruit_NeoPixel s_leds = Adafruit_NeoPixel(NUMBER_OF_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static uint8_t s_detector_counter[2] = {0,0};
static int s_trigger_count_ms = 0;
static const uint16_t DEBOUNCE_MS = 10;
static const uint16_t TRIGGER_AFTER_MS = 1500;

static const uint16_t LED_TURN_ON_MS[5] = {0, 300, 600, 900, 1200};
static const uint16_t LED_FADE_IN_MS = 500;
static const uint16_t FULL_BRIGHTNESS_R = 255;
static const uint16_t FULL_BRIGHTNESS_G = 0;
static const uint16_t FULL_BRIGHTNESS_B = 0;

static void set_rgb(int * rgb, int r, int g, int b)
{
	rgb[0] = r; rgb[1] = g; rgb[2] = b;
}

static void get_led_level(int led, int ms, int * rgb)
{
	int turn_on_ms = LED_TURN_ON_MS[led];

	if (ms < turn_on_ms)
	{
		set_rgb(rgb, 0, 0, 0);
	}
	else if (ms > (turn_on_ms+LED_FADE_IN_MS))
	{
		set_rgb(rgb, FULL_BRIGHTNESS_R, FULL_BRIGHTNESS_G, FULL_BRIGHTNESS_B);
	}
	else
	{
		set_rgb(rgb,
			((ms - turn_on_ms) * (uint32_t)FULL_BRIGHTNESS_R) / LED_FADE_IN_MS,
			((ms - turn_on_ms) * (uint32_t)FULL_BRIGHTNESS_G) / LED_FADE_IN_MS,
			((ms - turn_on_ms) * (uint32_t)FULL_BRIGHTNESS_B) / LED_FADE_IN_MS
		);
	}
}

static void set_all_leds(Adafruit_NeoPixel& leds, int r, int g, int b)
{
	for(int i=0; i < NUMBER_OF_PIXELS; i++)
	{
		leds.setPixelColor(i, r, g, b);
	}
	leds.show();
}

static void set_led_levels(Adafruit_NeoPixel& leds, int ms)
{
	int rgb[3];

	for(int i = 0; i < NUMBER_OF_PIXELS; i++)
	{
		get_led_level(i, ms, rgb);
		leds.setPixelColor(i, rgb[0], rgb[1], rgb[2]);
	}
	s_leds.show();
}

static void detector_task_fn(TaskAction * this_task)
{
	if ((digitalRead(DETECTOR_1_PIN) == HIGH) && (digitalRead(DETECTOR_2_PIN) == HIGH))
	{
		if (s_trigger_count_ms < TRIGGER_AFTER_MS)
		{
			s_trigger_count_ms += DEBOUNCE_MS;
		}
	}
	else
	{
		s_trigger_count_ms = 0;
	}
}
static TaskAction s_detector_task(detector_task_fn, DEBOUNCE_MS, INFINITE_TICKS);

static void relay_trigger_task_fn(TaskAction * this_task)
{
	digitalWrite(RELAY_PIN, s_trigger_count_ms >= TRIGGER_AFTER_MS ? HIGH : LOW);
}
static TaskAction s_relay_trigger_task(relay_trigger_task_fn, 100, INFINITE_TICKS);

static void neopixel_task_fn(TaskAction * this_task)
{
	if (s_trigger_count_ms == 0)
	{
		s_leds.clear();
		s_leds.show();
	}
	else if (s_trigger_count_ms >= TRIGGER_AFTER_MS)
	{
		set_all_leds(s_leds, 0, 255, 0);
	}
	else
	{
		set_led_levels(s_leds, s_trigger_count_ms);
	}
}
static TaskAction s_neopixel_task(neopixel_task_fn, 20, INFINITE_TICKS);

void setup()
{
	pinMode(RELAY_PIN, OUTPUT);

	pinMode(DETECTOR_1_PIN, INPUT);
	pinMode(DETECTOR_2_PIN, INPUT);

	s_leds.begin();
	s_leds.show();
}

void loop()
{
	s_detector_task.tick();
	s_relay_trigger_task.tick();
	s_neopixel_task.tick();
}
