#!/bin/bash

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}    Asciiloscope Installer v1.0          ${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo -e "${RED}❌ Error: Could not detect OS.${NC}"
    exit 1
fi

echo -e "🔍 OS Detected: ${GREEN}$NAME${NC}"

# Fedora
if [[ "$OS" == "fedora" ]]; then
    echo -e "${BLUE}📦 Configuring COPR for Fedora...${NC}"
    sudo dnf install dnf-plugins-core -y
    sudo dnf copr enable altaugust/asciiloscope -y
    
    echo -e "${BLUE}⬇️  Installing asciiloscope...${NC}"
    sudo dnf install asciiloscope -y

# Ubuntu / Debian / Mint / Pop!_OS
elif [[ "$OS" == "ubuntu" || "$OS" == "debian" || "$OS" == "linuxmint" || "$OS" == "pop" ]]; then
    echo -e "${BLUE}📦 Configuring PPA for Ubuntu/Debian...${NC}"
    
    # Ensure add-apt-repository exists
    sudo apt update
    sudo apt install software-properties-common -y
    
    sudo add-apt-repository ppa:altaugust/asciiloscope -y
    sudo apt update
    
    echo -e "${BLUE}⬇️  Installing asciiloscope...${NC}"
    sudo apt install asciiloscope -y

else
    echo -e "${RED}❌ Sorry, this script only supports Fedora and Ubuntu/Debian based systems.${NC}"
    echo "Please check GitHub for manual build instructions."
    exit 1
fi

# Success
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ Success! Run 'asciiloscope' to start.${NC}"
else
    echo ""
    echo -e "${RED}❌ Something went wrong during installation.${NC}"
fi
