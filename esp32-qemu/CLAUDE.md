# ESP32 QEMU Project — Session Notes

## Next Session TODO: Test REST API and MQTT Projects

### Test 03-rest-api
```bash
# 1. Start the Flask API server
docker compose up -d api-server

# 2. Build the ESP32 REST project (first time takes several minutes)
docker compose run --rm esp32-dev /workspace/scripts/build.sh /workspace/projects/03-rest-api

# 3. Run in QEMU with networking (interactive — Ctrl+A, X to exit)
docker compose run --rm -e QEMU_NET=1 esp32-dev /workspace/scripts/run-qemu.sh /workspace/projects/03-rest-api

# 4. Verify: API server should show received sensor data
curl http://localhost:5000/api/sensors

# 5. Cleanup
docker compose stop api-server
```

### Test 04-mqtt
```bash
# 1. Start the Mosquitto broker
docker compose up -d mqtt-broker

# 2. Build the ESP32 MQTT project
docker compose run --rm esp32-dev /workspace/scripts/build.sh /workspace/projects/04-mqtt

# 3. (Optional) In a separate terminal, watch MQTT messages:
#    mosquitto_sub -h localhost -t "esp32/#" -v

# 4. Run in QEMU with networking
docker compose run --rm -e QEMU_NET=1 esp32-dev /workspace/scripts/run-qemu.sh /workspace/projects/04-mqtt

# 5. (Optional) Send a command from host while QEMU is running:
#    mosquitto_pub -h localhost -t esp32/commands -m toggle_led

# 6. Cleanup
docker compose stop mqtt-broker
```

### Or use the one-liner (runs in terminal interactively):
```bash
./run-network-example.sh 03-rest-api
./run-network-example.sh 04-mqtt
```

### If build fails or needs a clean rebuild:
```bash
# Inside Docker container:
docker compose run --rm esp32-dev bash
cd /workspace/projects/03-rest-api   # or 04-mqtt
idf.py fullclean
idf.py set-target esp32
idf.py build
```

## Key Architecture Notes
- QEMU networking: `-nic user,model=open_eth` (slirp gives ESP32 IP 10.0.2.15, host at 10.0.2.2)
- ESP32 uses `esp_eth_mac_new_openeth()` (OpenCores Ethernet, QEMU only — not real hardware)
- API server reachable at `http://10.0.2.2:5000` from inside QEMU
- MQTT broker reachable at `mqtt://10.0.2.2:1883` from inside QEMU
- `QEMU_NET=1` environment variable enables the network flag in scripts/run-qemu.sh
