Changes in 0.9.10
================

This version is tagged for packaging. Changes include:

* Some native SSH improvement
* Some fix on format and spelling

Changes in 0.9.9
================

This version includes some clean up and refactoring. Other changes include:

* Better .desktop file
* Give a warning on insecure SSHv1 connection

A newer MS Windows build will also come with this release.

Changes in 0.9.8.7
==================

This version fixes the SSH MAC verification. However, you should still use your local ssh(1) if you can.

A new feature is introduced to show the connection info.

Changes in 0.9.8.6
==================

Well, it's still not the final version of FQTerm 0.9.x branch...

Changes include:

* drop mini server
* more refactor on native SSH
* merge the fix to #11

Changes in 0.9.8.4
==================

This is the first version that has a change log. New features from the time I forked this project include:

* build system:

  - Qt5 support
  - OpenSSL 1.1.0 support
  - can now build with MinGW

* functions:

  - recover local socket support that enables using external programs such ssh(1) to connect to a host
  - more functions in script engine and more scripts
  - raw packet capture
  - better SSH support: support diffie-hellman-group14-sha1, aes-ctr

* bug fixes and code clean up

