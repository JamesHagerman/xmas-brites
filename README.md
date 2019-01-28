# Xmas Brites

This repo holds code to send colors to ws2813 LED strips for use as holiday lights.

## Compiling 

- Grab the Particle firmware
- Switch to the correct branch
- In that repo:
  - `cd modules/`
  - `make all PLATFORM=xenon APPDIR=$HOME/dev/for_fun/xmas-brites`
- From this repo:
  - `p flash device-name target/xmas-brites.bin`
  - or
  - `p flash --usb target/xmas-brites.bin`


