# =========================================================
# 5. 🔥 COMPILE & INSTALL BDH-IDE (SPLIT-SCREEN MASTER)
# =========================================================
echo "Compiling BDH-IDE Master Controller..."
if [ -f "bdh-ide.c" ]; then
    CC_COMPILER="gcc"
    [ "$IS_TERMUX" = true ] && CC_COMPILER="clang"
    
    $CC_COMPILER bdh-ide.c -o bdh-ide
    
    if [ -f "bdh-ide" ]; then
        cp -f bdh-ide "$INSTALL_DIR/bdh-ide"
        chmod +x "$INSTALL_DIR/bdh-ide"
        echo -e "\e[1;32m✅ BDH-IDE installed globally as 'bdh-ide'!\e[0m"
    else
        echo -e "\e[1;31m❌ BDH-IDE compilation failed!\e[0m"
        exit 1
    fi
else
    echo -e "\e[1;33m⚠️ bdh-ide.c not found in current directory. Creating fallback bash launcher...\e[0m"
    # Fallback: C கோடு இல்லை என்றால் பழையபடி Bash ஸ்கிரிப்ட்டையே உருவாக்கும்
    cat << 'EOF' > "$INSTALL_DIR/bdh-ide"
#!/bin/bash
export BDH_IDE_ACTIVE=1
export EDITOR="bdh-edit"
exec bdh-terminal-engine "$@"
EOF
    chmod +x "$INSTALL_DIR/bdh-ide"
fi
