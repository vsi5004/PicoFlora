# PicoFlora - Smart Plant Watering Station

## Project Overview

PicoFlora is a smart plant watering station built on the RP2350 microcontroller with the following features:

- **Multi-Screen LVGL Interface**: Lock screen with touch unlock and automatic timeout
- **Real-Time Clock**: PCF85063 RTC integration with live time display on lock screen
- **PIO-Based Stepper Control**: Precise stepper motor control with adaptive acceleration
- **Touch Navigation**: Touch anywhere on lock screen to unlock, 30-second timeout to lock
- **High-Performance CPU**: Running at 220MHz for optimal performance

## Project Structure

```
PicoFlora/
├── main.c                      # Main application entry point with multi-screen setup
├── CMakeLists.txt             # Build configuration (renamed from hello_serial)
├── lvgl/
│   ├── lv_port/              # LVGL hardware abstraction layer
│   └── lvgl_screen/          # Multi-screen UI system
│       ├── stepper_screen.h/.c    # Stepper motor control UI
│       ├── lock_screen.h/.c       # Lock screen with time/date display
│       ├── screen_manager.h/.c    # Touch unlock and timeout manager
│       └── CMakeLists.txt         # UI module build config
├── stepper/                   # PIO-based stepper motor driver
│   ├── stepper_driver.h       # Driver interface with adaptive acceleration
│   ├── stepper_driver.c       # PIO-based implementation
│   ├── stepper.pio           # PIO state machine for precise timing
│   └── CMakeLists.txt        # Stepper module build config
├── mcp23017/                 # MCP23017 I/O expander integration
│   ├── mcp23017.h/.c         # Core I/O expander driver
│   ├── stepper_io_integration.h/.c  # Stepper power management
│   └── CMakeLists.txt        # MCP23017 module build config
└── libraries/                # External libraries (BSP, LVGL, FatFS)
    └── bsp/                  # Board support with PCF85063 RTC integration
```

## Key Features

### Multi-Screen Interface

**Lock Screen:**
1. **Real-Time Clock**: Live time from PCF85063 RTC chip
2. **Day of Week**: Full day name (e.g., "Saturday")
3. **12-Hour Format**: HH:MM AM/PM display (e.g., "03:42 PM")
4. **Full Date Display**: Month, day, and year (e.g., "August 3, 2025")
5. **Black Background**: High contrast white text on black
6. **Touch to Unlock**: Touch anywhere on screen to access stepper controls
7. **Automatic Display**: Updates continuously when active

**Stepper Control Screen:**
1. **Step Count Slider**: Range 1-10,000 steps (default: 1,000)
2. **Start/Stop Button**: Toggles stepper motor operation with automatic power management
3. **Progress Bar**: Visual progress indicator (0-100%)
4. **Status Labels**: 
   - Current step count with revolution display
   - Target step count with revolution display
   - Motor status (Ready/Running/Stopped/Completed)
5. **Auto-Lock**: Returns to lock screen after 30 seconds of inactivity
6. **Automatic Power Control**: MCP23017 enables stepper only when running

**Touch Navigation:**
- **Touch Lock Screen**: Touch anywhere to unlock and go to stepper controls
- **Automatic Timeout**: Returns to lock screen after 30 seconds of no interaction
- **Activity Reset**: Any touch on stepper screen resets the timeout timer
- **Smooth Transitions**: 500ms fade animation between screens

### PIO-Based Stepper Motor Control with Power Management

- **Pin Configuration**: GPIO 29 (step pin)
- **PIO Implementation**: Hardware-timed square wave generation
- **Frequency Range**: 2,000-5,000 Hz step frequency
- **Adaptive Acceleration**: Scales from 200-1000 steps based on movement distance
- **Timing Precision**: ±1-2μs accuracy validated with oscilloscope
- **States**: Idle → Accelerating → Running → Decelerating → Completed
- **Automatic Power Management**: MCP23017 I/O expander controls stepper enable pin
- **Power Savings**: Motor is only powered when actively running
- **Smart Enable/Disable**: Automatic enable on start, disable on completion

### Real-Time Clock Integration

- **RTC Chip**: PCF85063 I2C real-time clock
- **Time Persistence**: Maintains time across power cycles
- **Auto-Initialization**: Sets default time if RTC loses power
- **12-Hour Display**: User-friendly AM/PM format
- **I2C Communication**: Integrated with BSP I2C system

### Performance Settings

- **CPU Frequency**: 220MHz
- **LVGL Update Rate**: 5ms main loop period
- **Stepper Update Rate**: 2ms frequency updates for smooth acceleration
- **Screen Updates**: Conditional updates based on active screen

## Hardware Requirements

- **Microcontroller**: Raspberry Pi Pico 2 (RP2350)
- **Display**: ST7789 LCD with touch controller (CST328)
- **Real-Time Clock**: PCF85063 I2C RTC chip
- **I/O Expander**: MCP23017 I2C 16-bit I/O expander
- **Stepper Motor**: Any stepper motor compatible with step/direction control
- **Driver**: Stepper motor driver board (TMC2209 recommended, A4988, DRV8825, etc.)

## Pin Connections

- **GPIO 29**: Stepper motor step pin
- **I2C1 (GPIO 2/3)**: Communication bus for all I2C devices:
  - **MCP23017**: Address 0x27 (I/O expander for stepper enable)
  - **CST328**: Address 0x1A (touch controller)
  - **PCF85063**: Address 0x51 (RTC)
  - **QMI8658**: Address 0x6B (sensor)
