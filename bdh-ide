#!/bin/bash
# =========================================================
# 🚀 BDH LINUX ECOSYSTEM — OFFICIAL INSTALLER
# =========================================================

echo -e "\e[1;36m+===================================================+\e[0m"
echo -e "\e[1;32m     🔥 INSTALLING BDH SOVEREIGN DEV STACK 🔥      \e[0m"
echo -e "\e[1;36m+===================================================+\e[0m"

# 1. Base Developer Tools & Wayland Kiosk (NO TMUX!)
echo -e "\n\e[1;33m[1/4] Installing Base Tools (cage, zsh, fzf, bat, eza, postgresql, qrencode)...\e[0m"
sudo pacman -S --needed --noconfirm base-devel git cage zsh fzf bat eza postgresql qrencode gcc

# 2. Clone & Build BDH Terminal Engine (Pure POSIX C)
echo -e "\n\e[1;33m[2/4] Cloning & Building bdh-terminal-engine...\e[0m"
cd /tmp || exit
rm -rf bdh-terminal-engine
git clone https://github.com/BackendDeveloperHub/bdh-terminal-engine.git
cd bdh-terminal-engine || exit

make clean && make
sudo install -m 755 bdh-engine /usr/local/bin/bdh-terminal-engine

# 3. Compile BDH-Tree (File Manager)
echo -e "\n\e[1;33m[3/4] Compiling BDH-Tree...\e[0m"
cd - || exit
if [ -f "bdh-tree.c" ]; then
    gcc bdh-tree.c -o bdh-tree
    sudo install -m 755 bdh-tree /usr/local/bin/bdh-tree
    echo -e "  -> bdh-tree compiled and installed!"
fi

# 4. Compile & Install BDH-IDE Master Controller (Split-Screen C Program)
echo -e "\n\e[1;33m[4/4] Compiling & Installing BDH-IDE Master Controller...\e[0m"
if [ -f "bdh-ide.c" ]; then
    gcc bdh-ide.c -o bdh-ide
    sudo install -m 755 bdh-ide /usr/local/bin/bdh-ide
    echo -e "\e[1;32m✅ C-based BDH-IDE Master Controller installed successfully!\e[0m"
else
    echo -e "\e[1;31m❌ Error: bdh-ide.c not found! Please create bdh-ide.c first.\e[0m"
    exit 1
fi

echo -e "\n\e[1;32m✅ BDH Linux Ecosystem Setup Completed Successfully!\e[0m"
echo -e "\e[1;36m   Run 'cage -s -- bdh-ide' to launch Kiosk Mode!\e[0m"
