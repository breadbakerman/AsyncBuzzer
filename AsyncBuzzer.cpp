/* AsyncBuzzer.cpp - Non-blocking buzzer control with beeps, pulses and tone sequences
Copyright (c) 2025 by breadbaker
MIT License */
#include <AsyncBuzzer.h>
#ifdef BUZZER_USE_SD
#if __has_include(<SDCard.h>)
#include <SDCard.h>
#else
#undef BUZZER_USE_SD
#endif
#endif

#define BUZ_LOG_PREFIX ANSI_GRAY "[Buzzer] " ANSI_DEFAULT

namespace AsyncBuzzer
{
    static Config config;
    static Pulse pulseState;
    static Pattern patternState;
    static Melody melodyState;

    bool setup(Config conf, uint8_t flags)
    {
        if (conf.pin == 255 && config.pin != 255)
        {
            noTone(config.pin);
            pinMode(config.pin, INPUT);
            config = Config();
            pulseState = Pulse();
            patternState = Pattern();
            melodyState = Melody();
            return false;
        }
        if (conf.pin == config.pin && !(flags & BUZ_FORCE))
        {
#ifndef BUZZER_SERIAL_DISABLE
            if (!(flags & BUZ_SILENT))
                SERIAL.println(F(BUZ_LOG_PREFIX "Buzzer pin already initialized." ANSI_DEFAULT));
#endif
            return true;
        }

        config = Config(conf.pin);
        pinMode(conf.pin, OUTPUT);
        digitalWrite(conf.pin, LOW);
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            printConfig();
#endif
        if (flags & BUZ_BEEP)
            pulseBlocking(3);
        return true;
    }

    bool setup(uint8_t pin, uint8_t flags)
    {
        return setup(Config(pin), flags);
    }

    bool advancePattern()
    {
        if (!patternState.active || patternState.pulses == nullptr || patternState.count == 0)
        {
            patternState.active = false;
            return false;
        }
        patternState.current++;
        if (patternState.current >= patternState.count)
        {
            if (patternState.repeat)
                patternState.current = 0;
            else
            {
                patternState.active = false;
                return false;
            }
        }
        Pulse nextPulse = patternState.pulses[patternState.current];
        pulseState = Pulse(nextPulse.pulses, nextPulse.frequency, nextPulse.duration, nextPulse.interval, 0, true);
        return true;
    }

    bool update()
    {
        if (pulseState.active && config.pin != 255)
        {
            uint32_t now = millis();
            if (pulseState.pulses > 0)
            {
                if (pulseState.last == 0 || (now - pulseState.last) >= pulseState.interval + pulseState.duration)
                {
                    tone(config.pin, pulseState.frequency, pulseState.duration);
                    pulseState.last = now;
                    pulseState.pulses--;
                }
            }
            else
            {
                pulseState.active = false;
                if (patternState.active)
                {
                    patternState.lastPulseEnd = millis() + pulseState.duration;
                    patternState.waitingForDelay = true;
                }
            }
            return pulseState.last == now;
        }

        if (patternState.active && patternState.waitingForDelay)
        {
            uint32_t now = millis();
            if (now >= patternState.lastPulseEnd)
            {
                uint32_t elapsed = now - patternState.lastPulseEnd;
                if (elapsed >= patternState.pulseDelay)
                {
                    patternState.waitingForDelay = false;
                    advancePattern();
                }
            }
        }
        else if (patternState.active && !pulseState.active && !patternState.waitingForDelay)
            advancePattern();

        if (melodyState.active && config.pin != 255 && !pulseState.active && !patternState.active)
        {
            uint32_t now = millis();
            if (melodyState.current < melodyState.count)
            {
                Tone &currentTone = melodyState.tones[melodyState.current];
                if (melodyState.toneStart == 0)
                {
                    melodyState.toneStart = now;
                    melodyState.playingTone = true;
                    if (currentTone.frequency > 0)
                        tone(config.pin, currentTone.frequency, currentTone.duration);
                }
                else if (melodyState.playingTone)
                {
                    if (now - melodyState.toneStart >= currentTone.duration)
                    {
                        melodyState.playingTone = false;
                        noTone(config.pin);
                    }
                }
                else
                {
                    if (now - melodyState.toneStart >= currentTone.duration + currentTone.rest)
                    {
                        melodyState.current++;
                        melodyState.toneStart = 0;
                    }
                }
            }
            else
            {
                if (melodyState.repeat)
                {
                    melodyState.current = 0;
                    melodyState.toneStart = 0;
                    melodyState.playingTone = false;
                }
                else
                    melodyState.active = false;
            }
        }

        return false;
    }

