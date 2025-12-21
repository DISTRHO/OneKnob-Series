#include "BeatDescription.h"
#include "MathHelpers.h"
#include <fmidi/fmidi.h>
#include "tl/expected.hpp"
#include "FileReadingHelpers.h"
#include "json.hpp"

using nlohmann::json;

namespace { // anonymous namespace

struct BeatDescriptionErrorCategory : std::error_category {
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const char* BeatDescriptionErrorCategory::name() const noexcept
{
    return "beat-descriptions";
}

std::string BeatDescriptionErrorCategory::message(int ev) const
{
    switch (static_cast<batteur::BeatDescriptionError>(ev)) {
    case batteur::BeatDescriptionError::NonexistentFile:
        return "File does not exist";

    case batteur::BeatDescriptionError::NoFilename:
        return "No filename key in the JSON dictionary";

    case batteur::BeatDescriptionError::NoParts:
        return "No parts found in the JSON dictionary";

    default:
        return "Unknown error";
    }
}

// https://akrzemi1.wordpress.com/2017/07/12/your-own-error-code/
const BeatDescriptionErrorCategory beatDescriptionErrorCategory {};

}

std::error_code batteur::make_error_code(batteur::BeatDescriptionError e)
{
    return { static_cast<int>(e), beatDescriptionErrorCategory };
}

namespace batteur{
  
double barCount(const Sequence& sequence, double quartersPerBar)
{
    if (sequence.empty())
        return 0.0;

    return std::ceil(sequence.back().timestamp / quartersPerBar);
}

void alignSequenceEnd(Sequence& sequence, double numBars, double quartersPerBar)
{
    const double sequenceBars = barCount(sequence, quartersPerBar);
    DBG("Number of bars in the sequence to align" << sequenceBars
        << "vs the one to align to" << numBars);
    const double shift = (numBars - sequenceBars) * quartersPerBar;
    DBG("Shifting by " << shift);

    if (shift < 0.0) {
        sequence.erase(
            sequence.begin(),
            std::find_if(
                sequence.begin(),
                sequence.end(),
                [shift](const Note& note) { return note.timestamp >= shift; }));
    }

    for (auto& note : sequence)
        note.timestamp += shift;
}

std::unique_ptr<BeatDescription> buildDescriptionFromJson(const fs::path& virtualFile, const nlohmann::json& json, std::error_code& error)
{
    auto beat = std::unique_ptr<BeatDescription>(new BeatDescription());

    // Minimal file
    const auto title = json["name"];
    if (title.is_null()) {
        error = BeatDescriptionError::NoFilename;
        return {};
    }
    beat->name = title;

    const auto group = json.find("group");
    if (group != json.end())
        beat->group = *group;

    const auto parts = json.find("parts");
    if (parts == json.end() || !parts->is_array() || parts->size() == 0) {
        error = BeatDescriptionError::NoParts;
        return {};
    }

    beat->bpm = 120.0;
    const auto bpm = json.find("bpm");
    if (bpm != json.end())
        beat->bpm = *bpm;    

    beat->quartersPerBar = 4;
    beat->signature = { 4, 4 };
    const auto qpb = json.find("quarters_per_bar");
    const auto sig = json.find("signature");
    if (qpb != json.end()) {
        beat->quartersPerBar = checkQuartersPerBar(*qpb).value_or(beat->quartersPerBar);
        beat->signature.num = static_cast<int>(beat->quartersPerBar);
    } else if (sig != json.end() && sig->is_array() && sig->size() == 2) {
        const auto s = *sig;
        if (s[0].is_number_unsigned())
            beat->signature.num = s[0].get<int>();

        if (s[1].is_number_unsigned())
            beat->signature.denom = s[1].get<int>();

        beat->quartersPerBar = beat->signature.num * 4.0 / beat->signature.denom;
    }

    const auto rootDirectory = virtualFile.parent_path();

    if (auto seq = readSequenceByName(json, rootDirectory, "intro"))
        beat->intro = std::move(*seq);

    if (auto seq = readSequenceByName(json, rootDirectory, "ending"))
        beat->ending = std::move(*seq);

    for (auto& part : *parts) {
        Part newPart;
        newPart.name = part["name"];
        auto mainLoop = readSequenceByName(part, rootDirectory, "sequence");
        if (!mainLoop)
            continue;

        newPart.mainLoop = std::move(*mainLoop);

        for (auto& fill : part["fills"]) {
            if (auto seq = readSequence(fill, rootDirectory)) {
                newPart.fills.push_back(std::move(*seq));
            }
        }

        if (auto seq = readSequenceByName(part, rootDirectory, "transition")) {
            newPart.transition = std::move(*seq);
        }
        beat->parts.push_back(std::move(newPart));
    }

    if (beat->parts.empty()) {
        error = BeatDescriptionError::NoParts;
        return {};
    }

    return beat;
}

std::unique_ptr<BeatDescription> BeatDescription::buildFromFile(const fs::path& file, std::error_code& error)
{
    if (!fs::exists(file)) {
        error = BeatDescriptionError::NonexistentFile;
        return {};
    }

    fs::fstream inputStream { file, std::ios::ios_base::in };
    nlohmann::json json;
    inputStream >> json;
    return buildDescriptionFromJson(file, json, error);
}

std::unique_ptr<BeatDescription> BeatDescription::buildFromString(const fs::path& virtualFile, const std::string& string, std::error_code& error)
{
    return buildDescriptionFromJson(
        virtualFile, nlohmann::json::parse(string, nullptr, false), error);
}
  
}