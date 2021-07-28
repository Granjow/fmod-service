# FMOD Service

This service provides a message based interface to FMOD, allowing other clients to use it.

Todo:

* Decide on ZeroMQ vs grpc/ProtoBuf
* Parse command line arguments, e.g. for speaker configuration (5.1, 7.1, Stereo, etc.)
* Load banks etc. dynamically with ZeroMQ
* Change arguments with ZMQ
* Run as service/daemon

## Interface

* Load bank
* Load event
* Start/stop/play event
* Change parameter
