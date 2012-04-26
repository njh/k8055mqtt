#!/bin/sh

INSTALLED="/Library/Extensions/k8055.kext"

# Uninstall previous extension
if [ -e $INSTALLED ]; then
    sudo kextunload -verbose $INSTALLED
    sudo rm -Rf $INSTALLED
fi

# Compile new extension
xcodebuild -project k8055.xcodeproj -target k8055 -configuration Deployment clean build

if [ ! -e "build/Deployment/k8055.kext" ]; then
    echo "Failed to build extension"
    exit -1
fi


# Install new extension
sudo cp -R build/Deployment/k8055.kext $INSTALLED
sudo chown -R root:wheel $INSTALLED
sudo kextload -verbose $INSTALLED
