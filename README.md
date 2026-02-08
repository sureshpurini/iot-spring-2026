# IoT Course - Spring 2026

Course materials for the Internet of Things (IoT) course, Spring 2026.

## Repository Structure

```
iot-spring-2026/
├── esp32-qemu/       ESP32 development with QEMU emulation (no hardware needed)
├── arduino/          Arduino IDE examples for ESP32
├── slides/           Lecture presentation slides (LaTeX/PDF)
└── lectures/         Lecture notes and supplementary materials
```

## Getting Started

### ESP32 QEMU (Recommended Starting Point)

Develop and test ESP32 applications without physical hardware using QEMU emulation inside Docker.

```bash
cd esp32-qemu
./setup.sh                          # One-time Docker image build (~15 min)
./run-example.sh 01-hello-world     # Build and run in QEMU
```

**Available projects:**

| Project | Description |
|---------|-------------|
| `01-hello-world` | Basic UART output, chip info, FreeRTOS delays |
| `02-gpio-timer` | GPIO output, hardware timers, interrupts, queues |
| `03-rest-api` | HTTP GET/POST to REST API server (networking) |
| `04-mqtt` | MQTT publish/subscribe with Mosquitto broker (networking) |

Network-enabled projects (03, 04) include Docker Compose services and use QEMU's virtual Ethernet:
```bash
./run-network-example.sh 03-rest-api
./run-network-example.sh 04-mqtt
```

See [`esp32-qemu/docs/index.html`](esp32-qemu/docs/index.html) for the full interactive development guide.

### Arduino IDE Examples

Simple Arduino sketches for ESP32 — good for quick prototyping and learning the basics.

### Slides

Lecture presentation slides built with LaTeX Beamer.

```bash
cd slides
make          # Build PDF (requires pdflatex)
```

## Prerequisites

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) (for ESP32 QEMU)
- [Arduino IDE](https://www.arduino.cc/en/software) (for Arduino examples, optional)
- LaTeX distribution (for building slides, optional)
