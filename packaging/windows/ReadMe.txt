FritzCallIndicator - https://github.com/ElTh0r0/fritzcallindicator


About
-----

Show taskbar notifications for incoming calls from the Fritz!Box 


Prepare your Fritz!Box
----------------------

* Enable TR-064 protocol (required for phonebook and call histroy access):
  * Log into the Fritz!Box web interface (usually at http://fritz.box)
  * Go to Home Network -> Network -> Network Settings, on the bottom of the page go to 'Advanced network settings'
  * On tab 'Home network shares' enable the option 'Allow access for applications' (this activates TR-064)
* Set up a user for TR-064 access:
  * Go to System -> Fritz!Box Users
  * Create a new user specifically for TR-064 access (e.g., username like 'fritzcallindicator')
  * Assign permissions for Fritz!Box settings and Voicemails, fax messages, FRITZ!App Fon and call lists
* Enable real-time call monitoring:
  * Call the code #96*5* from a telephone connected to the Fritz!Box (either directly or via DECT base)
  * A confirmation tone should signal successful activation of call monitoring
  * Alternatively, enable the callmonitor via Fritz!Box internal phonebook by creating entries with the numbers #96*5*
  * (If it shall be deactivated later again, dial #96*4*)
* After enabling the above, restart the Fritz!Box under System -> Backup -> Restart to activate changes
* Firewall or security software
  * Ensure the TCP port 1012 (used by call monitor) is allowed
  * TR-064 communicates over TCP ports 49000 and 49443, which both should be allowed

  
Translations
------------

 - German: ElThoro

New translations and corrections are highly welcome! You can fork the source code from GitHub, make your changes and create a pull request.


Acknowledgments
---------------

This utility is heavily inspired by FritzBoxCallMonitor (https://github.com/petermost/FritzBoxCallMonitor) and using its main logic for connecting to the Fritz!Box for receiving calls and KFritz (https://github.com/Agundur-KDE/kfritz) from which the Fritz!Box phone book extraction is used. Kudos to both of the projects!

FritzCallIndicator uses Breeze icons from KDE: https://invent.kde.org/frameworks/breeze-icons
