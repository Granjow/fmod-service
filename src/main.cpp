#include "ZmqApi.h"
#include "version.h"

#include <string>
#include <vector>
#include <iostream>

enum Args {
    Arg_None,
    Arg_SampleRate,
    Arg_SpeakerMode,
    Arg_RawSpeakers,
} nextArg = Arg_None;

void printHelp(const char *arg0) {
    std::cout << "Usage: " << arg0 << " OPTIONS" << std::endl << std::endl;
    std::cout << "This is FMOD Service version " << version << "." << std::endl;
    std::cout << R"(
--help
  Print Help
--samplerate SAMPLE_RATE
  Set sample rate, e.g. 44100, 12000 (low quality)
--speakermode [stereo, quad, 5.1, 7.1, raw]
  Set speaker mode
--raw-speakers COUNT
  Set number of raw speakers (untested!)
--liveupdate
  Enable live update on port 9264
-v
  Verbose output
)" << std::endl;
}

int main(int argc, const char *argv[]) {

    std::vector<std::string_view> args(argv + 1, argv + argc);

    bool verbose = false;
    int rawSpeakerCount = 0;
    int sampleRate = 41000;
    bool liveUpdate = false;
    std::string speakerModeName = "7.1";
    FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_7POINT1;

    for (const auto &arg: args) {
        Args currentArg = nextArg;

        if (currentArg != Arg_None) {
            nextArg = Arg_None;
        }

        switch (currentArg) {
            case Arg_SampleRate:
                sampleRate = std::stoi(std::string(arg));
                break;
            case Arg_RawSpeakers:
                rawSpeakerCount = std::stoi(std::string(arg));
                break;
            case Arg_SpeakerMode:
                if (arg == "7.1") {
                    speakerMode = FMOD_SPEAKERMODE_7POINT1;
                    speakerModeName = "7.1";
                } else if (arg=="5.1") {
                    speakerMode = FMOD_SPEAKERMODE_5POINT1;
                    speakerModeName = "5.1";
                } else if (arg=="quad") {
                    speakerMode = FMOD_SPEAKERMODE_QUAD;
                    speakerModeName = "Quad";
                } else if (arg == "stereo") {
                    speakerMode = FMOD_SPEAKERMODE_STEREO;
                    speakerModeName = "Stereo";
                } else if (arg == "raw") {
                    speakerMode = FMOD_SPEAKERMODE_RAW;
                    speakerModeName = "RAW";
                } else {
                    std::cerr << "Unsupported speaker mode: " << arg << std::endl;
                    return -1;
                }
                break;
            case Arg_None:
                if (arg == "--samplerate") {
                    nextArg = Arg_SampleRate;
                } else if (arg == "--speakermode") {
                    nextArg = Arg_SpeakerMode;
                } else if (arg == "--raw-speakers") {
                    nextArg = Arg_RawSpeakers;
                } else if (arg == "--liveupdate") {
                    liveUpdate = true;
                } else if (arg == "--help" || arg == "-h") {
                    printHelp(argv[0]);
                    return 0;
                } else if (arg == "-v") {
                    verbose = true;
                } else {
                    std::cerr << "Unsupported argument: " << arg << std::endl;
                    return -1;
                }
                break;
        }
    }

    std::cout << "Starting with:" << std::endl
              << "* Sample rate " << sampleRate << std::endl
              << "* Speaker mode " << speakerModeName << std::endl
              << "* Live update " << (liveUpdate ? "enabled" : "disabled") << std::endl;

    FmodController fmodController(sampleRate, speakerMode, liveUpdate, rawSpeakerCount);
    fmodController.setEventCallback([](const std::string &eventId, EventType eventType) {
        std::cout << "CALLBACK: " << eventId << (eventType == EventType_Started ? " Started" : " Stopped") << std::endl;
    });

    ZmqApi zmqApi(fmodController);
    zmqApi.verbose = verbose;

    zmqApi.run();
}