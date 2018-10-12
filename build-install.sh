#!/bin/bash -xve

QTVERSION=5
BINDIR="/opt/ancamgui"

qtchooser -run-tool=qmake -qt=$QTVERSION
make

if [ ! -d $BINDIR ]
then
  sudo mkdir $BINDIR
fi

sudo cp ancamgui $BINDIR
sudo cp icon2.svg $BINDIR/icon.svg
cp ancamgui.desktop ~/.local/share/applications
make distclean

