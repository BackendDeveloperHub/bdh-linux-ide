#!/bin/bash
# =========================================================
# 🚀 BDH LINUX IDE — SMART AUTOMATED INSTALLER
# =========================================================

# 0. Environment Detection (Arch Linux vs Termux)
if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=true
    echo "📱 Termux Environment Detected..."
else
    IS_TERMUX=false
    echo "💻 Arch Linux / Standard Linux Environment Detected..."
fi

# Root Permission Check for Standard Linux
if [ "$IS_TERMUX" = false ] && [ "$EUID" -ne 0 ]; then
    echo "Warning: Global installation requires Root privilege!"
    echo "Please run this as 'sudo ./install.sh'."
    exit 1
fi

echo -e "\e[1;32mInstalling BDH Linux Sovereign IDE...\e[0m"

# 1. Dynamic Variables Configuration
if [ "$IS_TERMUX" = true ]; then
    INSTALL_DIR="$PREFIX/bin"
    CONFIG_DIR="$PREFIX/etc/bdh-ide"
    PG_DATA="$PREFIX/var/lib/postgresql"
    TMP_DIR="${TMPDIR:-$PREFIX/tmp}"
    TARGET_USER=$(whoami)
    TARGET_HOME="$HOME"
else
    INSTALL_DIR="/usr/local/bin"
    CONFIG_DIR="/etc/bdh-ide"
    PG_DATA="/var/lib/postgres/data"
    TMP_DIR="/tmp"
    TARGET_USER="$SUDO_USER"
    TARGET_HOME=$(eval echo ~$SUDO_USER)
fi

mkdir -p "$INSTALL_DIR"
mkdir -p "$CONFIG_DIR"
mkdir -p "$TMP_DIR"

# 2. Package Dependencies (No tmux!)
if [ -f "packages.txt" ]; then
    echo "Installing required packages..."
    if [ "$IS_TERMUX" = true ]; then
        pkg update -y
        pkg install -y $(cat packages.txt | tr '\n' ' ') make clang git
    else
        pacman -S --needed --noconfirm $(cat packages.txt | tr '\n' ' ') base-devel git
    fi
fi

# =========================================================
# 3. 🔥 SMART AUTO-DETECT & BUILD BDH-TERMINAL-ENGINE
# =========================================================
echo "Checking BDH Terminal Engine..."

if ! command -v bdh-terminal-engine &> /dev/null && [ ! -f "$INSTALL_DIR/bdh-terminal-engine" ]; then
    echo -e "\e[1;33m⚡ bdh-terminal-engine not found! Auto-cloning & building...\e[0m"
    CURRENT_DIR="$PWD"
    cd "$TMP_DIR" || exit 1
    rm -rf bdh-terminal-engine
    git clone https://github.com/BackendDeveloperHub/bdh-terminal-engine.git
    cd bdh-terminal-engine || { echo "Git Clone Failed!"; exit 1; }
    
    make clean && make || { echo "Engine Build Failed!"; exit 1; }
    cp -f bdh-engine "$INSTALL_DIR/bdh-terminal-engine"
    chmod 755 "$INSTALL_DIR/bdh-terminal-engine"
    cd "$CURRENT_DIR" || exit 1
    echo -e "\e[1;32m✅ BDH Terminal Engine installed globally!\e[0m"
else
    echo -e "\e[1;32m✅ BDH Terminal Engine is already installed. Using existing engine!\e[0m"
fi

# =========================================================
# 4. 🔥 DYNAMIC GENERATION OF BDH-IDE LAUNCHER (ZERO BUG)
# =========================================================
echo "Creating clean Sovereign IDE Launcher..."
cat << 'EOF' > "$INSTALL_DIR/bdh-ide"
#!/bin/bash
export BDH_IDE_ACTIVE=1
export EDITOR="nano"
export BROWSER="bdh-browser"

clear
echo -e "\e[1;32m  🚀 Starting BDH Sovereign Terminal Engine...\e[0m"
sleep 0.3
exec bdh-terminal-engine "$@"
EOF
chmod +x "$INSTALL_DIR/bdh-ide"

# 5. Create bdh-ide-kill Command
cat << 'EOF' > "$INSTALL_DIR/bdh-ide-kill"
#!/bin/bash
pkill -f bdh-terminal-engine 2>/dev/null
echo -e "\e[1;32mBDH Sovereign IDE sessions stopped successfully!\e[0m"
EOF
chmod +x "$INSTALL_DIR/bdh-ide-kill"

# 6. Extra Shortcuts (Browser, DB, Record)
for cmd in "bdh-browser" "bdh-db" "bdh-record"; do
    if [ -f "$cmd" ]; then
        sed -i 's/\r$//' "$cmd" 2>/dev/null
        cp "$cmd" "$INSTALL_DIR/$cmd"
        chmod +x "$INSTALL_DIR/$cmd"
    fi
done

# 7. PostgreSQL Setup
echo "Setting up PostgreSQL Database..."
if [ "$IS_TERMUX" = true ]; then
    mkdir -p "$PG_DATA"
    [ -z "$(ls -A "$PG_DATA" 2>/dev/null)" ] && initdb "$PG_DATA"
else
    if [ -z "$(ls -A "$PG_DATA" 2>/dev/null)" ]; then
        su - postgres -c "initdb -D $PG_DATA"
    fi
    systemctl enable --now postgresql 2>/dev/null
fi

# 8. Copy Configuration files
echo "Copying configuration files..."
cp configs/nanorc "$CONFIG_DIR/nanorc" 2>/dev/null
cp configs/ide_layout.tz "$CONFIG_DIR/ide_layout.tz" 2>/dev/null
chmod -R 755 "$CONFIG_DIR"

# 9. Ranger setup for Tamizhi (.tz)
if [ -n "$TARGET_HOME" ]; then
    RANGER_CONF_DIR="$TARGET_HOME/.config/ranger"
    mkdir -p "$RANGER_CONF_DIR" 2>/dev/null
    if ! grep -q "ext tz" "$RANGER_CONF_DIR/rifle.conf" 2>/dev/null; then
        echo 'ext tz = nano "$@"' >> "$RANGER_CONF_DIR/rifle.conf"
    fi
fi

# 10. Reset Old Sessions
pkill -f bdh-terminal-engine 2>/dev/null

echo "---------------------------------------------------"
echo -e "✅ \e[1;32mInstallation Complete for $TARGET_USER!\e[0m"
echo -e "✅ \e[1;36mIDE open        : bdh-ide\e[0m"
echo -e "✅ \e[1;36mIDE full close  : bdh-ide-kill\e[0m"
echo "---------------------------------------------------"
