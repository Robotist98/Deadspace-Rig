# Deadspace LEDs

ESP8266 ESP-12E firmware with password-protected ArduinoOTA uploads over Wi-Fi.

## LEDs on GPIO5

FastLED drives 30 WS2812B LEDs with a Dead Space RIG-inspired animation at
brightness 64/255. A 1.6-second power-up fill runs from the first LED to the last,
then the cyan health bar gently pulses every 3.2 seconds while staying lit.
Change the count, brightness, colour order or `RIG_HEALTH_PERCENT` in
`include/led_config.h`.
FastLED is pinned to 3.6.0 for the installed ESP8266 Arduino 2.7.4 toolchain.

The `animateDeadSpaceRig(leds, count, elapsedMs, healthPercent)` function in
`lib/RigLighting` renders a frame without delays or writing to the strip.
Call it with time since the animation started, then call `FastLED.show()`.
The health argument controls the lit portion and colour: 51-100 cyan,
26-50 amber, 1-25 red with a faster 1.2-second pulse, and 0 off. Health defaults
to the configured value of 100; it does not cycle through damage states.

| Connection | Wiring |
| --- | --- |
| External ESP-12 GPIO5 / IO5 | LED strip DIN, through a 330-ohm series resistor |
| LED strip 5 V | Suitable regulated 5 V supply; 2 A covers 30 standard WS2812B LEDs at full white |
| LED strip GND | Supply GND and ESP GND |

Connect the data wire to the strip's DIN end, following the strip's arrows.
GPIO5 is labelled D1 on a NodeMCU; the LEDs connect to the external ESP being
programmed. The bare ESP-12 still needs its own regulated 3.3 V power.
For reliable signalling to a 5 V strip, use a 74AHCT125 or equivalent
3.3 V-to-5 V data buffer, powered from 5 V, before the series resistor.

The animation runs every 20 ms without a blocking delay, including while Wi-Fi
is disconnected. OTA requests are serviced between frames. The LEDs turn off
during an OTA upload and resume on reboot, or after an upload error.

## Firmware structure

`lib/OtaService` is a local PlatformIO library that owns Wi-Fi setup, static IP
configuration, reconnection and ArduinoOTA. It accepts settings from the
application rather than including the project's credentials or LED configuration.

`src/main.cpp` starts it with `ota.begin(OTA_CONFIG, onStart)`, calls `ota.handle()`
each loop, and uses `ota.isUpdating()` to pause LED frames. The optional start
callback clears the LEDs when an update starts. The library resets its update
state on an upload error, letting the animation resume.

## First flash over USB

1. Open `include/config.h` and fill in `WIFI_SSID`, `WIFI_PASSWORD`, and a nonempty
   `OTA_PASSWORD`. Use a 2.4 GHz Wi-Fi network. If the file is missing, copy
   `include/config.example.h` to `include/config.h`. Local credentials are ignored
   by Git. The build requires this local configuration file. With an empty SSID
   or OTA password, the firmware leaves OTA disabled and prints setup instructions
   over serial.
2. Connect the board by USB and upload from the PlatformIO terminal:

   ```powershell
   pio run -e esp12e -t upload
   ```

   If needed, select a serial port with `--upload-port COM3` (use your board's port).
3. Open the serial monitor to see the board's Wi-Fi IP address:

   ```powershell
   pio device monitor -b 115200
   ```

   Once connected, it prints `OTA ready: deadspace-leds.local (...)`.
   If OTA does not work immediately after the initial USB upload, press reset
   or power-cycle the board once.

## Optional fixed IP address

The local `include/config.h` is configured for this network:

```cpp
#define WIFI_STATIC_IP "192.168.0.48"
#define WIFI_GATEWAY "192.168.0.1"
#define WIFI_SUBNET "255.255.255.0"
#define WIFI_DNS ""
```

An empty `WIFI_DNS` uses the router as the DNS server. Reserve `192.168.0.48`
for the ESP in your router, or exclude it from the router's DHCP address pool,
so another device is not assigned the same address. The serial monitor prints
the ESP's MAC address when connected; use that address for the reservation.

Flash the updated firmware, remove the external ESP's GPIO0-to-GND flashing
jumper, and reset the external ESP to run it. From a PC on the same network:

```powershell
ping 192.168.0.48
```

When sending this update over OTA, use the ESP's current address with
`--upload-port`; the new fixed address takes effect after the firmware restarts.

To return to automatic addressing, set `WIFI_STATIC_IP` to `""` and reflash.
The example configuration defaults to automatic addressing. Invalid IP strings
or settings rejected by the Wi-Fi library fall back to DHCP with a serial
message. An unused address on the correct network is still required for a static
configuration to be reachable.

## Subsequent flashes over Wi-Fi

Keep the board powered and connect your computer to the same local network.
Upload from the `esp12e_ota` environment in PlatformIO, or run:

```powershell
pio run -e esp12e_ota -t upload
```

The uploader automatically reads `OTA_PASSWORD` from your local `include/config.h`
and keeps it hidden in the upload output. No environment variable is needed for
normal uploads, including the PlatformIO Upload button. The board restarts
automatically after a successful update.

If you change the firmware's OTA password, authenticate that update with the old
password using a temporary override in the same PowerShell terminal:

```powershell
$env:OTA_PASSWORD = 'old-ota-password'
pio run -e esp12e_ota -t upload
Remove-Item Env:OTA_PASSWORD
```

Later uploads will use the new password from `config.h`.

If an upload reports `Authentication Failed`, the uploader reached the ESP but
the password did not match the firmware currently running on it. Ping does not
check the OTA password. Use the override above if the running firmware has an
older password, or reflash via USB to install the current settings.

If `deadspace-leds.local` does not resolve, use the IP printed by the serial monitor
or shown in your router's client list:

```powershell
pio run -e esp12e_ota -t upload --upload-port 192.168.1.123
```

With the local fixed-IP configuration above, use `--upload-port 192.168.0.48`.

If you change `OTA_HOSTNAME`, update `upload_port` in `platformio.ini` to match,
or use `--upload-port` explicitly. Wi-Fi reconnects automatically if the network
temporarily drops; connection attempts retry every 30 seconds.

If authentication succeeds but the uploader reports `No response from device`,
check the computer's firewall. The board listens on UDP 8266 and connects back
to the uploader on the computer's TCP port 8267, fixed by `upload_flags` in
`platformio.ini`. The network must also allow devices to reach each other
(guest-network isolation can block this).

On Windows, run this once from an administrator PowerShell to allow that
connection from the ESP at `192.168.0.48`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\allow_ota_firewall.ps1
```

The rule applies only to PlatformIO's Python executable, TCP port 8267 and the
specified ESP address. If Windows already has an explicit inbound TCP block for
that Python executable on all ports, the script narrows the block to every other
port, then adds the ESP-specific allow rule. Original port settings are backed up
in `.pio/ota-firewall-backup.xml`. Use `-EspAddress` or `-PythonExe` if these differ
on your machine.

When adding LED code, keep `ota.handle()` running frequently in `loop()`
and avoid long blocking animations. Keep OTA support in future firmware to retain
wireless flashing. USB uploading remains available through the `esp12e` environment.

Reference: [PlatformIO ESP8266 OTA documentation](https://docs.platformio.org/en/latest/platforms/espressif8266.html#over-the-air-ota-update).
