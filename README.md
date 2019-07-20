# Breeze10

## Overview

Breeze10 is a fork of KDE Breeze decoration with the following changes:

 * The title-bar opacity is configurable.
 * A very mild light line is added to the top of title-bar (especially for dark color schemes) and the separator between title-bar and window is removed.
 * The spacing between buttons is configurable.
 * Opaqueness, opacity override is added to the exception list properties.
 * Title-bar font is set indpendent from the KDE font settings (for use outside KDE).

Please note that Breeze10 is not related to the Breeze widget style. In fact, it is made to match various themes of the [Kvantum](https://github.com/tsujan/Kvantum) widget style but it works with all styles.

## Credits:

Breeze10 was started from BreezeEnhanced (https://github.com/tsujan/BreezeEnhanced), a former fork of Breeze with title-bar translucency and blurring.

Needless to say, the main work behind Breeze10 is the Breeze KWin decoration itself, which can be downloaded from https://download.kde.org/stable/plasma/.

## Installation

The version number in the file NEWS shows the main version of KWin that is required for the compilation. *Compilation should not be done against other versions of KWin!*.

Open a terminal inside the source directory and do:
```sh
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install
```
After the intallation, restart KWin by logging out and in. Then, Breeze10 will appear in *System Settings &rarr; Application Style &rarr; Window Decorations*.

### Installation with package manager

Users of Arch and its derivatives can install breeze-enhanced-git from AUR.

Users of OpenSUSE Tumbleweed can do (thanks to trmdi at GitHub):
```sh
sudo zypper ar obs://home:trmdi trmdi
sudo zypper in -r trmdi Breeze10
```

## Screenshots:

![Settings](screenshots/Settings.png?raw=true "Settings")

![Exception](screenshots/Exception.png?raw=true "Exception")

![Buttons](screenshots/Buttons.png?raw=true "Buttons")

![Animation](screenshots/Animation.gif?raw=true "Animation")
