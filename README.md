# Deadspace LEDs

ESP8266 ESP-12E firmware with password-protected ArduinoOTA uploads over Wi-Fi.

## LEDs on GPIO5

FastLED drives 24 WS2812B LEDs on GPIO5 at the configured brightness of 255/255.
LED numbers start at 1 at the strip DIN end:

| LEDs | RIG section | Behaviour |
| --- | --- | --- |
| 1-5 | Status / stasis gauge | Blue charge bar, independent of spine health |
| 6-24 | Main spine | 1.6-second power-up fill from LED 6 toward LED 24, then a gentle 3.2-second health pulse |

Change the section positions and lengths with `RIG_STATUS_FIRST_LED`,
`RIG_STATUS_LED_COUNT`, `RIG_SPINE_FIRST_LED` and `RIG_SPINE_LED_COUNT` in
`include/led_config.h`. The sections must fit within `LED_COUNT` without
overlapping; the build checks this. Any LEDs outside the sections are kept off.
The same file sets the overall count, brightness, colour order and
`RIG_HEALTH_PERCENT`.

`RIG_STATUS_REVERSED` defaults to 1 and `RIG_SPINE_REVERSED` defaults to 0.
The status gauge refills from LED 5 toward LED 1; the spine uses its original
direction, filling from LED 6 toward LED 24. Their physical positions stay on
LEDs 1-5 and 6-24. Each direction flag can be changed independently.
FastLED is pinned to 3.6.0 for the installed ESP8266 Arduino 2.7.4 toolchain.

`RigController` tracks health, stasis, power, animation timing and demo mode.
`renderRig()` in `lib/RigLighting` renders both sections without delays or writing
to the strip. Health colours are cyan above 50%, amber at 26-50%, and red with a
double heartbeat at 1-25%. At zero health the spine is off; the stasis gauge
continues to show its charge. `animateDeadSpaceRig()` remains available for
rendering a single health bar.

## Web controls and animations

After flashing, open **http://192.168.0.48/** from a phone or computer on the same
Wi-Fi network. The page is stored in firmware, with no internet connection,
filesystem upload or external assets required. It shows live health and stasis
levels, brightness and the current effect.

| Control | Behaviour |
| --- | --- |
| Health slider | Set spine health from 0 to 100% |
| Stasis slider | Smoothly change charge on LEDs 1-5 independently of health |
| Brightness slider | Set overall brightness from 0 to 255 |
| Power up | Replay the 1.6-second fill, retaining current health and stasis |
| Take damage | Flash the spine red twice, then settle 25 percentage points lower |
| Heal to full | Fill the spine in cyan from current health to full over 2.2 seconds |
| Critical heartbeat | Set health to 20% with a red double heartbeat |
| Drain / recharge stasis | Empty or refill the blue status gauge over 1.8 seconds |
| Malfunction | Flicker and lose sections for 2.4 seconds, then recover previous levels |
| Power down | Shrink the spine toward LED 6, fade the status light last, then stay off |
| Start / stop demo | Cycle through power-up, damage, critical, healing, stasis, malfunction and power-down |

Health, stasis and effect commands stop demo mode. Brightness can change while
it runs. Stopping the demo lets the current effect finish. Levels and demo mode
are held in RAM: restarting restores `RIG_HEALTH_PERCENT`, `LED_BRIGHTNESS`, full
stasis and demo off. The control page has no login and is intended for devices
on your local network; OTA continues to use its separate existing password.

The HTTP API uses `GET /api/state`, `POST /api/settings` with form fields
`health`, `stasis`, `brightness`, and `POST /api/action` with an `action` field.
Actions are `power-up`, `damage`, `heal`, `critical`, `malfunction`, `power-down`,
`stasis-drain`, `stasis-recharge`, `demo-start` and `demo-stop`. Successful requests
return the current state as JSON. Invalid values return HTTP 400 without changing
settings. State reads do not trigger animations.

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
is disconnected. Web requests and OTA requests are serviced between frames. The LEDs turn off
during an OTA upload and resume on reboot, or after an upload error.

## Party lights

Open **http://192.168.0.48/party** or use the Party lights tab on the RIG page.
Party mode uses all 24 LEDs as one strip. It offers rainbow, colour wave, chase,
sparkle, breathing, scanner, comet, solid colour and strobe effects, plus a colour picker,
speed and shared brightness controls. Rainbow and wave use their own spectrum;
the other effects use the chosen colour. Strobe flashes the whole strip in that
colour, with the speed control adjusting roughly 1-10 flashes per second.
It also participates in the automatic playlist.

The party playlist changes effects every 10 seconds. Choosing an effect stops
the playlist; adjusting colour, speed or brightness lets it continue. Selecting a
party effect or changing its colour/speed activates Party mode and stops the RIG
demo. Opening either page only displays controls and never changes the mode.
Activate RIG, or use a RIG health/stasis/effect control, to return to the suit.
The previous health and stasis levels are retained. Brightness applies to both
modes. Restarting returns to RIG mode with the configured boot defaults.

`POST /api/party` accepts form fields `pattern`, `speed` (1-100), `colour`
(`#RRGGBB`) and `cycle` (`0` or `1`). Pattern names are `rainbow`, `wave`, `chase`,
`sparkle`, `breathe`, `scanner`, `comet`, `solid` and `strobe`. `POST /api/mode` accepts
`mode=rig` or `mode=party`. Invalid requests return 400 before changing either
mode or settings. `GET /api/state` includes the active mode, party settings and
RIG direction flags as well as the original RIG state.

## Firmware structure

`lib/OtaService` is a local PlatformIO library that owns Wi-Fi setup, static IP
configuration, reconnection and ArduinoOTA. It accepts settings from the
application rather than including the project's credentials or LED configuration.

`lib/RigLighting` owns the hardware-independent controller and the FastLED
renderer, section reversal and the nonblocking `PartyLighting` controller.
`lib/RigWeb` owns the HTTP server and both embedded control pages.
`src/main.cpp` connects these libraries, calls `ota.handle()` each loop, and
pauses web handling and LED frames while an OTA upload is active. The OTA start
callback clears the LEDs; an upload error allows normal operation to resume.

Run the controller and renderer tests with a C++ compiler on your PATH:

```powershell
python scripts/test_rig.py
```

Set `CXX` if the compiler is elsewhere. Tests cover effect transitions, interrupted
healing, demo cycling, manual override, timer rollover, reversed section boundaries,
all party effects and playlist transitions.
The small host FastLED adapter tests pixel placement; build the ESP environments
to verify compatibility with the actual FastLED library and ESP8266 framework.

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
