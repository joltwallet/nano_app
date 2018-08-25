BIN_NAME=Nano.elf
rm ${BIN_NAME}
make
xtensa-esp32-elf-gcc -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.nanofmt.ld \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.ld -s -o ${BIN_NAME} \
    -Wl,-r \
    -Wl,-eapp_main \
    -Wl,--warn-unresolved-symbols \
    build/src/libsrc.a 
#    build/nano_lib/libnano_lib.a

# Add path as an ELF Section;
# Format purpose and coin has uint32
HARDEN=0x80000000
PURPOSE=44
COIN=165
BIP32_KEY="ed25519_seed"
H_PURPOSE=$(( $PURPOSE | $HARDEN ))
H_COIN=$(( $COIN | $HARDEN ))
# Swap endianess https://stackoverflow.com/a/9955198
printf "0: %.8x" $H_PURPOSE | sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' | \
        xxd -r -g0 > coin.path
printf "0: %.8x" $H_COIN | sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' | \
        xxd -r -g0 >> coin.path
printf $BIP32_KEY >> coin.path

if [[ "$OSTYPE" == "darwin"* ]]; then
    gobjcopy --add-section .coin.path=coin.path ${BIN_NAME}
fi
