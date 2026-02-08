"""
Simple REST API Server for ESP32 IoT Demo
IoT Course - Spring 2026

Endpoints:
  GET  /api/sensors           - List all sensor readings
  POST /api/sensors           - Submit a new sensor reading
  GET  /api/sensors/latest    - Get the most recent reading
  GET  /api/config            - Get device configuration
  GET  /health                - Health check
"""

from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

# In-memory storage for sensor readings
sensor_readings = []

# Device configuration
device_config = {
    "sample_interval_ms": 5000,
    "device_name": "esp32-qemu-01",
    "firmware_version": "1.0.0",
    "sensors_enabled": ["temperature", "humidity"]
}


@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok", "timestamp": datetime.now().isoformat()})


@app.route("/api/config", methods=["GET"])
def get_config():
    return jsonify(device_config)


@app.route("/api/sensors", methods=["GET"])
def get_sensors():
    return jsonify({"readings": sensor_readings, "count": len(sensor_readings)})


@app.route("/api/sensors", methods=["POST"])
def post_sensor():
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"error": "Invalid JSON body"}), 400

    reading = {
        "id": len(sensor_readings) + 1,
        "received_at": datetime.now().isoformat(),
        "device": data.get("device", "unknown"),
        "temperature": data.get("temperature"),
        "humidity": data.get("humidity"),
    }
    sensor_readings.append(reading)

    print(f"[SENSOR DATA] Device={reading['device']} "
          f"Temp={reading['temperature']} Humidity={reading['humidity']}")

    return jsonify(reading), 201


@app.route("/api/sensors/latest", methods=["GET"])
def get_latest():
    if not sensor_readings:
        return jsonify({"error": "No readings yet"}), 404
    return jsonify(sensor_readings[-1])


if __name__ == "__main__":
    print("=" * 50)
    print("  IoT Sensor API Server")
    print("  Listening on http://0.0.0.0:5000")
    print("=" * 50)
    app.run(host="0.0.0.0", port=5000, debug=False)
