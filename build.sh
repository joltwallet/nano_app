rm Nano.elf
make
xtensa-esp32-elf-gcc -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.nanofmt.ld \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.ld -s -o Nano.elf \
    -Wl,-r \
    -Wl,--whole-archive \
    build/src/libsrc.a