- **MCP23017 Pin A0**: Stepper motor enable control
- **Display/Touch**: ST7789 LCD and CST328 touch controller via BSP
- **Power Management**: Battery monitoring system integrated

## Build Instructions

1. Ensure Pico SDK 2.1.1 is installed
2. Open project in VS Code with Pico extension
3. Run "Compile Project" task
4. Flash `build/PicoFlora.uf2` to Pico 2 in BOOTSEL mode

## Usage Instructions

### Lock Screen (Default)
1. **View complete date/time information** including day of week
2. **Touch anywhere** on the screen to unlock and access controls
3. **Automatic updates** of time and date display
4. **RTC maintains time** across power cycles

### Stepper Control Screen (Unlocked)
1. **Set target steps** using the slider (1-10,000)
2. **Press START** to begin stepper motor movement
3. **Monitor progress** via progress bar and step counter
4. **Press STOP** to halt movement at any time
5. **View status** updates (Ready/Running/Completed)
6. **Automatic lock** after 30 seconds of inactivity

### Navigation
1. **Start**: System boots to lock screen showing current time/date
2. **Unlock**: Touch anywhere on lock screen to access stepper controls
3. **Auto-lock**: Returns to lock screen after 30 seconds without interaction
4. **Reset timeout**: Any touch on stepper screen resets the 30-second timer

## Code Architecture

### Main Application (`main.c`)
- Initializes system clock to 220MHz
- Sets up PCF85063 RTC with default time fallback
- Initializes multi-screen LVGL interface
- Manages screen navigation and conditional updates
- Runs main event loop with 5ms updates

### Multi-Screen UI (`lvgl/lvgl_screen/`)
- **Screen Manager**: Touch unlock system with 30-second timeout
- **Stepper Screen**: Motor control interface with progress tracking
- **Lock Screen**: Complete date/time display with day of week, 12-hour format, and full date
- **Responsive Design**: Touch-friendly centered layout

### PIO Stepper Driver (`stepper/`)
- **PIO State Machine**: Hardware-timed square wave generation
- **Adaptive Acceleration**: Smart acceleration scaling (200-1000 steps)
- **Frequency Control**: Precise timing with divider calculations
- **Position Tracking**: Real-time step counting and status reporting

## Future Enhancements for Plant Watering Station

1. **Watering Scheduling**: Time-based automatic watering with RTC integration
2. **Sensor Integration**: Soil moisture, light level, and temperature sensors
3. **Water Level Monitoring**: Reservoir level detection and alerts
4. **Data Logging**: Historical watering events and sensor readings
5. **Multiple Plants**: Support for multiple plant zones with individual schedules
6. **Pump Control**: Precise water volume dispensing using stepper-controlled pumps
7. **User Settings**: Configurable watering schedules and sensor thresholds
8. **SD Card Storage**: Long-term data logging and configuration backup

## Configuration Options

Key parameters can be modified in `stepper/stepper_driver.h`:

```c
#define STEPPER_STEP_PIN 29          // GPIO pin for step signal
#define STEPPER_MIN_FREQ_HZ 2000     // Minimum step frequency (PIO-based)
#define STEPPER_MAX_FREQ_HZ 5000     // Maximum step frequency (PIO-based)
#define ADAPTIVE_ACCEL_MIN 200       // Minimum acceleration steps
#define ADAPTIVE_ACCEL_MAX 1000      // Maximum acceleration steps
```

UI parameters in `lvgl/lvgl_screen/stepper_screen.h`:

```c
#define MIN_STEPS 1                  // Minimum slider value
#define MAX_STEPS 10000              // Maximum slider value
#define DEFAULT_STEPS 1000           // Default slider value
```

Screen management in `lvgl/lvgl_screen/screen_manager.h`:

```c
typedef enum {
    SCREEN_LOCK = 0,                // Lock screen with time/date display
    SCREEN_STEPPER,                 // Stepper control screen
    SCREEN_COUNT                    // Total screen count
} screen_id_t;

#define TIMEOUT_MS 30000            // 30-second timeout to lock screen
```

## Troubleshooting

- **No display output**: Check ST7789 display connections and BSP configuration
- **Touch not working**: Verify CST328 touch controller I2C connections
- **Motor not moving**: Verify GPIO 29 connection and TMC2209 driver configuration
- **Erratic movement**: Check power supply stability and driver microstepping settings
- **Lock screen not responding**: Check touch event handlers and screen initialization
- **Auto-lock not working**: Verify timeout logic and lv_tick_get() functionality
- **Time not updating on lock screen**: Check PCF85063 I2C connections and RTC initialization
- **Build errors**: Verify all BSP dependencies are linked in CMakeLists.txt

## Technical Achievements

- **Hardware-Accurate Timing**: PIO generates square waves with ±1-2μs precision
- **Adaptive Algorithm**: Acceleration scales intelligently with movement distance
- **Lock Screen System**: Touch-to-unlock with 30-second auto-timeout functionality
- **Smooth Animations**: 500ms fade transitions between lock and stepper screens
- **Real-Time Integration**: Live clock display from hardware RTC on lock screen
- **Modular Architecture**: Clean separation of UI, motor control, and hardware layers

This project provides a robust foundation for smart plant watering automation and demonstrates advanced embedded GUI development with precise motor control on the RP2350 platform.
