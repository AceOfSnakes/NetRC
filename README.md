# NetRC. Based on Qt 6+
<p align="center">
<img src="src/images/NetRC.png" width ="128" height="128"/>
</p>

Preview | Description
:-------------------------:|:-------------------------:
![NetRC](doc/linux/images/Oppo-203.png) | **NetRC** also known as  **Network remote control**. The base idea of this project is to provide access to control your devices via local network. All devices should be configurable without changes in source code. All you need is to create device configuration file. Load it into application and control your device via network. Based on ideas and code from this project https://sourceforge.net/projects/avrpioremote/.

# NetRC Integration steps
* Create configuration file to control your device. When you know how to control your devices via network then you can create [device protocol file](settings/)
and map buttons from this program to your device (more details will be provided soon).
Some useful files for make device protocol JSON configuration file can be found [here](doc/rfc/)

1. Create Device protocol file description [TBD]
    * Connection settings
    * Predefined buttons / panels
    * Custom buttons / panels
1. Load Device protocol file description [TBD]
1. Connect To Device /  Auto Search Device [TBD]

# Known devices
* [OPPO UDP-203](doc/OPPO_UDP-203/README.md)
* [Pioneer BDP-140](doc/Pioneer_BDP-140/README.md)
* [Pioneer VSX-923](doc/Pioneer_VSX923/README.md)

# Tested Environment
* [Windows 11](doc/w11/README.md)
* [Ubuntu 22.04](doc/linux/README.md)

# C11n
* [Themes](doc/style/README.md)
