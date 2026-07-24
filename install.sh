#!/bin/bash

# Sudo பர்மிஷன் உள்ளதா என சரிபார்க்க...
if [ "$EUID" -ne 0 ]; then
  echo "எச்சரிக்கை: Global ஆக இன்ஸ்டால் செய்ய Root உரிமை தேவை!"
  echo "தயவுசெய்து இதை 'sudo ./install.sh' என ரன் செய்யவும்."
  exit
fi

echo "BDH Linux IDE சிஸ்டம் முழுவதும் (Global) இன்ஸ்டால் செய்யப்படுகிறது..."

# Dependencies இன்ஸ்டால் செய்ய...
if [ -f "packages.txt" ]; then
    echo "தேவையான பேக்கேஜ்கள் இன்ஸ்டால் செய்யப்படுகின்றன..."
    pacman -S --needed $(cat packages.txt | tr '\n' ' ')
else
    echo "எச்சரிக்கை: packages.txt கோப்பு காணவில்லை!"
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
echo "Executable ஃபைல் செட் செய்யப்படுகிறது..."

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
echo "Kill கமாண்ட் உருவாக்கப்படுகிறது..."
cat << 'EOF' > "$INSTALL_DIR/bdh-ide-kill"
#!/bin/bash
tmux kill-server 2>/dev/null
echo -e "\e[1;32mBDH IDE செஷன்கள் வெற்றிகரமாக நிறுத்தப்பட்டன!\e[0m"
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
# 4. PostgreSQL Setup & bdh-db கமாண்டை இன்ஸ்டால் செய்தல் (புதிதாக சேர்க்கப்பட்டது)
# =========================================================
echo "PostgreSQL Database செட் செய்யப்படுகிறது..."

# Data Folder காலியாக இருந்தால் மட்டும் initdb கமாண்டை ரன் செய்யவும் (எரரைத் தவிர்க்க)
if [ -z "$(ls -A /var/lib/postgres/data 2>/dev/null)" ]; then
    su - postgres -c "initdb -D /var/lib/postgres/data"
    echo "PostgreSQL வெற்றிகரமாக Initialize செய்யப்பட்டது."
else
    echo "PostgreSQL ஏற்கனவே Initialize செய்யப்பட்டுள்ளது."
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
# 5. Configuration file-களை காப்பி செய்தல்
# =========================================================
echo "Config ஃபைல்கள் காப்பி செய்யப்படுகின்றன..."
cp configs/tmux.conf "$CONFIG_DIR/tmux.conf"
cp configs/nanorc "$CONFIG_DIR/nanorc"

# தமிழி ஸ்கிரிப்ட் ஃபைலை காப்பி செய்தல்
cp configs/ide_layout.tz "$CONFIG_DIR/ide_layout.tz"

# Config ஃபைல்களுக்கு அனைவருக்கும் படிக்கும் (Read) உரிமை கொடுக்க
chmod -R 755 "$CONFIG_DIR"

# 6. பழைய Tmux செஷனை அழித்தல் (புதிய அப்டேட்கள் உடனடியாக வேலை செய்ய)
echo "பழைய செஷன்கள் ரீசெட் செய்யப்படுகின்றன..."
if [ -n "$SUDO_USER" ]; then
    sudo -u $SUDO_USER tmux kill-server 2>/dev/null
else
    tmux kill-server 2>/dev/null
fi

echo "Global இன்ஸ்டாலேஷன் வெற்றிகரமாக முடிந்தது!"
echo "---------------------------------------------------"
echo "✅ IDE-ஐ திறக்க: bdh-ide"
echo "✅ IDE-ஐ முழுமையாக மூட: bdh-ide-kill"
echo "✅ Browser-ஐ திறக்க: bdh-browser <URL>"
echo "✅ Database-ஐ திறக்க: bdh-db"
echo "---------------------------------------------------"
