#!/bin/bash

# Sudo பர்மிஷன் உள்ளதா என சரிபார்க்க...
if [ "$EUID" -ne 0 ]; then
  echo "Warning: Global installation requires Root privilege!"
  echo "Please run this as 'sudo ./install.sh'."
  exit
fi

echo "Installing BDH Linux IDE globally..."

# Dependencies இன்ஸ்டால் செய்ய...
if [ -f "packages.txt" ]; then
    echo "Installing required packages..."
    pacman -S --needed $(cat packages.txt | tr '\n' ' ')
else
    echo "Warning: packages.txt not found!"
fi

# Global Paths
INSTALL_DIR="/usr/local/bin"
CONFIG_DIR="/etc/bdh-ide"

# Directories உருவாக்குதல்
mkdir -p "$INSTALL_DIR"
mkdir -p "$CONFIG_DIR"

# =========================================================
# 1. Main executable-ஐ காப்பி செய்தல்
# =========================================================
echo "Setting up main executable..."

# காப்பி செய்வதற்கு முன்பே அசல் ஃபைலில் உள்ள \r பிழையை நிரந்தரமாக நீக்க:
sed -i 's/\r$//' bdh-ide

# இப்போது சுத்தமான ஃபைலை சிஸ்டம் போல்டருக்கு காப்பி செய்ய:
cp bdh-ide "$INSTALL_DIR/bdh-ide"

# ஆட்டோமேட்டிக்காக Execute Permission கொடுப்பதற்கு
chmod +x "$INSTALL_DIR/bdh-ide"
# =========================================================

# =========================================================
# 2. bdh-ide-kill கமாண்டை உருவாக்குதல் 
# =========================================================
echo "Creating kill command..."
cat << 'EOF' > "$INSTALL_DIR/bdh-ide-kill"
#!/bin/bash
tmux kill-server 2>/dev/null
echo -e "\e[1;32mBDH IDE sessions stopped successfully!\e[0m"
EOF
chmod +x "$INSTALL_DIR/bdh-ide-kill"
# =========================================================

# =========================================================
# 3. bdh-browser கமாண்டை இன்ஸ்டால் செய்தல் 
# =========================================================
echo "Installing bdh-browser..."
sed -i 's/\r$//' bdh-browser 2>/dev/null # Windows/CRLF பிழையைத் தவிர்க்க
cp bdh-browser "$INSTALL_DIR/bdh-browser"
chmod +x "$INSTALL_DIR/bdh-browser"
# =========================================================

# =========================================================
# 4. PostgreSQL Setup & bdh-db கமாண்டை இன்ஸ்டால் செய்தல் 
# =========================================================
echo "Setting up PostgreSQL Database..."

# Data Folder காலியாக இருந்தால் மட்டும் initdb கமாண்டை ரன் செய்யவும் (எரரைத் தவிர்க்க)
if [ -z "$(ls -A /var/lib/postgres/data 2>/dev/null)" ]; then
    su - postgres -c "initdb -D /var/lib/postgres/data"
    echo "PostgreSQL initialized successfully."
else
    echo "PostgreSQL is already initialized."
fi

# டேட்டாபேஸ் சர்வீஸை Start மற்றும் Enable செய்ய
systemctl enable --now postgresql

# bdh-db கமாண்டை சிஸ்டமில் இன்ஸ்டால் செய்ய
echo "Installing bdh-db shortcut..."
sed -i 's/\r$//' bdh-db 2>/dev/null
cp bdh-db "$INSTALL_DIR/bdh-db"
chmod +x "$INSTALL_DIR/bdh-db"
# =========================================================

# =========================================================
# 5. bdh-record கமாண்டை இன்ஸ்டால் செய்தல் 
# =========================================================
echo "Installing bdh-record shortcut..."
sed -i 's/\r$//' bdh-record 2>/dev/null
cp bdh-record "$INSTALL_DIR/bdh-record"
chmod +x "$INSTALL_DIR/bdh-record"
# =========================================================

# =========================================================
# 6. Configuration file-களை காப்பி செய்தல்
# =========================================================
echo "Copying configuration files..."
cp configs/tmux.conf "$CONFIG_DIR/tmux.conf"
cp configs/nanorc "$CONFIG_DIR/nanorc"

# தமிழி ஸ்கிரிப்ட் ஃபைலை காப்பி செய்தல்
cp configs/ide_layout.tz "$CONFIG_DIR/ide_layout.tz"

# Config ஃபைல்களுக்கு அனைவருக்கும் படிக்கும் (Read) உரிமை கொடுக்க
chmod -R 755 "$CONFIG_DIR"
# =========================================================

# =========================================================
# 7. Ranger-ல் தமிழி (.tz) ஃபைலை Nano-வில் திறக்க ஆட்டோ-செட்டப்
# =========================================================
echo "Configuring Ranger for Tamizhi (.tz) files..."

if [ -n "$SUDO_USER" ]; then
    USER_HOME=$(eval echo ~$SUDO_USER)
    
    # Ranger config ஃபோல்டரை யூசர் பெயரில் உருவாக்குதல்
    sudo -u $SUDO_USER mkdir -p "$USER_HOME/.config/ranger"
    
    # ஃபைலில் ஏற்கனவே இந்த ரூல் இருக்கிறதா என செக் செய்து, இல்லை என்றால் மட்டும் சேர்க்க
    if ! grep -q "ext tz" "$USER_HOME/.config/ranger/rifle.conf" 2>/dev/null; then
        echo 'ext tz = nano "$@"' | sudo -u $SUDO_USER tee -a "$USER_HOME/.config/ranger/rifle.conf" > /dev/null
    fi
fi
# =========================================================

# =========================================================
# 8. பழைய Tmux செஷனை அழித்தல் (புதிய அப்டேட்கள் உடனடியாக வேலை செய்ய)
# =========================================================
echo "Resetting old sessions..."
if [ -n "$SUDO_USER" ]; then
    sudo -u $SUDO_USER tmux kill-server 2>/dev/null
else
    tmux kill-server 2>/dev/null
fi

echo "Global install complete!"
echo "---------------------------------------------------"
echo "✅ IDE open: bdh-ide"
echo "✅ IDE full close : bdh-ide-kill"
echo "✅ Browser open: bdh-browser <URL>"
echo "✅ Database- postgresql open: bdh-db"
echo "✅ Record screen: bdh-record"
echo "---------------------------------------------------"
