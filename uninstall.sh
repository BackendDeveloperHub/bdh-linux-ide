#!/bin/bash

if [ "$EUID" -ne 0 ]; then
  echo "தயவுசெய்து இதை 'sudo ./uninstall.sh' என ரன் செய்யவும்."
  exit 1
fi

echo "BDH Linux Sovereign IDE & Engine அன்-இன்ஸ்டால் செய்யப்படுகிறது..."

INSTALL_DIR="/usr/local/bin"
CONFIG_DIR="/etc/bdh-ide"

# 1. Main Executables & Engine-ஐ நீக்குதல்
BINARIES=("bdh-ide" "bdh-ide-kill" "bdh-terminal-engine" "bdh-browser" "bdh-db" "bdh-record")

for cmd in "${BINARIES[@]}"; do
    if [ -f "$INSTALL_DIR/$cmd" ]; then
        rm "$INSTALL_DIR/$cmd"
        echo "✅ $cmd நீக்கப்பட்டது."
    fi
done

# 2. Configuration போல்டரை நீக்குதல்
if [ -d "$CONFIG_DIR" ]; then
    rm -rf "$CONFIG_DIR"
    echo "✅ Configuration files ($CONFIG_DIR) நீக்கப்பட்டன."
fi

# 3. பின்னணியில் ஓடும் BDH இன்ஜின் ப்ராசஸை முழுமையாக நிறுத்துதல்
pkill -f bdh-terminal-engine 2>/dev/null
echo "✅ ஓடிக்கொண்டிருந்த BDH Engine ப்ராசஸ்கள் நிறுத்தப்பட்டன."

echo "---------------------------------------------------"
echo "BDH Linux Ecosystem வெற்றிகரமாக சிஸ்டமிலிருந்து நீக்கப்பட்டது!"
echo "---------------------------------------------------"
