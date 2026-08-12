#!/bin/bash
# =========================================================
# 🚀 BDH LINUX ECOSYSTEM — UNIVERSAL SMART INSTALLER
# =========================================================

echo -e "\e[1;36m+===================================================+\e[0m"
echo -e "\e[1;32m     🔥 INSTALLING BDH SOVEREIGN DEV STACK 🔥      \e[0m"
echo -e "\e[1;36m+===================================================+\e[0m"

# 0. Environment Detection & Repo Path Saving
REPO_DIR="$PWD"

if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=true
    INSTALL_DIR="$PREFIX/bin"
    BUILD_TMP="${TMPDIR:-$PREFIX/tmp}"
    echo -e "\n\e[1;35m📱 Termux Environment Detected...\e[0m"
else
    IS_TERMUX=false
    INSTALL_DIR="/usr/local/bin"
    BUILD_TMP="/tmp"
    echo -e "\n\e[1;35m💻 Arch Linux / Standard Linux Environment Detected...\e[0m"
fi

# 1. Package Installation
echo -e "\n\e[1;33m[1/3] Installing Required Packages...\e[0m"
if [ "$IS_TERMUX" = true ]; then
    pkg update -y
    pkg install -y make clang git postgresql zsh fzf bat eza
else
    if [ "$EUID" -ne 0 ]; then
        echo "❌ Error: Please run with sudo on Arch Linux ('sudo ./install.sh')."
        exit 1
    fi
    pacman -S --needed --noconfirm base-devel git cage zsh fzf bat eza postgresql qrencode gcc
fi

# 2. Clone & Build BDH Terminal Engine
echo -e "\n\e[1;33m[2/3] Cloning & Building bdh-terminal-engine...\e[0m"
mkdir -p "$BUILD_TMP"
cd "$BUILD_TMP" || exit 1
rm -rf bdh-terminal-engine
git clone https://github.com/BackendDeveloperHub/bdh-terminal-engine.git
cd bdh-terminal-engine || { echo "❌ Git Clone Failed!"; exit 1; }

make clean && make || { echo "❌ Engine Build Failed!"; exit 1; }

mkdir -p "$INSTALL_DIR"
cp -f bdh-engine "$INSTALL_DIR/bdh-terminal-engine"
chmod 755 "$INSTALL_DIR/bdh-terminal-engine"
echo -e "\e[1;32m  -> bdh-terminal-engine installed to $INSTALL_DIR!\e[0m"

# 3. Compile & Install BDH-IDE (Modular C Project)
echo -e "\n\e[1;33m[3/3] Compiling & Installing BDH-IDE Modular System...\e[0m"
cd "$REPO_DIR" || exit 1

# Makefile மூலமாக ப்ராஜெக்ட்டைக் கம்பைல் செய்கிறோம்
if [ "$IS_TERMUX" = true ]; then
    echo -e "  -> Running make (with clang for Termux)..."
    make clean && make CC=clang || { echo -e "\e[1;31m❌ BDH-IDE compilation via Makefile failed!\e[0m"; exit 1; }
else
    echo -e "  -> Running make..."
    make clean && make || { echo -e "\e[1;31m❌ BDH-IDE compilation via Makefile failed!\e[0m"; exit 1; }
fi

# கம்பைல் ஆன பைனரியை bin/ ஃபோல்டரில் இருந்து Install Directory-க்கு மாற்றுகிறோம்
if [ -f "$REPO_DIR/bin/bdh-linux-ide" ]; then
    cp -f "$REPO_DIR/bin/bdh-linux-ide" "$INSTALL_DIR/bdh-ide"
    chmod +x "$INSTALL_DIR/bdh-ide"
    echo -e "\e[1;32m✅ Modular C-based BDH-IDE installed successfully!\e[0m"
else
    echo -e "\e[1;31m❌ Error: Compiled binary not found in bin/!\e[0m"
    exit 1
fi

echo -e "\n\e[1;32m---------------------------------------------------\e[0m"
echo -e "✅ \e[1;32mBDH Modular Ecosystem Setup Completed Successfully!\e[0m"
echo -e "✅ \e[1;36mRun 'bdh-ide' to start your IDE!\e[0m"
echo -e "---------------------------------------------------"
