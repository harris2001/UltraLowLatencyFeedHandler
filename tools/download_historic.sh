#!/usr/bin/env bash
set -euo pipefail

echo "Downloading smallest recent ITCH Nasdaq file from emi"

file_="12302019.NASDAQ_ITCH50.gz"
base_url="https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH"

wget "${base_url}/${file_}"
if [ ! -f "${file_}" ]; then
    echo "Error: File ${file_} not found."
    exit 1
fi

wget "${base_url}/${file_}.md5sum"
if [ ! -f "${file_}.md5sum" ]; then
    echo "Error: File ${file_}.md5sum not found."
    exit 1
fi

if ! md5sum -c "${file_}.md5sum"; then
    echo "Error: MD5 checksum does not match."
    exit 1
fi

echo "Checksum verified. Decompressing..."
if ! gunzip "${file_}"; then
    echo "Error: Failed to decompress ${file_}"
    exit 1
fi

echo "Done: ${file_%.gz} is ready."