# Breeze10

## Overview

Breeze10 is a fork of KDE Breeze decoration with the following changes:

 * The title-bar opacity is configurable.
 * The separator between title-bar and window is removed.
 * Opaqueness, opacity override is added to the exception list properties.
 * Title-bar font is set indpendent from the KDE font settings (for use outside KDE).

## Credits

Breeze10 was started from BreezeEnhanced (https://github.com/tsujan/BreezeEnhanced), a former fork of Breeze with title-bar translucency and blurring.

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

## Known Issues

- Cannot resize windows from the top border. I must have messed up something while modifying the code.
- There's an unwanted 1px padding below the buttons. I haven't figured it out where it came from.

## Screenshots

![Settings](screenshots/Settings.png?raw=true "Settings")

![Desktop](screenshots/Desktop.png?raw=true "Desktop")
