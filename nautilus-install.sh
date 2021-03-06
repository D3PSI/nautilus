#!/bin/bash
echo "Thank you for choosing to install nautilus created by D3PSI!"
build=true
ci=false
while [ "$1" != "" ]; do
    case $1 in
        --no-build )            shift
                                build=false
                                ;;
        --ci )                  shift
                                ci=true
                                ;;
    esac
    shift
done
if [[ $EUID -ne 0 ]]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "Trying to install dependencies for macOS..."
        if brew update && brew install git glfw3 glm assimp cmake make ninja python3 curl; then
            echo "Successfully installed dependencies for macOS"
        else
            echo "Could not install one or more dependencies of nautilus. Going kamikaze, hoping all will work out."
        fi
        git submodule sync
        git submodule update --init --recursive
        if [[ "$build" = true ]]; then
            echo "Generating build files..."
            if cmake CMakeLists.txt; then
                echo "Building project..."
                threads=$(sysctl -n hw.ncpu)
                if [[ "$docker" = true ]]; then
                    if make; then
                        echo "You can now run the examples which have been built and written to bin/. Enjoy!"
                    else
                        echo "Failed to build the project!"
                        exit 1
                    fi
                else
                    echo "Compiling using $threads threads..."
                    if make -j$threads; then
                        echo "You can now run the examples which have been built and written to bin/. Enjoy!"
                    else
                        echo "Failed to build the project!"
                        exit 1
                    fi
                fi
            else
                echo "Failed to generate build files!"
                exit 1
            fi
        fi
        exit 0;
    else
        echo "This script must be run as root" 1>&2
        exit 1
    fi
else
    arch=$(uname -m)
    kernel=$(uname -r)
    declare -A osInfo;
    osInfo[/etc/redhat-release]=dnf
    osInfo[/etc/arch-release]=pacman
    osInfo[/etc/gentoo-release]=emerge
    osInfo[/etc/SuSE-release]=zypp
    osInfo[/etc/debian_version]=apt
    docker=false
    if [ -f /.dockerenv ]; then
        echo "Running inside docker contained environment";
        docker=true
    else
        echo "Running standalone";
    fi
    sessiontype=$(echo ${XDG_SESSION_TYPE})
    for f in ${!osInfo[@]}; do
        if [[ -f $f ]]; then
            echo "Found package manager: ${osInfo[$f]}."
            pkgman=${osInfo[$f]}
        fi
    done
    if [[ -f "/etc/os-release" ]]; then
        distroname=$(awk -F= '/^NAME/{print $2}' /etc/os-release)
    elif [[ -n "$(command -v lsb_release)" ]]; then
        distroname=$(lsb_release -s -d)
    elif [[ -f "/etc/debian_version" ]]; then
        distroname="Debian $(cat /etc/debian_version)"
    elif [[ -f "/etc/redhat-release" ]]; then
        distroname=$(cat /etc/redhat-release)
    else
        distroname="$(uname -s) $(uname -r)"
    fi
    if [ ${sessiontype}==x11 ]; then
        session=x11
    elif [ ${sessiontype}==gnome-wayland ]; then
        session=wayland
    fi
    echo "Trying to install dependencies for ${distroname} using ${pkgman} on ${session}."
    if [[ $distroname == *Fedora* ]]; then
        if dnf -y update && dnf -y install git cmake make pkgconf-pkg-config gcc g++ gdb glm-devel glfw glfw-devel assimp assimp-devel mesa-libGL-devel mesa-vulkan-devel mesa-vulkan-drivers vulkan-validation-layers vulkan-validation-layers-devel vulkan-tools && dnf -y groupinstall "X Software Development"; then
            echo "Successfully installed dependencies for your system." 
        else
            echo "Failed to install some dependencies!"
            echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) 
                and necessary libraries are installed or included in the repository in this case. Good luck!"
        fi
    elif [[ $distroname == *CentOS* ]]; then
        if yum -y update && yum -y install git cmake make pkgconf-pkg-config gcc gcc-c++ gdb libX11 libX11-devel libX11-xcb libXcursor libXcursor-devel libXi libXi-devel libXinerama libXinerama-devel libXrandr libXrandr-devel mesa-libGL-devel mesa-vulkan-devel mesa-vulkan-drivers vulkan-validation-layers vulkan-tools; then
            echo "Successfully installed dependencies for your system." 
        else
            echo "Failed to install some dependencies!"
            echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) 
                and necessary libraries are installed or included in the repository in this case. Good luck!"
        fi
    elif [[ ${pkgman} == pacman ]]; then
        if pacman -Syu --noconfirm && pacman -Sy --noconfirm git cmake make pkg-config gcc gdb glm glfw-${session} assimp xorg vulkan-validation-layers vulkan-extra-layers vulkan-mesa-layer vulkan-tools; then
            echo "Successfully installed dependencies for your system." 
        else
            echo "Failed to install some dependencies!"
            echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) 
                and necessary libraries are installed or included in the repository in this case. Good luck!"
        fi
    elif [[ ${pkgman} == apt ]]; then
        if apt-get -y update && apt-get -y install git cmake make pkg-config gcc g++ gdb libglfw3 libglfw3-dev libglm-dev libassimp-dev assimp-utils libegl1-mesa-dev xorg-dev libvulkan1 libvulkan-dev mesa-vulkan-drivers vulkan-utils; then
            if [[ "$ci" = true ]]; then
                apt-get -y install gcc-8 g++-8
                export CC=gcc-8
                export CXX=g++-8
            fi
            echo "Successfully installed dependencies for your system." 
        else
            echo "Failed to install some dependencies!"
            echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) 
                and necessary libraries are installed or included in the repository in this case. Good luck!"
            exit 1
        fi
    elif [[ $distroname = *openSUSE* ]]; then
        if zypper -n refresh && zypper -n install git cmake make pkg-config gcc gcc-c++ gdb libglfw3 libglfw-devel glm-devel libassimp3 assimp-devel xorg-x11-server xorg-x11-server-sdk xorg-x11-util-devel libxcb1 libX11-xcb1 xcb-util-devel Mesa-libGL-devel Mesa-libGL1 libvulkan1 vulkan-devel; then
            echo "Successfully installed dependencies for your system." 
        else
            echo "Failed to install some dependencies!"
            echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) 
                and necessary libraries are installed or included in the repository in this case. Good luck!"
            exit 1
        fi
    else
        echo "No supported package manager found!"
        echo "Going full kamikaze. Hoping relevant dependencies (git, cmake, make, pkg-config g++/clang++ (or any other C++17-capable compiler)) and necessary libraries are installed or included in the repository in this case. Good luck!"
    fi
    git submodule sync
    git submodule update --init --recursive
    if [[ "$build" = true ]]; then
        echo "Generating build files..."
        if cmake CMakeLists.txt; then
            echo "Building project..."
            threads=$(grep -c ^processor /proc/cpuinfo)
            if [[ "$docker" = true ]]; then
                if make; then
                    echo "You can now run the examples which have been built and written to bin/. Enjoy!"
                else
                    echo "Failed to build the project!"
                    exit 1
                fi
            else
                echo "Compiling using $threads threads..."
                if make -j$threads; then
                    echo "You can now run the examples which have been built and written to bin/. Enjoy!"
                else
                    echo "Failed to build the project!"
                    exit 1
                fi
            fi
        else
                echo "Failed to generate build files!"
                exit 1
        fi
    fi
    chown ${SUDO_USER:-${USER}} -R .
    exit 0
fi
