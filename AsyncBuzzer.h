/* AsyncBuzzer.h - Non-blocking buzzer control with beeps, pulses and tone sequences
Copyright (c) 2025 by breadbaker
MIT License */
#pragma once
#include <Arduino.h>
#if __has_include(<config.h>)
#include <config.h> // Include a global config if available
#endif

#ifndef BUZZER_NOUSE_SD
#define BUZZER_USE_SD // Enable SD card support in AsyncBuzzer (requires SDCard library)
#endif

#ifdef SERIAL_OUT_DISABLE
#define BUZZER_SERIAL_DISABLE // Disable Serial output
#endif

#if !defined(BUZZER_SERIAL_DISABLE)
#undef SERIAL // Serial port mapping
#ifdef SERIAL_PORT_USBVIRTUAL
#define SERIAL SERIAL_PORT_USBVIRTUAL
#else
#define SERIAL Serial // Use default Serial port
#endif
#endif

#ifndef BUZZER_PIN
#define BUZZER_PIN 255 // Pin for buzzer (255 means no pin defined)
#endif
#ifndef BUZZER_ACK_FREQ
#define BUZZER_ACK_FREQ 800 // Frequency for acknowledgment beep
#endif
#ifndef BUZZER_ACK_DURATION
#define BUZZER_ACK_DURATION 30 // Duration for acknowledgment beep in milliseconds
#endif
#ifndef BUZZER_ERR_FREQ
#define BUZZER_ERR_FREQ 1000 // Frequency for error beep
#endif
#ifndef BUZZER_ERR_DURATION
#define BUZZER_ERR_DURATION 300 // Duration for error beep in milliseconds
#endif
#ifndef BUZZER_PULSE_INTERVAL
#define BUZZER_PULSE_INTERVAL 50 // Interval between pulses in milliseconds
#endif
#ifndef BUZZER_MAX_MELODY_TONES
#define BUZZER_MAX_MELODY_TONES 30 // Maximum number of tones in a melody
#endif
#ifndef BUZZER_MAX_PATTERN_PULSES
#define BUZZER_MAX_PATTERN_PULSES 20 // Maximum number of pulses in a pattern
#endif

// Flags:
#define BUZ_NONE 0x00
#define BUZ_BEEP 0x01
#define BUZ_PULSE 0x02
#define BUZ_FORCE 0x08
#define BUZ_SILENT 0x80

namespace AsyncBuzzer
{
    struct Tone
    {
        uint16_t frequency;
        uint16_t duration;
        uint16_t rest;
        Tone(uint16_t f = 0, uint16_t d = 0, uint16_t r = BUZZER_PULSE_INTERVAL) : frequency(f), duration(d), rest(r) {}
    };

    struct Config
    {
        uint8_t pin;
        Tone ack;
        Tone err;
        Config(uint8_t p = 255,
               Tone a = Tone(BUZZER_ACK_FREQ, BUZZER_ACK_DURATION, BUZZER_PULSE_INTERVAL),
               Tone e = Tone(BUZZER_ERR_FREQ, BUZZER_ERR_DURATION, BUZZER_PULSE_INTERVAL)) : pin(p), ack(a), err(e) {}
    };

    struct Pulse
    {
        uint8_t pulses;
        uint16_t frequency;
        uint16_t duration;
        uint16_t interval;
        uint32_t last;
        bool active;
        Pulse(uint8_t p = 0, uint16_t f = 0, uint16_t d = 0, uint16_t i = 0, uint32_t t = 0, bool a = false) : pulses(p), frequency(f), duration(d), interval(i), last(t), active(a) {}
    };

    struct Pattern
    {
        Pulse *pulses;
        uint8_t count;
        uint8_t current;
        bool active;
        bool repeat;
        uint16_t pulseDelay;   // Delay between pulses in milliseconds
        uint32_t lastPulseEnd; // When the last pulse finished
        bool waitingForDelay;  // True when waiting for delay between pulses
        Pattern(Pulse *p = nullptr, uint8_t c = 0, uint8_t cur = 0, bool a = false, bool r = false, uint16_t pd = 300)
            : pulses(p), count(c), current(cur), active(a), repeat(r), pulseDelay(pd), lastPulseEnd(0), waitingForDelay(false) {}
    };

    struct Melody
    {
        Tone *tones;        // Array of tone definitions
        uint8_t count;      // Number of tones in melody
        uint8_t current;    // Current tone index
        bool active;        // Melody active state
        bool repeat;        // Should melody repeat when finished
        uint32_t toneStart; // When current tone started
        bool playingTone;   // True when playing tone, false during rest
        Melody(Tone *t = nullptr, uint8_t c = 0, uint8_t cur = 0, bool a = false, bool r = false)
            : tones(t), count(c), current(cur), active(a), repeat(r), toneStart(0), playingTone(false) {}
    };

    bool setup(Config conf, uint8_t flags = BUZ_NONE);
    bool setup(uint8_t pin = BUZZER_PIN, uint8_t flags = BUZ_NONE);
    bool update();
    Config getConfig();
    Config setConfig(Config conf, uint8_t flags = BUZ_NONE);
    void printConfig(const String &message = "");

    void beep(uint16_t frequency = getConfig().ack.frequency, uint16_t duration = getConfig().ack.duration);
    void pulse(uint8_t count = 3, uint16_t frequency = getConfig().ack.frequency, uint16_t duration = getConfig().ack.duration, uint16_t interval = getConfig().ack.rest);
    void pulseBlocking(uint8_t count = 3, uint16_t frequency = getConfig().ack.frequency, uint16_t duration = getConfig().ack.duration, uint16_t interval = getConfig().ack.rest);

    void pattern(Pulse *pulses, uint8_t count, bool repeat = false, uint16_t pulseDelay = 300);
    void patternBlocking(Pulse *pulses, uint8_t count, bool repeat = false, uint16_t pulseDelay = 300);
    bool isPatternActive();
    void stopPattern();

    uint8_t loadPattern(const String &path, Pulse *pulses, uint8_t flags = BUZ_NONE);

    void melody(Tone *tones, uint8_t count, bool repeat = false);
    void melodyBlocking(Tone *tones, uint8_t count, bool repeat = false);
    bool isMelodyActive();
    void stopMelody();

    uint8_t loadTones(const String &path, Tone *tones, uint8_t flags = BUZ_NONE);
    bool playFile(const String &path, uint8_t flags = BUZ_NONE);
}