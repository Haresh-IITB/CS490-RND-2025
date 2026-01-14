#!/usr/bin/env bash

set -e

CONFIG_FILE="config.txt"
EXEC="./bin/benchmark-stepsize"
TIMEOUT="1h"

NODE_SIZES=(512 1024 2048)

echo "=== Benchmark started at $(date) ==="

for N in "${NODE_SIZES[@]}"; do
    echo "-----------------------------------"
    echo "Running experiment with node_sizes=${N}"

    # Update node_sizes in config.txt
    sed -i.bak -E "s/^node_sizes=.*/node_sizes=${N}/" "$CONFIG_FILE"

    echo "Config updated:"
    grep "^node_sizes=" "$CONFIG_FILE"

    # Run with timeout
    echo "Running with timeout ${TIMEOUT}..."
    if timeout "${TIMEOUT}" "$EXEC"; then
        echo "✔ Completed node_sizes=${N}"
    else
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 124 ]; then
            echo "⏰ Timeout reached for node_sizes=${N} (1 hour). Skipping."
        else
            echo "❌ Program failed with exit code ${EXIT_CODE}"
            exit $EXIT_CODE
        fi
    fi
done

echo "=== Benchmark finished at $(date) ==="
