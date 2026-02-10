#!/bin/bash

# Cores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # Sem cor

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}    Instalador do Asciiloscope v1.0      ${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Detectar a Distribuição
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo -e "${RED}❌ Erro: Não foi possível detectar o sistema operacional.${NC}"
    exit 1
fi

echo -e "🔍 Sistema detectado: ${GREEN}$NAME${NC}"

# Fedora
if [[ "$OS" == "fedora" ]]; then
    echo -e "${BLUE}📦 Configurando COPR para Fedora...${NC}"
    sudo dnf install dnf-plugins-core -y
    sudo dnf copr enable altaugust/asciiloscope -y
    
    echo -e "${BLUE}⬇️  Instalando asciiloscope...${NC}"
    sudo dnf install asciiloscope -y

# Ubuntu / Debian / Mint / Pop!_OS
elif [[ "$OS" == "ubuntu" || "$OS" == "debian" || "$OS" == "linuxmint" || "$OS" == "pop" ]]; then
    echo -e "${BLUE}📦 Configurando PPA para Ubuntu/Debian...${NC}"
    
    # Garante que tem o add-apt-repository
    sudo apt update
    sudo apt install software-properties-common -y
    
    sudo add-apt-repository ppa:altaugust/asciiloscope -y
    sudo apt update
    
    echo -e "${BLUE}⬇️  Instalando asciiloscope...${NC}"
    sudo apt install asciiloscope -y

else
    echo -e "${RED}❌ Desculpe, este script suporta apenas Fedora e Ubuntu/Debian.${NC}"
    echo "Consulte o GitHub para compilação manual."
    exit 1
fi

# Sucesso
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ Sucesso! Digite 'asciiloscope' para iniciar.${NC}"
else
    echo ""
    echo -e "${RED}❌ Algo deu errado na instalação.${NC}"
fi
