rm Nano.elf
make
xtensa-esp32-elf-gcc -Wl,-Tld/esp32.rom.nanofmt.ld -Wl,-r -nostartfiles -nodefaultlibs -nostdlib -Os -Wl,-Tld/esp32.rom.ld -o Nano.elf build/src/main.o build/src/second.o