    Config getConfig()
    {
        return config;
    }

    Config setConfig(Config conf, uint8_t flags)
    {
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            printConfig();
#endif
        return (config = conf);
    }

    void printConfig(const String &message)
    {
#ifndef BUZZER_SERIAL_DISABLE
        SERIAL.print(F(BUZ_LOG_PREFIX));
        if (message.length())
        {
            SERIAL.print(message);
            SERIAL.print(F(" "));
        }
        SERIAL.print(F("Pin: " ANSI_YELLOW));
        SERIAL.print(config.pin);
        SERIAL.print(F(ANSI_DEFAULT "  Ack: " ANSI_YELLOW));
        SERIAL.print(config.ack.frequency);
        SERIAL.print(F(ANSI_DEFAULT "Hz/" ANSI_YELLOW));
        SERIAL.print(config.ack.duration);
        SERIAL.print(F(ANSI_DEFAULT "ms/" ANSI_YELLOW));
        SERIAL.print(config.ack.rest);
        SERIAL.print(F(ANSI_DEFAULT "ms  Err: " ANSI_YELLOW));
        SERIAL.print(config.err.frequency);
        SERIAL.print(F(ANSI_DEFAULT "Hz/" ANSI_YELLOW));
        SERIAL.print(config.err.duration);
        SERIAL.print(F(ANSI_DEFAULT "ms/" ANSI_YELLOW));
        SERIAL.print(config.err.rest);
        SERIAL.println(F(ANSI_DEFAULT "ms" ANSI_DEFAULT));
#endif
    }

    void beep(uint16_t frequency, uint16_t duration)
    {
        if (config.pin == 255)
            return;
        tone(config.pin, frequency, duration);
    }

    void pulse(uint8_t count, uint16_t frequency, uint16_t duration, uint16_t interval)
    {
        if (config.pin == 255 || count == 0)
            return;
        pulseState = Pulse(count, frequency, duration, interval, 0, true);
    }

    void pulseBlocking(uint8_t count, uint16_t frequency, uint16_t duration, uint16_t interval)
    {
        pulse(count, frequency, duration, interval);
        while (pulseState.pulses)
            update();
    }

    void pattern(Pulse *pulses, uint8_t count, bool repeat, uint16_t pulseDelay)
    {
        if (config.pin == 255 || pulses == nullptr || count == 0)
            return;
        patternState.active = false;
        pulseState.active = false;
        patternState = Pattern(pulses, count, 0, true, repeat, pulseDelay);
        Pulse firstPulse = patternState.pulses[0];
        pulseState = Pulse(firstPulse.pulses, firstPulse.frequency, firstPulse.duration, firstPulse.interval, 0, true);
    }

    void patternBlocking(Pulse *pulses, uint8_t count, bool repeat, uint16_t pulseDelay)
    {
        pattern(pulses, count, repeat, pulseDelay);
        while (patternState.active || pulseState.active)
            update();
    }

    bool isPatternActive()
    {
        return patternState.active;
    }

    void stopPattern()
    {
        patternState.active = false;
        pulseState.active = false;
    }

    void melody(Tone *tones, uint8_t count, bool repeat)
    {
        if (config.pin == 255 || tones == nullptr || count == 0)
            return;
        stopMelody();
        melodyState.tones = tones;
        melodyState.count = count;
        melodyState.current = 0;
        melodyState.active = true;
        melodyState.repeat = repeat;
        melodyState.toneStart = 0;
        melodyState.playingTone = false;
    }

