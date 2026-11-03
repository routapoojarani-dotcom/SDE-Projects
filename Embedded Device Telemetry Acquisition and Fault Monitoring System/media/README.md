# Media

Place project screenshots and photos here:

- `dashboard_screenshot.png` — screenshot of the live telemetry dashboard running in the terminal
- `wiring_photo.png` — photo of the STM32 Nucleo board wired with potentiometer, LEDs, and I2C sensor

To capture a dashboard screenshot, run the simulate command and take a screenshot:

```bash
python -m telemetry_client.cli simulate --csv data/sample_logs/simulated.csv --duration 30
```
