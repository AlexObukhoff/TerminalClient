
# Links

* [FAQ](https://dev.cyberplat.com/CyberPlat/TerminalClient/wiki/faq)
* [Supported devices](https://dev.cyberplat.com/CyberPlat/TerminalClient/wiki/supported_device)

# How to setup build OS

```shell
sudo apt-get install mc htop vim

; qtcreator тащит почти все нужные зависимости
sudo apt-get install qtcreator
sudo apt-get install build-essential libgl1-mesa-dev  

sudo apt-get install libqt5xmlpatterns5-dev  qtscript5-dev qtdeclarative5-private-dev qttools5-private-dev qtbase5-private-dev libqt5xmlpatterns5-dev qtscript5-dev  libqt5xmlpatterns5-dev qml-module-qtquick-privatewidgets libqt5script5 libqt5scripttools5  qml-module-qtquick-privatewidgets qml-module-qtquick-privatewidgets qtbase5-private-dev libqaccessibilityclient-qt5-dev qtmultimedia5-dev qtwebengine5-dev

sudo apt-get install qbs qt5-default

; WTF !!!
sudo strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so.5

qbs setup-toolchains --detect
qbs-setup-qt --detect
qbs config defaultProfile qt-5-11-3
qbs config profiles.qt-5-11-3.baseProfile  gcc
```
