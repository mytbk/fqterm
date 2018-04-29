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

