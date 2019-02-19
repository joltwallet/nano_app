# App Specific Parameters
APP_BASENAME=Nano
COIN_PATH="44'/165'"
BIP32_KEY="ed25519 seed"
ELF2JELF="jolt_wallet/elf2jelf/elf2jelf.py"
PYTHONBIN="python3"

# Convert names to absolute paths and stuff
ELF_BIN_NAME=${APP_BASENAME}.elf
JELF_BIN_NAME=${APP_BASENAME}.jelf
JELF_BIN_COMPRESSED_NAME=${JELF_BIN_NAME}.gz
ELF_BIN_NAME=$(realpath $ELF_BIN_NAME)
JELF_BIN_NAME=$(realpath $JELF_BIN_NAME)
JELF_BIN_COMPRESSED_NAME=$(realpath $JELF_BIN_COMPRESSED_NAME)
ELF2JELF=$(realpath $ELF2JELF)

# OS specific configurations
#if [[ "$OSTYPE" == "linux-gnu" ]]; then
#    # ...
#    echo "linux-gnu detected."
#    OBJCOPY=objcopy
#elif [[ "$OSTYPE" == "darwin"* ]]; then
#    # Mac OSX
#    echo "MacOS detected."
#    OBJCOPY=gobjcopy
#elif [[ "$OSTYPE" == "cygwin" ]]; then
#    # POSIX compatibility layer and Linux environment emulation for Windows
#    echo "cygwin detected."
#    exit 1
#elif [[ "$OSTYPE" == "msys" ]]; then
#    # Lightweight shell and GNU utilities compiled for Windows (part of MinGW)
#    echo "msys detected."
#    exit 1
#elif [[ "$OSTYPE" == "win32" ]]; then
#    # I'm not sure this can happen.
#    echo "win32 detected."
#    exit 1
#elif [[ "$OSTYPE" == "freebsd"* ]]; then
#    # ...
#    echo "freeBSD detected."
#    exit 1
#else
#    # Unknown.
#    echo "Error detecting operating system."
#    exit 1
#fi

printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

if [ -f ${ELF_BIN_NAME} ] ; then
    rm ${ELF_BIN_NAME}
fi
if [ -f ${JELF_BIN_NAME} ] ; then
    rm ${JELF_BIN_NAME}
fi
if [ -f ${JELF_BIN_COMPRESSED_NAME} ] ; then
    rm ${JELF_BIN_COMPRESSED_NAME}
fi

make app -j15

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
xtensa-esp32-elf-objcopy --remove-section=.comment \
         --remove-section=.xtensa.info \
         --remove-section=.xt.prop \
         --remove-section=.rela.xt.prop \
         --remove-section=.xt.lit \
         --remove-section=.rela.xt.lit \
         --remove-section=.data \
         --remove-section=.bss \
         --remove-section=.text \
         $ELF_BIN_NAME

#######################
# Convert ELF to JELF #
#######################
# todo: signing key argument
if $PYTHONBIN $ELF2JELF  "$ELF_BIN_NAME" \
    --output "$JELF_BIN_NAME" \
    --coin "$COIN_PATH" \
    --bip32key "$BIP32_KEY" \
    --verbose DEBUG \
    ;
then
    echo "Successfully converted ELF to JELF"
else
    echo "Failed converting ELF to JELF"
    exit 1;
fi
