# FritzCallIndicator
FritzCallIndicator displays a notification pop-up in the taskbar for incoming calls from the Fritz!Box.

![Call](https://github.com/user-attachments/assets/6eb39205-a047-4bdb-b7dc-dc7b8fd85126)

In addition, FritzCallIndicator has the following features:

* Several options to resolve the number of the incoming call:
  * Thunderbird address books
  * CardDAV address books (e.g. from Nextcloud)
  * Online telecommunications directories to reverse lookup the number of the incoming caller
  * If the number cannot be resolved from the above sources, the tool will show the country or city of origin of the call (currently only German cities supported).
* Show the history of the incoming calls from the Fritz!Box
* Play a notification sound when an incoming call is received

## Installation
* [Build for Windows](https://github.com/ElTh0r0/fritzcallindicator/releases/latest)
* [Ubuntu PPA](https://launchpad.net/~elthoro/+archive/fritzcallindicator)
* [Builds for Debian, Fedora, openSUSE](http://software.opensuse.org/download.html?project=home%3AElThoro&package=fritzcallindicator)
* [Arch AUR](https://aur.archlinux.org/packages/fritzcallindicator/)

## Prepare your Fritz!Box
* Enable TR-064 protocol (required for phone book and call history access):
  * Log into the Fritz!Box web interface (usually at http://fritz.box)
  * Go to Home Network -> Network -> Network Settings, on the bottom of the page go to 'Advanced network settings'
  * On tab 'Home network shares' enable the option 'Allow access for applications' (this activates TR-064)
* Set up a user for TR-064 access:
  * Go to System -> Fritz!Box Users
  * Create a new user specifically for TR-064 access (e.g., username like 'fritzcallindicator')
  * Assign permissions for Fritz!Box settings and Voicemails, fax messages, FRITZ!App Fon and call lists
* Enable real-time call monitoring:
  * Call the code #96\*5\* from a telephone connected to the Fritz!Box (either directly or via DECT base)
  * A confirmation tone should signal successful activation of call monitoring
  * Alternatively, enable the callmonitor via Fritz!Box internal phone book by creating entries with the numbers #96\*5\*
  * (If it shall be deactivated later again, dial #96\*4\*)
* After enabling the above, restart the Fritz!Box under System -> Backup -> Restart to activate changes
* Firewall or security software
  * Ensure the TCP port 1012 (used by call monitor) is allowed
  * TR-064 communicates over TCP ports 49000 and 49443, which both should be allowed

## Settings
See [Wiki](https://github.com/ElTh0r0/fritzcallindicator/wiki) on GitHub.

## Build instructions
* FritzCallIndicator can be compiled with Qt >= 6.2
* Besides the Qt Essential modules and Qt **XML**, the following Add-On modules are (optionally) required:
  * If the Thunderbird addressbook integration shall be used, the Qt **SQL** module is required
  * If a notification sound shall be played during an incoming call, the Qt **Multimedia** module is required

### cmake
Adjust CMAKE_PREFIX_PATH according to your Qt installation. Optionally the following options can be changed (all are enabled by default):

* `FRITZ_USE_ONLINE_RESOLVERS` - Enable online resolvers
* `FRITZ_USE_THUNDERBIRD_ADDRESSBOOK` - Enable Thunderbird addressbook
* `FRITZ_USE_CARDDAV_ADDRESSBOOK` - Enable CardDAV addressbook
* `FRITZ_USE_NOTIFICATION_SOUND` - Enable notification sound

```
cmake -B build-cmake -DCMAKE_PREFIX_PATH=/usr/include/qt6
cmake --build build-cmake -- -j8  # Adjust -j8 according to your available cores.
sudo cmake --install build-cmake  # or DESTDIR=foobar cmake --install build-cmake
```

## Help translating
New translations and corrections are highly welcome! You can fork the source code from GitHub, make your changes and create a pull request.

## Acknowledgments
This utility is heavily inspired by [FritzBoxCallMonitor](https://github.com/petermost/FritzBoxCallMonitor) and using its main logic for connecting to the Fritz!Box for receiving calls and [KFritz](https://github.com/Agundur-KDE/kfritz) from which the Fritz!Box phone book extraction is used. Kudos to both of the projects!
