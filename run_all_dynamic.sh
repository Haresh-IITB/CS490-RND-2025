#!/bin/bash
set -e

BIN_DIR="./bin"
CFG_DIR="./config"

# Define Arrays using parentheses ()
EXECUTABLES=(
    "dynamic-lptkr"
    "dynamic-lpirp"
    "dynamic-greedy"
    "dynamic-hillclimbing"
    "dynamic-localSearch"
)

CONFIGS=(
    "config-lptkr.txt"
    "config-lpirp.txt"
    "config-greedy.txt"
    "config-hillc.txt"
    "config-ls.txt"
)

# Get the number of items in the array
COUNT=${#EXECUTABLES[@]}

# Loop using the index
for (( i=0; i<COUNT; i++ )); do
    exe="${EXECUTABLES[$i]}"
    cfg="${CONFIGS[$i]}"

    BIN="$BIN_DIR/$exe"
    CFG="$CFG_DIR/$cfg"

    echo "======================================"
    echo "Running $exe using $cfg"
    echo "======================================"

    if [ ! -x "$BIN" ]; then
        echo "Skipping: $BIN not executable (or not found)"
        continue
    fi

    if [ ! -f "$CFG" ]; then
        echo "Skipping: $CFG not found"
        continue
    fi

    # Backup config
    cp "$CFG" "$CFG.bak"

    # ---- IC run ----
    sed 's/^diffusion_model=.*/diffusion_model=IC/' "$CFG" > "${CFG}.tmp" && mv "${CFG}.tmp" "$CFG"
    "$BIN"

    # ---- LT run ----
    sed 's/^diffusion_model=.*/diffusion_model=LT/' "$CFG" > "${CFG}.tmp" && mv "${CFG}.tmp" "$CFG"
    "$BIN"

    # Restore config
    mv "$CFG.bak" "$CFG"
done

echo "All runs completed"