# PicoFlora - Smart Plant Watering Station

## Project Overview

PicoFlora is a smart plant watering station built on the Waveshare RP2350-Touch-LCD-2.8 development board with additional components for precision plant care automation. The system features:

- **Multi-Screen LVGL Interface**: Lock screen with touch unlock and automatic timeout
- **Real-Time Clock**: PCF85063 RTC integration with live time display on lock screen
- **Precision Pump Control**: BigTreeTech TMC2209-driven Boxer 9QX peristaltic pump for accurate water dosing
- **Touch Navigation**: Touch anywhere on lock screen to unlock, 30-second timeout to lock
- **High-Performance CPU**: Running at 220MHz for optimal performance
- **Automatic Power Management**: MCP23017 I/O expander controls pump enable for power savings

## Project Structure

```
PicoFlora/
├── main.c                      # Main application entry point with multi-screen setup
├── CMakeLists.txt             # Build configuration
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
    └── bsp/                  # Board support from Waveshare
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

- **Pin Configuration**: GPIO 29 (step pin to TMC2209)
- **PIO Implementation**: Hardware-timed square wave generation
- **Frequency Range**: 2,000-5,000 Hz step frequency
- **Adaptive Acceleration**: Scales from 200-1000 steps based on movement distance
- **Timing Precision**: ±1-2μs accuracy validated with oscilloscope
- **States**: Idle → Accelerating → Running → Decelerating → Completed
- **Stepper Driver**: BigTreeTech TMC2209 for silent operation and precision control
- **Pump Integration**: Boxer 9QX peristaltic pump for accurate water dosing
- **Automatic Power Management**: MCP23017 I/O expander controls TMC2209 enable pin
- **Power Savings**: Motor is only powered when actively running
- **Smart Enable/Disable**: Automatic enable on start, disable on completion

### Hardware Platform Features

**Waveshare RP2350-Touch-LCD-2.8 Specifications:**
- **Processor**: Dual-core ARM Cortex-M33 @ 150MHz (overclocked to 220MHz)
- **Memory**: 520KB SRAM, 4MB Flash
- **Display**: 2.8" IPS LCD, 320×240 resolution, ST7789 controller
- **Touch**: Capacitive touch with CST328 controller
- **Sensors**: PCF85063 RTC, QMI8658 6-axis IMU
- **Power**: USB-C charging, battery management, power monitoring
- **Form Factor**: Compact development board with integrated peripherals

**Performance Optimizations:**
- **CPU Overclocking**: 220MHz operation for responsive UI and precise timing
- **I2C Bus Management**: Single bus handles multiple devices efficiently
- **PIO Utilization**: Hardware-timed stepper control without CPU blocking
- **Power Management**: Dynamic frequency scaling and component power control

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

**Base Platform:**
- **[Waveshare RP2350-Touch-LCD-2.8](https://www.waveshare.com/wiki/RP2350-Touch-LCD-2.8#Introduction)** - Complete development board featuring:
  - Raspberry Pi Pico 2 (RP2350) microcontroller
  - 2.8" ST7789 320×240 IPS LCD display
  - CST328 capacitive touch controller
  - PCF85063 I2C real-time clock
  - QMI8658 6-axis IMU sensor
  - Battery management system
  - Onboard USB-C connector and power management

**Additional Components:**
- **MCP23017 I2C I/O Expander** - External breakout board for stepper enable control
- **BigTreeTech TMC2209 Stepper Driver** - Silent stepper motor driver with UART control
- **Boxer 9QX Peristaltic Pump** - Precision dosing pump for plant watering applications

## Pin Connections

**Waveshare RP2350-Touch-LCD-2.8 Built-in Connections:**
- **GPIO 29**: Stepper motor step pin (connected to TMC2209 STEP)
- **I2C1 (GPIO 6/7)**: Onboard I2C bus connecting:
  - **CST328**: Address 0x1A (capacitive touch controller)
  - **PCF85063**: Address 0x51 (real-time clock)
  - **QMI8658**: Address 0x6B (6-axis IMU sensor)
- **ST7789 Display**: Connected via SPI (handled by BSP layer)
- **Battery Management**: Integrated power monitoring and charging

**External Connections:**
- **MCP23017**: Address 0x27 (connected to I2C1 bus via breakout board)
  - **Pin A0**: TMC2209 stepper driver enable control
- **TMC2209 Stepper Driver**: Connected to Boxer 9QX peristaltic pump
  - **STEP**: GPIO 29 (step pulses from RP2350)
  - **DIR**: Left unconnected 
  - **EN**: MCP23017 Pin A0 (automatic enable/disable control)
- **Boxer 9QX Pump**: Connected to TMC2209 stepper output

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

1. **Automated Watering Schedules**: Time-based watering with RTC integration and multiple daily cycles
2. **Sensor Integration**: Soil moisture sensors for demand-based watering triggers
3. **Multi-Plant Support**: Multiple pump channels with individual schedules and dosing profiles
4. **Water Level Monitoring**: Reservoir level detection with low-water alerts
5. **Environmental Monitoring**: Light level and temperature sensors using onboard QMI8658 IMU
6. **Data Logging**: Historical watering events, sensor readings, and pump performance metrics
7. **Wireless Connectivity**: Wi-Fi module for remote monitoring and control
8. **Precision Dosing**: Calibrated pump control for fertilizer and nutrient solutions
9. **User Interface Enhancements**: Settings screens for pump calibration and schedule configuration
10. **SD Card Integration**: Long-term data storage and configuration backup using FatFS library

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
