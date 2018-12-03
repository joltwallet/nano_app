ELF_BIN_NAME=Nano.elf
JELF_BIN_NAME=Nano.jelf
BIN_COMPRESSED_NAME=Nano.jelf.hs
COIN_PATH="44'/165'"
BIP32_KEY="ed25519_seed"

STRIP=true

printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

if [ -f ${ELF_BIN_NAME} ] ; then
    rm ${ELF_BIN_NAME}
fi
if [ -f ${JELF_BIN_NAME} ] ; then
    rm ${JELF_BIN_NAME}
fi
if [ -f ${BIN_COMPRESSED_NAME} ] ; then
    rm ${BIN_COMPRESSED_NAME}
fi

make app

if xtensa-esp32-elf-gcc -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    -ffunction-sections -fdata-sections -Wl,--gc-sections \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.nanofmt.ld \
    -Wl,-T${IDF_PATH}/components/esp32/ld/esp32.rom.ld -s -o ${ELF_BIN_NAME} \
    -Wl,-r \
    -Wl,-eapp_main \
    -Wl,--warn-unresolved-symbols \
    build/src/libsrc.a \
    -Wl,-whole-archive build/nano_parse/libnano_parse.a -Wl,-no-whole-archive \
    build/nano_lib/libnano_lib.a \
    ;
then
    echo "Successfully assembled ELF"
else
    echo "Failed assembling ELF"
    exit 1;
fi

# Remove unneccessary sections
if $STRIP; then
    gobjcopy --remove-section=.comment \
             --remove-section=.xtensa.info \
             --remove-section=.xt.prop \
             --remove-section=.rela.xt.prop \
             --remove-section=.xt.lit \
             --remove-section=.rela.xt.lit \
             --remove-section=.data \
             --remove-section=.bss \
             --remove-section=.text \
             $ELF_BIN_NAME
fi

#######################
# Convert ELF to JELF #
#######################
# todo: signing key argument
python3 elf2jelf/elf2jelf.py $ELF_BIN_NAME \
    --output $JELF_BIN_NAME \
    --coin $COIN_PATH \
    --bip32key $BIP32_KEY

#####################
# Compress the JELF #
#####################
JELF_SIZE=$(stat -f%z $JELF_BIN_NAME)
# Prepend the uncompressed file size to the compressed data
printf "0: %.8x" $JELF_SIZE | sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' | \
        xxd -r -g0 > $BIN_COMPRESSED_NAME
# perform compression
./heatshrink_bin -w 8 -l 4 $JELF_BIN_NAME >> $BIN_COMPRESSED_NAME

