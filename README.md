# FMOD Service

This service provides a message based interface to FMOD, allowing other clients
to control [FMOD][fmod] sound projects by playing events and setting
parameters.


## Pre-requisites

`libzmq3-dev` must be installed on the system.


## Compiling

You need the FMOD Studio API (only available to registered users); unpack it to
`lib/` so it contains an `fmodstudioapi20203linux` directory.

The `cppzmq` submodule should be checked out in the `lib/` directory as well.

Then, build as usual with CMake.

[fmod]: https://www.fmod.com/


## Configuring sound cards on Raspberry Pi

Some defaults in `/usr/share/alsa/alsa.conf`

Other defaults in `/etc/asound.conf`


[so]: https://raspberrypi.stackexchange.com/questions/80072/how-can-i-use-an-external-usb-sound-card-and-set-it-as-default/80075#80075


## Release Notes

* Upcoming
* **v1.4.2** (2025-06-12)
  * Changed: FMOD API updated to 2.03.08
* **v1.4.1** (2025-06-05)
  * Issue: **Do not use!** 2.03.07 has sustain point issues.
  * Changed: FMOD API updated to 2.03.07
* **v1.4.0** (2025-03-27)
  * Issue: **Do not use!** 2.03.06 has sustain point issues.
  * Changed: cppzmq bumped to 4.10.0
  * Changed: FMOD API bumped to 2.03.06
* **v1.3.1** (2023-11-02)
  * Changed: FMOD API bumped to 2.02.18
* **v1.3.0** (2023-01-31)
  * Changed: Communication is not logged anymore unless the service is run in verbose mode with `-v`.
  * Fixed: If a voice key does not exist, an error is returned and the service does not crash anymore.
* **v1.2.0** (2022-12-30)
  * Added: Stop all started events
  * Added: Service version is now printed in the `--help` output
* **v1.1.0** (2022-07-25)
  * Added: Global parameters can now be played by using `global` as event ID
    when using the ZeroMQ API.
* **v1.0.0** (2022-03-30)
  * Added: Play “voice events”, i.e. events with a programmer instrument where
    the sound file from the programmer instrument can be specified from the API
  * Added: Sample rate, speaker configuration, and live update can now be
    configured with CLI arguments
  * Added: Support unloading banks for localisation support with audio tables
  * Added: Playing/stopped events (not on ZMQ yet)
  * Added: List paths of loaded banks
