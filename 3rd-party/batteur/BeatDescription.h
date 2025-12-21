#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "filesystem.hpp"
#include "Debug.h"
#include "tl/optional.hpp"

namespace fs = ghc::filesystem;

namespace batteur {

struct Note {
    Note(double timestamp, double duration, uint8_t number, float velocity)
    : timestamp(timestamp), duration(duration), number(number), velocity(velocity) {}
    double timestamp;
    double duration;
    uint8_t number;
    float velocity;
};

using Sequence = std::vector<Note>;

double barCount(const Sequence& sequence, double quartersPerBar);
void alignSequenceEnd(Sequence& sequence, double numBars, double quartersPerBar);

struct Part {
    std::string name;
    Sequence mainLoop;
    std::vector<Sequence> fills;
    tl::optional<Sequence> transition;
};

struct TimeSignature {
    int num;
    int denom;
};

struct BeatDescription {
    std::string name;
    std::string group;
    float bpm;
    double quartersPerBar;
    TimeSignature signature;
    tl::optional<Sequence> intro;
    std::vector<Part> parts;
    tl::optional<Sequence> ending;
    static std::unique_ptr<BeatDescription> buildFromFile(const fs::path& file, std::error_code& error);
    static std::unique_ptr<BeatDescription> buildFromString(const fs::path& virtualFile, const std::string& string, std::error_code& error);
};

enum class BeatDescriptionError {
    NonexistentFile = 1,
    NoFilename,
    NoParts
};

std::error_code make_error_code(BeatDescriptionError);

}

template <>
struct std::is_error_code_enum<batteur::BeatDescriptionError> : true_type {};
