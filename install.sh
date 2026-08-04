#!/bin/bash

# =========================================================
# 0. Environment Detection (Arch Linux vs Termux)
# =========================================================
if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=true
    echo "📱 Termux Environment Detected..."
else
    IS_TERMUX=false
    echo "💻 Arch Linux / Standard Linux Environment Detected..."
fi

# Arch Linux-ல் மட்டும் Root (sudo) பர்மிஷன் உள்ளதா என சரிபார்க்க...
if [ "$IS_TERMUX" = false ] && [ "$EUID" -ne 0 ]; then
    echo "Warning: Global installation requires Root privilege!"
    echo "Please run this as 'sudo ./install.sh'."
    exit 1
fi

echo "Installing BDH Linux IDE..."

# =========================================================
# 1. Dynamic Variables Configuration
# =========================================================
if [ "$IS_TERMUX" = true ]; then
    # Termux Paths & User
    INSTALL_DIR="$PREFIX/bin"
    CONFIG_DIR="$PREFIX/etc/bdh-ide"
    PG_DATA="$PREFIX/var/lib/postgresql"
    TARGET_USER=$(whoami)
    TARGET_HOME="$HOME"
else
    # Arch Linux Paths & User
    INSTALL_DIR="/usr/local/bin"
    CONFIG_DIR="/etc/bdh-ide"
    PG_DATA="/var/lib/postgres/data"
    TARGET_USER="$SUDO_USER"
    TARGET_HOME=$(eval echo ~$SUDO_USER)
fi

# Directories உருவாக்குதல்
mkdir -p "$INSTALL_DIR"
mkdir -p "$CONFIG_DIR"

# =========================================================
# 2. Package Dependencies இன்ஸ்டால் செய்தல்
# =========================================================
if [ -f "packages.txt" ]; then
    echo "Installing required packages..."
    if [ "$IS_TERMUX" = true ]; then
        pkg update -y
        pkg install -y $(cat packages.txt | tr '\n' ' ')
    else
        pacman -S --needed --noconfirm $(cat packages.txt | tr '\n' ' ')
    fi
else
    echo "Warning: packages.txt not found!"
fi

# =========================================================
# 3. Main Executable & Browser Shortcut
# =========================================================
echo "Setting up main executables..."

sed -i 's/\r$//' bdh-ide 2>/dev/null
cp bdh-ide "$INSTALL_DIR/bdh-ide"
chmod +x "$INSTALL_DIR/bdh-ide"

sed -i 's/\r$//' bdh-browser 2>/dev/null
cp bdh-browser "$INSTALL_DIR/bdh-browser" 2>/dev/null || echo "bdh-browser skipped (not found)"
[ -f "$INSTALL_DIR/bdh-browser" ] && chmod +x "$INSTALL_DIR/bdh-browser"

# =========================================================
# 4. bdh-ide-kill கமாண்டை உருவாக்குதல் 
# =========================================================
echo "Creating kill command..."
cat << 'EOF' > "$INSTALL_DIR/bdh-ide-kill"
#!/bin/bash
tmux kill-server 2>/dev/null
echo -e "\e[1;32mBDH IDE sessions stopped successfully!\e[0m"
EOF
chmod +x "$INSTALL_DIR/bdh-ide-kill"

# =========================================================
# 5. PostgreSQL Setup & bdh-db கமாண்டு
# =========================================================
echo "Setting up PostgreSQL Database..."

if [ "$IS_TERMUX" = true ]; then
    # Termux-க்கான PostgreSQL Setup
    mkdir -p "$PG_DATA"
    if [ -z "$(ls -A "$PG_DATA" 2>/dev/null)" ]; then
        initdb "$PG_DATA"
        echo "PostgreSQL initialized for Termux."
    else
        echo "PostgreSQL is already initialized."
    fi
    echo "Note: In Termux, start DB manually using: pg_ctl -D $PG_DATA start"
else
    # Arch Linux-க்கான PostgreSQL Setup
    if [ -z "$(ls -A "$PG_DATA" 2>/dev/null)" ]; then
        su - postgres -c "initdb -D $PG_DATA"
        echo "PostgreSQL initialized successfully."
    else
        echo "PostgreSQL is already initialized."
    fi
    systemctl enable --now postgresql
fi

sed -i 's/\r$//' bdh-db 2>/dev/null
cp bdh-db "$INSTALL_DIR/bdh-db" 2>/dev/null || echo "bdh-db skipped (not found)"
[ -f "$INSTALL_DIR/bdh-db" ] && chmod +x "$INSTALL_DIR/bdh-db"

# =========================================================
# 6. bdh-record கமாண்டை இன்ஸ்டால் செய்தல் 
# =========================================================
echo "Installing bdh-record shortcut..."
sed -i 's/\r$//' bdh-record 2>/dev/null
cp bdh-record "$INSTALL_DIR/bdh-record" 2>/dev/null || echo "bdh-record skipped (not found)"
[ -f "$INSTALL_DIR/bdh-record" ] && chmod +x "$INSTALL_DIR/bdh-record"

# =========================================================
# 7. Configuration file-களை காப்பி செய்தல்
# =========================================================
echo "Copying configuration files..."
cp configs/tmux.conf "$CONFIG_DIR/tmux.conf" 2>/dev/null
cp configs/nanorc "$CONFIG_DIR/nanorc" 2>/dev/null
cp configs/ide_layout.tz "$CONFIG_DIR/ide_layout.tz" 2>/dev/null

chmod -R 755 "$CONFIG_DIR"

# =========================================================
# 8. Ranger-ல் தமிழி (.tz) ஃபைலை Nano-வில் திறக்க செட்டப்
# =========================================================
echo "Configuring Ranger for Tamizhi (.tz) files..."

if [ -n "$TARGET_HOME" ]; then
    RANGER_CONF_DIR="$TARGET_HOME/.config/ranger"
    
    if [ "$IS_TERMUX" = true ]; then
        mkdir -p "$RANGER_CONF_DIR"
        if ! grep -q "ext tz" "$RANGER_CONF_DIR/rifle.conf" 2>/dev/null; then
            echo 'ext tz = nano "$@"' >> "$RANGER_CONF_DIR/rifle.conf"
        fi
    else
        sudo -u "$TARGET_USER" mkdir -p "$RANGER_CONF_DIR"
        if ! grep -q "ext tz" "$RANGER_CONF_DIR/rifle.conf" 2>/dev/null; then
            echo 'ext tz = nano "$@"' | sudo -u "$TARGET_USER" tee -a "$RANGER_CONF_DIR/rifle.conf" > /dev/null
        fi
    fi
fi

# =========================================================
# 9. பழைய Tmux செஷனை அழித்தல்
# =========================================================
echo "Resetting old sessions..."
if [ "$IS_TERMUX" = true ] || [ -z "$SUDO_USER" ]; then
    tmux kill-server 2>/dev/null
else
    sudo -u "$TARGET_USER" tmux kill-server 2>/dev/null
fi

echo "---------------------------------------------------"
echo "✅ Installation Complete for $TARGET_USER!"
echo "✅ IDE open: bdh-ide"
echo "✅ IDE full close : bdh-ide-kill"
echo "✅ Browser open: bdh-browser <URL>"
echo "✅ Database open: bdh-db"
echo "✅ Record screen: bdh-record"
echo "---------------------------------------------------"