    void melodyBlocking(Tone *tones, uint8_t count, bool repeat)
    {
        melody(tones, count, repeat);
        melodyState.repeat = false;
        while (melodyState.active)
        {
            update();
            delay(1);
        }
    }

    bool isMelodyActive()
    {
        return melodyState.active;
    }

    void stopMelody()
    {
        melodyState.active = false;
        if (config.pin != 255)
            noTone(config.pin);
    }

    static uint8_t split(char *input, char **output, uint8_t max_elements)
    {
        for (uint8_t i = 0; i < max_elements; ++i)
            output[i] = nullptr;
        uint8_t count = 0;
        char *ptr = input;
        while (*ptr && count < max_elements)
        {
            // Skip leading spaces
            while (*ptr == ' ')
                ++ptr;
            if (!*ptr)
                break;
            if (*ptr == '"')
            {
                // Quoted token
                ++ptr; // Skip opening quote
                output[count++] = ptr;
                while (*ptr && *ptr != '"')
                    ++ptr;
                if (*ptr == '"')
                {
                    *ptr = '\0'; // Null-terminate token
                    ++ptr;       // Move past closing quote
                }
            }
            else
            {
                // Unquoted token
                output[count++] = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '"')
                    ++ptr;
                if (*ptr)
                {
                    *ptr = '\0';
                    ++ptr;
                }
            }
        }
        return count;
    }

    uint8_t loadPattern(const String &path, Pulse *pulses, uint8_t flags)
    {
#ifdef BUZZER_USE_SD
        if (pulses == nullptr)
            return 0;
        static Pulse *staticPulses = nullptr;
        static uint8_t staticPulseCount = 0;
        staticPulses = pulses;
        staticPulseCount = 0;
        bool validFile = SDCard::processLines(path, flags, [](const String &line, uint16_t linenum, uint8_t flags) -> bool
                                              {
            String trimmedLine = line;
            trimmedLine.trim();
            if (linenum == 1) {
                if (trimmedLine != F("# pattern")) {
#ifndef BUZZER_SERIAL_DISABLE
                    if (!(flags & BUZ_SILENT))
                        SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "Invalid pattern file format!" ANSI_DEFAULT));
#endif
                    return false;
                }
                return true;
            }
            if (trimmedLine.length() == 0 || trimmedLine.startsWith(F("#")))
                return true;
            if (staticPulseCount >= BUZZER_MAX_PATTERN_PULSES)
                return true;

            char buff[25];
            char *tok[6];
            trimmedLine.toCharArray(buff, sizeof(buff));
            split(buff, tok, 4);
            if (tok[0] != nullptr && tok[1] != nullptr && tok[2] != nullptr && tok[3] != nullptr)
            {
                uint8_t pulseCnt = (uint8_t)atoi(tok[0]);
                uint16_t freq = (uint16_t)atoi(tok[1]);
                uint16_t dur = (uint16_t)atoi(tok[2]);
                uint16_t interval = (uint16_t)atoi(tok[3]);
                staticPulses[staticPulseCount] = Pulse(pulseCnt, freq, dur, interval, 0, false);
                staticPulseCount++;
            }
            return true; });

        uint8_t pulseCount = staticPulseCount;
        if (!validFile)
            return 0;
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
        {
            SERIAL.print(F(BUZ_LOG_PREFIX "Loaded "));
            SERIAL.print(pulseCount);
            SERIAL.print(F(" pulses from "));
            SERIAL.println(path);
        }
#endif
        return pulseCount;
#else
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "SD card support not enabled!" ANSI_DEFAULT));
#endif
        return 0;
