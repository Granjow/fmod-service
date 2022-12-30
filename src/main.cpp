#include "ZmqApi.h"
#include "version.h"

#include <string>
#include <vector>
#include <iostream>

enum Args {
    Arg_None,
    Arg_SampleRate,
    Arg_SpeakerMode,
} nextArg = Arg_None;

void printHelp(const char *arg0) {
    std::cout << "Usage: " << arg0 << " OPTIONS" << std::endl << std::endl;
    std::cout << "This is FMOD Service version " << version << "." << std::endl;
    std::cout << R"(
--help
  Print Help
--samplerate SAMPLE_RATE
  Set sample rate, e.g. 44100, 12000 (low quality)
--speakermode [stereo, 7.1]
  Set speaker mode
--liveupdate
  Enable live update on port 9264
)" << std::endl;
}

int main(int argc, const char *argv[]) {

    std::vector<std::string_view> args(argv + 1, argv + argc);

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
            case Arg_SpeakerMode:
                if (arg == "7.1") {
                    speakerMode = FMOD_SPEAKERMODE_7POINT1;
                    speakerModeName = "7.1";
                } else if (arg == "stereo") {
                    speakerMode = FMOD_SPEAKERMODE_STEREO;
                    speakerModeName = "Stereo";
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
                } else if (arg == "--liveupdate") {
                    liveUpdate = true;
                } else if (arg == "--help" || arg == "-h") {
                    printHelp(argv[0]);
                    return 0;
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

    FmodController fmodController(sampleRate, speakerMode, liveUpdate);
    fmodController.setEventCallback([](const std::string &eventId, EventType eventType) {
        std::cout << "CALLBACK: " << eventId << (eventType == EventType_Started ? " Started" : " Stopped") << std::endl;
    });

    ZmqApi zmqApi(fmodController);

    zmqApi.run();
}