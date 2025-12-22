# Audio

Behringer UMC cards are in `/usr/share/alsa/ucm2/USB-Audio/Behringer/UMC202HD.conf` e.g.



`lsof /dev/snd/*` lists what application is using sound output.

* OSS (Open Sound System) is the original sound system for Linux and is not used anymore. 
* ALSA replaced OSS and supports USB as source/sink and other features. It runs in the kernel, and devices
  provide ALSA hardware drivers.
* PulseAudio sits on top of ALSA and allows multiple applications to use a (PulseAudio) sound device;
  ALSA only allows a single process. It can control volume per process etc.
* JACK is a low-latency sound server. It can run in parallel to PipeWire.
* PipeWire also sits on top of ALSA and replaces (and also emulates) PulseAudio and JACK. It also controls
  video streams, not only audio, and supports low-latency audio. PipeWire can route media streams between applications
  and ALSA. Older applications using PulseAudio or JACK also run with PipeWire as it emulates them.


https://forum.manjaro.org/t/understanding-pulseaudio-profiles-and-sinks/159259/4

https://chatbot.eps.ch/share/C1-8dUVih7pZD556VjmIX

[pulseaudio-profiles]: https://www.freedesktop.org/wiki/Software/PulseAudio/Backends/ALSA/Profiles/

FMOD → PulseAudio → ALSA flow:

- FMOD (PulseAudio backend) creates a PA stream with a channel count and an explicit PA channel map that matches FMOD’s speakermode. For 7.1 it requests 8 channels with PA channel labels: `front-left, front-right, front-center, lfe, rear-left, rear-right, side-left, side-right` (the standard PA `surround-71` map).
- FMOD does not send arbitrary strings per sample; it submits interleaved frames in that fixed order, and the PA channel map labels tell PulseAudio which slot is which.
- PulseAudio uses that channel map to decide which underlying sink channels to route to. If the sink’s map matches (e.g., also `front-left,front-right,...`), channels are wired 1:1. If the sink advertises only FL/FR, PulseAudio will downmix. If it advertises nonstandard labels (e.g., `aux0..aux11`), PulseAudio lacks semantic positions and often routes only FL/FR unless you remap.
- PipeWire emulating PA behaves similarly: the PA channel map from the client is matched against the sink’s channel map; missing labels cause downmix or partial routing.

ALSA layer:

- PulseAudio ultimately opens an ALSA PCM (often `front`, `surround71`, `hw`, etc.) according to the sink definition. The ALSA PCM plugin defines how logical positions map to hardware device channels. That mapping is usually encoded in the PCM definition (e.g., `surround71:CARD=UMC1820,DEV=0` has a known 7.1 map). Custom mappings can be set in `/etc/asound.conf` or `~/.asoundrc` by defining named PCM devices with a specific `route` or `ttable`.
- If the ALSA PCM has no positional map (e.g., a bare `hw` device), PulseAudio can’t infer semantics, so it exposes them as `aux0..auxN`; clients then lack positional info and typically fall back to stereo/downmix.

Practical implications for your setup:

- For FMOD + PA: ensure the sink advertises a real surround map (not `aux*`). Then FMOD’s 7.1 map will route correctly.
- If the sink shows `aux*`, either switch to a profile that exposes positional labels (e.g., “Pro Audio”/surround) or define an ALSA PCM (`surround71` or custom route) and make PA use that as the sink, so the channel labels are known all the way down.
