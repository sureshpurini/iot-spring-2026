# ESP32 QEMU Development Environment

**IoT Course - Spring 2026**

This Docker-based environment allows you to develop and test ESP32 applications without physical hardware using QEMU emulation.

## Prerequisites

- **Docker Desktop** installed and running
  - macOS: https://docs.docker.com/desktop/install/mac-install/
  - Windows: https://docs.docker.com/desktop/install/windows-install/
  - Linux: https://docs.docker.com/desktop/install/linux-install/

## Quick Start

### 1. Initial Setup (one-time)

```bash
./setup.sh
```

This builds the Docker image with ESP-IDF and QEMU. Takes ~15-20 minutes on first run.

### 2. Run an Example

```bash
./run-example.sh 01-hello-world
```

Or for the GPIO/Timer example:

```bash
./run-example.sh 02-gpio-timer
```

### 3. Interactive Development

```bash
./start.sh
```

This opens a shell inside the container where you can:

```bash
# Navigate to a project
cd projects/01-hello-world

# Build the project
/workspace/scripts/build.sh .

# Run in QEMU
/workspace/scripts/run-qemu.sh .

# Or do both at once
/workspace/scripts/build-and-run.sh .
```

## Creating Your Own Project

1. Create a new directory in `projects/`:

```bash
mkdir -p projects/my-project/main
```

2. Create `projects/my-project/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my-project)
```

3. Create `projects/my-project/main/CMakeLists.txt`:

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")
```

4. Create `projects/my-project/main/main.c`:

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Hello from my project!\n");
}
```

5. Build and run:

```bash
./run-example.sh my-project
```

## QEMU Controls

- **Exit QEMU**: Press `Ctrl+A` then `X`
- **QEMU Monitor**: Press `Ctrl+A` then `C` (type `quit` to exit)

## What Works in QEMU

| Feature | Status | Notes |
|---------|--------|-------|
| CPU (Xtensa) | ✅ | Dual-core emulation |
| UART | ✅ | Console I/O works |
| GPIO | ✅ | State tracked, no physical output |
| Timers | ✅ | Hardware timers work |
| Flash | ✅ | Persistent storage |
| FreeRTOS | ✅ | Full RTOS support |
| WiFi | ❌ | Not emulated |
| Bluetooth | ❌ | Not emulated |
| I2C | ⚠️ | Basic support only |
| SPI | ⚠️ | Basic support only |
| ADC/DAC | ❌ | Not emulated |

## Directory Structure

```
esp32-qemu/
├── Dockerfile           # Docker image definition
├── docker-compose.yml   # Docker Compose configuration
├── setup.sh             # One-time setup script
├── start.sh             # Start interactive environment
├── run-example.sh       # Run a project directly
├── scripts/             # Build scripts (used inside container)
│   ├── build.sh
│   ├── run-qemu.sh
│   └── build-and-run.sh
├── projects/            # Your ESP32 projects go here
│   ├── 01-hello-world/
│   └── 02-gpio-timer/
└── README.md
```

## Troubleshooting

### Docker image build fails

- Ensure you have a stable internet connection
- Try running `docker system prune` to free up space
- Check Docker Desktop has at least 8GB memory allocated

### QEMU hangs or crashes

- Press `Ctrl+A` then `X` to force exit
- Rebuild the project: `./scripts/build.sh .`

### Build errors inside container

- Make sure you're in the project directory (with CMakeLists.txt)
- Run `idf.py fullclean` then rebuild

## Resources

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/v5.1.2/)
- [ESP-IDF Examples](https://github.com/espressif/esp-idf/tree/v5.1.2/examples)
- [Espressif QEMU](https://github.com/espressif/qemu)
