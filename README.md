# FMOD Service

This service provides a message based interface to FMOD, allowing other clients
to control [FMOD][fmod] sound projects by playing events and setting
parameters.


## Compiling

You need the FMOD Studio API (only available to registered users); unpack it to
`lib/` so it contains an `fmodstudioapi20203linux` direcory.

The `cppzmq` submodule should be checked out in the `lib/` directory as well.

Then, build as usual with CMake.

[fmod]: https://www.fmod.com/

Todo:

* Parse command line arguments, e.g. for speaker configuration (5.1, 7.1, Stereo, etc.)