#endif
    }

    uint8_t loadTones(const String &path, Tone *tones, uint8_t flags)
    {
#ifdef BUZZER_USE_SD
        if (tones == nullptr)
            return 0;
        static Tone *staticTones = nullptr;
        static uint8_t staticToneCount = 0;
        staticTones = tones;
        staticToneCount = 0;
        bool validFile = SDCard::processLines(path, flags, [](const String &line, uint16_t linenum, uint8_t flags) -> bool
                                              {
            String trimmedLine = line;
            trimmedLine.trim();
            if (linenum == 1) {
                if (trimmedLine != F("# play")) {
#ifndef BUZZER_SERIAL_DISABLE
                    if (!(flags & BUZ_SILENT))
                        SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "Invalid tone file format!" ANSI_DEFAULT));
#endif
                    return false;
                }
                return true;
            }
            if (trimmedLine.length() == 0 || trimmedLine.startsWith(F("#")))
                return true;
            if (staticToneCount >= BUZZER_MAX_MELODY_TONES)
                return true;

            char buff[20];
            char *tok[5];
            trimmedLine.toCharArray(buff, sizeof(buff));
            split(buff, tok, 3);
            if (tok[0] != nullptr && tok[1] != nullptr && tok[2] != nullptr)
            {
                uint16_t freq = (uint16_t)atoi(tok[0]);
                uint16_t dur = (uint16_t)atoi(tok[1]);
                uint16_t rest = (uint16_t)atoi(tok[2]);

                staticTones[staticToneCount] = Tone(freq, dur, rest);
                staticToneCount++;
            }
            return true; });
        uint8_t toneCount = staticToneCount;
        if (!validFile)
            return 0;
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
        {
            SERIAL.print(F(BUZ_LOG_PREFIX "Loaded "));
            SERIAL.print(toneCount);
            SERIAL.print(F(" tones from "));
            SERIAL.println(path);
        }
#endif
        return toneCount;
#else
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "SD card support not enabled!" ANSI_DEFAULT));
#endif
        return 0;
#endif
    }

    bool playFile(const String &path, uint8_t flags)
    {
#ifdef BUZZER_USE_SD
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            SERIAL.println();
#endif
        bool result = SDCard::processLines(path, flags, [](const String &line, uint16_t linenum, uint8_t flags) -> bool
                                           {
            String trimmedLine = line;
            trimmedLine.trim();
            if (linenum == 1) {
                if (trimmedLine != F("# play")) {
#ifndef BUZZER_SERIAL_DISABLE
                    if (!(flags & BUZ_SILENT))
                        SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "Invalid play file!" ANSI_DEFAULT));
#endif
                    return false;
                }
                return true;
            }
            if (trimmedLine.length() == 0 || trimmedLine.startsWith(F("#")))
                return true;

            char buff[20];
            char *tok[5];
            trimmedLine.toCharArray(buff, sizeof(buff));
            split(buff, tok, 3);
            if (tok[0] != nullptr && tok[1] != nullptr && tok[2] != nullptr)
            {
                unsigned int freq = (unsigned int)atoi(tok[0]);
                unsigned int dur = (unsigned int)atoi(tok[1]);
                unsigned int rest = (unsigned int)atoi(tok[2]);
                tone(config.pin, freq, dur);
#ifndef BUZZER_SERIAL_DISABLE
                if (!(flags & BUZ_SILENT))
                {
                    SERIAL.print(F(ANSI_CURSORUP BUZ_LOG_PREFIX "Playing freq: " ANSI_YELLOW));
                    SERIAL.print(freq);
                    SERIAL.print(F(ANSI_DEFAULT " dur: " ANSI_YELLOW));
                    SERIAL.print(dur);
                    SERIAL.print(F("/"));
                    SERIAL.print(rest);
                    SERIAL.println(F(ANSI_CLEAREOL));
                }
#endif
                delay(dur + rest);
            }
            return true; });
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            SERIAL.println(F(ANSI_CURSORUP BUZ_LOG_PREFIX "Play finished." ANSI_CLEAREOL));
#endif
        return result;
#else
#ifndef BUZZER_SERIAL_DISABLE
        if (!(flags & BUZ_SILENT))
            SERIAL.println(F(BUZ_LOG_PREFIX ANSI_ERROR "SD card support not enabled!" ANSI_DEFAULT));
#endif
        return false;
#endif
    }
}