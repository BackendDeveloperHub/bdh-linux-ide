#!/bin/bash
# =========================================================
# 🚀 BDH LINUX ECOSYSTEM — UNIVERSAL SMART INSTALLER
# =========================================================

echo -e "\e[1;36m+===================================================+\e[0m"
echo -e "\e[1;32m     🔥 INSTALLING BDH SOVEREIGN DEV STACK 🔥      \e[0m"
echo -e "\e[1;36m+===================================================+\e[0m"

# 0. Environment Detection (Arch Linux vs Termux)
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

# 1. Package Installation based on Environment
echo -e "\n\e[1;33m[1/4] Installing Required Packages...\e[0m"
if [ "$IS_TERMUX" = true ]; then
    pkg update -y
    pkg install -y make clang git postgresql qrencode zsh fzf bat eza
else
    # Arch Linux (Root check)
    if [ "$EUID" -ne 0 ]; then
        echo "❌ Error: Please run this script with sudo on Arch Linux ('sudo ./install.sh')."
        exit 1
    fi
    pacman -S --needed --noconfirm base-devel git cage zsh fzf bat eza postgresql qrencode gcc
fi

# 2. Clone & Build BDH Terminal Engine
echo -e "\n\e[1;33m[2/4] Cloning & Building bdh-terminal-engine...\e[0m"
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

# 3. Compile & Install BDH-Tree (File Manager)
echo -e "\n\e[1;33m[3/4] Compiling BDH-Tree...\t\t\e[0m"
# ஸ்கிரிப்ட் இருக்கும் ஒரிஜினல் ஃபோல்டருக்குத் திரும்புதல்
cd - > /dev/null || exit 1

if [ -f "bdh-tree.c" ]; then
    CC_COMP_TOOL="gcc"
    [ "$IS_TERMUX" = true ] && CC_COMP_TOOL="clang"
    
    $CC_COMP_TOOL bdh-tree.c -o bdh-tree
    if [ -f "bdh-tree" ]; then
        cp -f bdh-tree "$INSTALL_DIR/bdh-tree"
        chmod +x "$INSTALL_DIR/bdh-tree"
        echo -e "  -> bdh-tree compiled and installed!"
    else
        echo -e "\e[1;31m❌ BDH-Tree compilation failed!\e[0m"
        exit 1
    fi
else
    echo -e "\e[1;33m⚠️ bdh-tree.c not found. Skipping.\e[0m"
fi

# 4. Compile & Install BDH-IDE Master Controller (Split-Screen)
echo -e "\n\e[1;33m[4/4] Compiling & Installing BDH-IDE Master Controller...\e[0m"
if [ -f "bdh-ide.c" ]; then
    CC_COMP_TOOL="gcc"
    [ "$IS_TERMUX" = true ] && CC_COMP_TOOL="clang"
    
    $CC_COMP_TOOL bdh-ide.c -o bdh-ide
    if [ -f "bdh-ide" ]; then
        cp -f bdh-ide "$INSTALL_DIR/bdh-ide"
        chmod +x "$INSTALL_DIR/bdh-ide"
        echo -e "\e[1;32m✅ C-based BDH-IDE Master Controller installed successfully!\e[0m"
    else
        echo -e "\e[1;31m❌ BDH-IDE compilation failed!\e[0m"
        exit 1
    fi
else
    echo -e "\e[1;31m❌ Error: bdh-ide.c not found in current directory!\e[0m"
    exit 1
fi

echo -e "\n\e[1;32m---------------------------------------------------\e[0m"
echo -e "✅ \e[1;32mBDH Ecosystem Setup Completed Successfully!\e[0m"
if [ "$IS_TERMUX" = true ]; then
    echo -e "✅ \e[1;36mRun 'bdh-ide' to start your IDE!\e[0m"
else
    echo -e "✅ \e[1;36mRun 'cage -s -- bdh-ide' to launch Kiosk Mode!\e[0m"
fi
echo -e "---------------------------------------------------"
