<div style="float:right;">

![Arduino](https://gist.githubusercontent.com/breadbakerman/8ca75dd6123b4d2882f26837436ef647/raw/arduino-logo.svg)
![Platform.io](https://gist.githubusercontent.com/breadbakerman/70b7ee4430defddaf4f9323ffb73634c/raw/platformio-logo.svg)

</div>

# AsyncBuzzer Library
[![Library Compile Test](https://github.com/breadbakerman/AsyncBuzzer/actions/workflows/compile.yml/badge.svg)](https://github.com/breadbakerman/AsyncBuzzer/actions/workflows/compile.yml)

A non-blocking Arduino library for controlling buzzers with support for beeps, pulses, and tone sequence playback.

## Features

- **Non-blocking operation**: Uses `update()` calls instead of `delay()` for timing
- **Multiple sound types**: Single beeps, pulse sequences, pattern sequences, melody playback, and tone sequence playback
- **Melody independence**: Melodies play independently and don't interfere with beep, pulse, or pattern functions
- **Individual tone control**: Each tone can have its own rest period for precise timing control
- **Pattern support**: Play complex sequences of pulse groups with configurable delays
- **SD card support**: Load melodies and patterns from SD card files with simple text formats (optional)
- **Configurable settings**: Separate settings for acknowledgment and error sounds
- **Memory efficient**: Configurable limits for melody and pattern arrays
- **Hardware flexible**: Use pin 255 to disable buzzer functionality, automatic pin mode configuration

## Installation

1. Copy the `AsyncBuzzer` folder to your Arduino libraries directory
2. Include the header in your sketch: `#include <AsyncBuzzer.h>`

## Examples

The library includes example sketches to demonstrate usage:

- **Beep**: Basic beeping functionality with timed intervals
- **Pulse**: Pulse patterns with different timing configurations

Access examples in the Arduino IDE via File → Examples → AsyncBuzzer.

## Configuration

The library can be configured through compile-time definitions in an optinal `config.h` or before including the header:

```cpp
#define BUZZER_USE_SD            // Enable SD card support (requires SDCard library)
#define BUZZER_SERIAL_DISABLE    // Disable serial output (set if SERIAL_OUT_DISABLE is defined)
#define BUZZER_PIN A1            // Pin for buzzer
#define BUZZER_ACK_FREQ 4000     // Frequency for acknowledgment beep (Hz)
#define BUZZER_ACK_DURATION 20   // Duration for acknowledgment beep (ms)
#define BUZZER_ACK_REST 80       // Rest period after acknowledgment beep (ms)
#define BUZZER_ERR_FREQ 100      // Frequency for error beep (Hz)
#define BUZZER_ERR_DURATION 300  // Duration for error beep (ms)
#define BUZZER_ERR_REST 150      // Rest period after error beep (ms)
#define BUZZER_PULSE_INTERVAL 80 // Default interval between pulses (ms) - for backward compatibility
#define BUZZER_MAX_MELODY_TONES 30    // Maximum number of tones in a melody
#define BUZZER_MAX_PATTERN_PULSES 20  // Maximum number of pulses in a pattern
```

### ANSI Color Configuration

The library supports optional ANSI color codes for serial output. If not defined in your `config.h`, they default to empty strings (no colors). To enable colored output, define these in your `config.h`:

```cpp
#define ANSI_GRAY "\e[38;5;8m"      // Gray color for log prefixes
#define ANSI_DEFAULT "\e[1;37m"     // Default white color
#define ANSI_YELLOW "\e[38;5;11m"   // Yellow color for values
#define ANSI_ERROR "\e[38;5;9m"     // Red color for errors
#define ANSI_CURSORUP "\e[1A"       // Move cursor up (for file playback)
#define ANSI_CLEAREOL "\e[0K"       // Clear to end of line
```

The library automatically provides fallback empty definitions if these are not defined, ensuring compilation compatibility regardless of whether color support is configured.

## API Reference

Many functions accept optional flags to control their behavior and timing.

### Setup Functions

```cpp
// Initialize with pin and optional flags
bool setup(uint8_t pin = BUZZER_PIN, uint8_t flags = BUZ_NONE);

// Initialize with full configuration
bool setup(Config conf, uint8_t flags = BUZ_NONE);
```

**Flags:**
- `BUZ_NONE`: No special behavior
- `BUZ_BEEP`: Play initialization beep sequence
- `BUZ_SILENT`: Suppress console output
- `BUZ_FORCE`: Force re-initialization even if pin hasn't changed

### Configuration Management

```cpp
// Get current configuration
Config getConfig();

// Set new configuration
Config setConfig(Config conf, uint8_t flags = BUZ_NONE);
```

### Sound Functions

```cpp
// Single beep with specified frequency and duration
void beep(uint16_t frequency = getConfig().ack.frequency,
          uint16_t duration = getConfig().ack.duration);

// Non-blocking pulse sequence
void pulse(uint8_t count = 3,
           uint16_t frequency = getConfig().ack.frequency,
           uint16_t duration = getConfig().ack.duration,
           uint16_t rest = getConfig().ack.rest);

// Blocking pulse sequence (waits until complete)
void pulseBlocking(uint8_t count = 3,
                   uint16_t frequency = getConfig().ack.frequency,
                   uint16_t duration = getConfig().ack.duration,
                   uint16_t rest = getConfig().ack.rest);

// Play tone sequence from SD card (requires BUZZER_USE_SD)
bool playFile(const String &path, uint8_t flags = BUZ_NONE);

// Pattern playback - play sequence of pulse groups
void pattern(Pulse* pulses, uint8_t count, bool repeat = false, uint16_t pulseDelay = 300);

// Blocking pattern playback (waits until complete)
void patternBlocking(Pulse* pulses, uint8_t count, bool repeat = false, uint16_t pulseDelay = 300);

// Check if pattern is currently playing
bool isPatternActive();

// Stop current pattern immediately
void stopPattern();

// Load pattern from SD card file (requires BUZZER_USE_SD)
uint8_t loadPattern(const String &path, Pulse *pulses, uint8_t flags = BUZ_NONE);

// Melody playback - play sequence of tones (non-interfering with other sounds)
void melody(Tone *tones, uint8_t count, bool repeat = false);

// Blocking melody playback (waits until complete)
void melodyBlocking(Tone *tones, uint8_t count, bool repeat = false);

// Check if melody is currently playing
bool isMelodyActive();

// Stop current melody immediately
void stopMelody();

// Load melody from SD card file (requires BUZZER_USE_SD)
uint8_t loadTones(const String &path, Tone *tones, uint8_t flags = BUZ_NONE);
```

### Update Function

Non-blocking pulse sequences, patterns, and melodies require regular `update()` calls to function properly.
```cpp
// Must be called regularly in main loop for non-blocking operation
bool update();
```

## Configuration Structures

### Tone Structure
```cpp
struct Tone {
    uint16_t frequency;  // Frequency in Hz
    uint16_t duration;   // Duration in milliseconds
    uint16_t rest;       // Rest period after tone in milliseconds
};
```

### Config Structure
```cpp
struct Config {
    uint8_t pin;    // Buzzer pin (255 = disabled)
    Tone ack;       // Acknowledgment tone settings
    Tone err;       // Error tone settings
};
```

### Pulse Structure
```cpp
struct Pulse {
    uint8_t pulses;      // Number of beeps in this pulse group
    uint16_t frequency;  // Tone frequency in Hz
    uint16_t duration;   // Duration of each beep in milliseconds
    uint16_t interval;   // Time between beeps in this pulse group
    uint32_t last;       // Runtime field - last beep timestamp
    bool active;         // Runtime field - pulse group active state
};
```

### Pattern Structure
```cpp
struct Pattern {
    Pulse* pulses;          // Array of pulse definitions
    uint8_t count;          // Number of pulses in pattern
    uint8_t current;        // Current pulse index (runtime)
    bool active;            // Pattern active state (runtime)
    bool repeat;            // Should pattern repeat when finished
    uint16_t pulseDelay;    // Delay between pulse groups in milliseconds
    uint32_t lastPulseEnd;  // When last pulse finished (runtime)
    bool waitingForDelay;   // Waiting for delay between pulses (runtime)
};
```

## Usage Examples

### Basic Setup and Usage

```cpp
#include <AsyncBuzzer.h>

void setup() {
    // Initialize buzzer on pin A1 with startup beep
    AsyncBuzzer::setup(A1, BUZ_BEEP);
}

void loop() {
    // Must call update() for non-blocking operation
    AsyncBuzzer::update();

    // Your other code here
    if (someCondition) {
        AsyncBuzzer::beep();  // Quick acknowledgment beep
    }

    if (errorCondition) {
        AsyncBuzzer::pulse(5);  // 5-pulse error sequence
    }
}
```

### Custom Configuration

```cpp
void setup() {
    // Create custom configuration
    AsyncBuzzer::Config conf;
    conf.pin = A1;
    conf.ack = {2000, 50, 100};  // 2kHz, 50ms duration, 100ms rest
    conf.err = {500, 200, 150};  // 500Hz, 200ms duration, 150ms rest

    AsyncBuzzer::setup(conf);
}
```

### Runtime Configuration Changes

```cpp
void changeSettings() {
    AsyncBuzzer::Config conf = AsyncBuzzer::getConfig();
    conf.ack.frequency = 3000;  // Change ack frequency to 3kHz
    conf.ack.rest = 80;         // Change ack rest period to 80ms
    conf.err.duration = 500;    // Change error duration to 500ms
    AsyncBuzzer::setConfig(conf);
}
```

### Pattern Sequences

```cpp
// Define pattern arrays as global/static for persistence
AsyncBuzzer::Pulse startupPattern[] = {
    AsyncBuzzer::Pulse(1, 440, 100, 0),   // A4 note
    AsyncBuzzer::Pulse(1, 554, 100, 0),   // C#5 note
    AsyncBuzzer::Pulse(1, 659, 200, 0)    // E5 note
};

AsyncBuzzer::Pulse alarmPattern[] = {
    AsyncBuzzer::Pulse(3, 1000, 100, 100), // 3 high beeps
    AsyncBuzzer::Pulse(1, 500, 500, 0),    // 1 long low beep
    AsyncBuzzer::Pulse(2, 800, 150, 200)   // 2 medium beeps
};

void setup() {
    AsyncBuzzer::setup(A1);

    // Play startup melody once with 200ms between pulse groups
    AsyncBuzzer::pattern(startupPattern, 3, false, 200);
}

void triggerAlarm() {
    // Play alarm pattern repeatedly with 500ms between pulse groups
    AsyncBuzzer::pattern(alarmPattern, 3, true, 500);
}

void stopAlarm() {
    if (AsyncBuzzer::isPatternActive()) {
        AsyncBuzzer::stopPattern();
    }
}
```

### Blocking Pattern for Initialization

```cpp
void setup() {
    AsyncBuzzer::setup(A1);

    // Play startup sequence and wait for completion
    AsyncBuzzer::patternBlocking(startupPattern, 3, false, 300);

    // This code runs after pattern completes
    Serial.println("Startup sequence complete");
}
```

### Melody Playback

```cpp
// Define melody as Tone array
AsyncBuzzer::Tone imperialMarch[BUZZER_MAX_MELODY_TONES];
AsyncBuzzer::Tone twinkleTwinkle[] = {
    AsyncBuzzer::Tone(262, 400, 100),  // C4
    AsyncBuzzer::Tone(262, 400, 100),  // C4
    AsyncBuzzer::Tone(392, 400, 100),  // G4
    AsyncBuzzer::Tone(392, 400, 100),  // G4
    AsyncBuzzer::Tone(440, 400, 100),  // A4
    AsyncBuzzer::Tone(440, 800, 200),  // A4 (longer)
    AsyncBuzzer::Tone(392, 800, 400)   // G4 (longer with pause)
};

void setup() {
    AsyncBuzzer::setup(A1);

    // Load melody from SD card
    uint8_t toneCount = AsyncBuzzer::loadTones("/sound/imperial", imperialMarch);
    if (toneCount > 0) {
        AsyncBuzzer::melody(imperialMarch, toneCount, true); // repeat
    }
}

void playTwinkle() {
    // Play predefined melody once
    AsyncBuzzer::melody(twinkleTwinkle, 7, false);
}

void stopBackgroundMusic() {
    if (AsyncBuzzer::isMelodyActive()) {
        AsyncBuzzer::stopMelody();
    }
}
```

### Loading Patterns from SD Card

```cpp
AsyncBuzzer::Pulse alarmPattern[BUZZER_MAX_PATTERN_PULSES];

void loadAndPlayAlarm() {
    // Load pattern from SD card file
    uint8_t pulseCount = AsyncBuzzer::loadPattern("/sound/alarm.pat", alarmPattern);

    if (pulseCount > 0) {
        AsyncBuzzer::pattern(alarmPattern, pulseCount, true, 500); // repeat with 500ms delay
    } else {
        Serial.println("Failed to load alarm pattern");
    }
}
```

### Sound File Playback (with SD support)

The playFile function supports the same format as melody files but plays them immediately using blocking delays. The key difference is that it reads a file line by line, allowing melodies of any length to be played.

```cpp
// Sound file format:
// # play
// # frequency, duration, rest
// 1000, 100, 50
// 800, 150, 100

void playSoundFile() {
    if (!AsyncBuzzer::playFile("/sounds/startup.txt")) {
        Serial.println("Failed to play sound file");
    }
}
```

## Sound File Formats

When using SD card support, the library supports multiple file formats:

### Melody Files (for loadTones and melody functions)

```
# play
frequency, duration, rest
frequency, duration, rest
...
```

- `# play`: required prologue
- `frequency`: Tone frequency in Hz (0 = rest/silence)
- `duration`: Tone duration in milliseconds
- `rest`: Silent period after tone in milliseconds
- Values are comma-separated
- Lines starting with `#` are comments
- Empty lines are ignored

Example melody file (`imperial.txt`):
```
# play
# Imperial March
440, 500, 150
440, 500, 150
440, 500, 150
349, 350, 105
523, 150, 45
440, 500, 150
```

### Pattern Files (for loadPattern and pattern functions)

```
# pattern
pulses, frequency, duration, interval
pulses, frequency, duration, interval
...
```

- `# pattern`: required prologue
- `pulses`: Number of beeps in this pulse group
- `frequency`: Tone frequency in Hz
- `duration`: Duration of each beep in milliseconds
- `interval`: Time between beeps in this pulse group
- Values are comma-separated
- Lines starting with `#` are comments
- Empty lines are ignored

Example pattern file (`alarm.pat`):
```
# pattern
# Braun Alarm Clock
4, 2000, 150, 100
4, 2000, 150, 100
4, 2000, 150, 100
```

## Dependencies

- Arduino core libraries
- `config.h` (optional, auto-detected for project-specific settings)
- `SDCard.h` (optional, auto-detected for SD card file playback)

## Notes

- Always call `AsyncBuzzer::update()` in your main loop for proper non-blocking operation
- Starting a new pattern or pulse automatically stops any current pattern
- Sound file playback is blocking and uses `delay()` internally
- Memory limits: Melody arrays are limited by `BUZZER_MAX_MELODY_TONES`, pattern arrays by `BUZZER_MAX_PATTERN_PULSES`
- Pattern pulse delay accounts for the last beep's duration to ensure accurate timing
- Use pin 255 to disable the buzzer functionality
- The library automatically handles pin mode configuration

## License

MIT License - Copyright (c) 2025 by breadbaker
