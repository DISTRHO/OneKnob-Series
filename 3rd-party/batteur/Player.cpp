#include "Player.h"
#include "BeatDescription.h"
#include "MathHelpers.h"
#include <cmath>

namespace batteur {

Player::Player()
{
    queuedSequences.reserve(4);
    deferredNotes.reserve(1024);
    potentialNotesToMerge.reserve(128);
}

bool Player::loadBeatDescription(const BeatDescription& description)
{
    const std::unique_lock<std::mutex> lock { callbackGuard };
    
    if (description.parts.empty())
        return false;

    currentBeat = &description;
    setTempo(description.bpm);
    reset();
    return true;
}

void Player::reset()
{
    state = State::Stopped;
    position = 0.0;
    queuedSequences.clear();
    fillIndex = 0;
    partIndex = 0;
}

bool Player::start()
{
    return messages.try_push(Message::Start);
}

bool Player::stop()
{
    return messages.try_push(Message::Stop);
}

bool Player::fillIn()
{
    return messages.try_push(Message::Fill);
}

bool Player::next()
{
    return messages.try_push(Message::Next);
}

void Player::_start()
{
    if (currentBeat->intro) {
        queuedSequences.push_back(&*currentBeat->intro);
        queuedSequences.push_back(&currentBeat->parts[partIndex].mainLoop);
        state = State::Intro;
    } else {
        queuedSequences.push_back(&currentBeat->parts[partIndex].mainLoop);
        state = State::Playing;
    }
}

void Player::_stop()
{
    while (queuedSequences.size() > 1)
        queuedSequences.pop_back();

    if (currentBeat->ending)
        queuedSequences.push_back(&*currentBeat->ending);

    state = State::Ending;
}

void Player::_fillIn()
{
    if (currentBeat->parts[partIndex].fills.empty())
        return;

    queuedSequences.push_back(&currentBeat->parts[partIndex].fills[fillIndex]);
    queuedSequences.push_back(&currentBeat->parts[partIndex].mainLoop);
    state = State::Fill;
    fillIndex = (fillIndex + 1) % currentBeat->parts[partIndex].fills.size();
}

void Player::_next()
{
    const auto& currentTransition = currentBeat->parts[partIndex].transition;
    if (enteringFillInState()) {
        queuedSequences.pop_back(); // Remove the back (which should be the next part)
        if (currentTransition) {
            queuedSequences.pop_back();
            queuedSequences.push_back(&currentTransition.value());
        }
    } else if (leavingFillInState()) {
        queuedSequences.pop_back(); // Remove the back (which should be the next part)
    } else {
        if (currentTransition) {
            queuedSequences.push_back(&currentTransition.value());
        }
    }
    partIndex = (partIndex + 1) % currentBeat->parts.size();
    fillIndex = 0;
    queuedSequences.push_back(&currentBeat->parts[partIndex].mainLoop);
    state = State::Next;
}

void Player::updateState()
{
    Message msg;
    while (messages.try_pop(msg)) {
        switch (msg) {
        case Message::Start:
            if (state == State::Stopped)
                _start();
            break;
        case Message::Stop:
            if (state != State::Stopped && state != State::Ending)
                _stop();
            break;
        case Message::Fill:
            if (state == State::Playing)
                _fillIn();
            break;
        case Message::Next:
            if (state == State::Playing || state == State::Fill)
                _next();
            break;
        }
    }
}

void Player::tick(int sampleCount)
{
    const std::unique_lock<std::mutex> lock { callbackGuard, std::try_to_lock };
    if (!lock.owns_lock())
        return;

    if (!currentBeat)
        return;

    updateState();

    if (queuedSequences.empty())
        return;

    const auto currentQPB = currentBeat->quartersPerBar;
    auto blockStart = position;
    double blockEnd = blockStart + samplesToQuarter(sampleCount);
    const auto midiDelay = [&] (double timestamp) -> int {
        return quarterToSamples(timestamp - blockStart);
    };

    auto current = queuedSequences.front(); // Otherwise we have ** everywhere..
    auto noteIt = current->begin();

    const auto barStartedAt = [currentQPB] (double pos) -> double {
        const auto barPosition = pos / currentQPB;
        return std::floor(barPosition) * currentQPB;
    };

    const auto movePosition = [&] (double offset) {
        blockEnd += offset; 
        blockStart += offset; 
        position += offset;
    };

    const auto eraseFrontSequence = [&] {
        queuedSequences.erase(queuedSequences.begin());
        current = queuedSequences.front();
        noteIt = current->begin();
        movePosition(-barStartedAt(position));
    };


    while (state != State::Stopped) {
        noteIt = std::find_if(
            noteIt,
            current->end(),
            [&](const Sequence::value_type& v) { return v.timestamp >= position; }
        );
    
        if (noteIt == current->end()) {
            if (queuedSequences.size() == 2 && state != State::Ending) {
                // DBG("Exiting fill-in state: removing the top sequence");
                eraseFrontSequence();
                state = State::Playing;
                continue;
            }

            const auto sequenceDuration = 
                barCount(*current, currentQPB) * currentQPB;

            if (blockEnd < sequenceDuration) {
                position = blockEnd;
                break;
            }

            blockEnd -= sequenceDuration;
            blockStart -= sequenceDuration; // will be negative but it's OK!
            position = 0.0;

            if (queuedSequences.size() == 1 && state == State::Ending) {
                // DBG("Ending finished; resetting");
                reset();
                break;
            }

            noteIt = current->begin();
        }        

        if (enteringFillInState() || enteringEndingState()) {
            const auto barStart = barStartedAt(position);
            const auto relPosition = position - barStart;
            const auto qpb = currentBeat->quartersPerBar;
            const auto barThreshold = qpb - 0.7;
            const auto relFillStart = queuedSequences[1]->front().timestamp;
            // DBG("Could start fill in; relative position: " << relPosition
            //     << ", fill start at " << relFillStart);
            if (relPosition > barThreshold) {
                if(relFillStart > barThreshold && relFillStart < relPosition && relFillStart < qpb) {
                    // DBG("Fill-in has a short bar; starting fill-in now");
                    eraseFrontSequence();
                    continue;
                }
                // DBG("Deferring fill in next bar");
            } else {
                if (relFillStart < relPosition) {
                    // DBG("Starting fill-in now");
                    eraseFrontSequence();
                    continue;
                }

                if (relFillStart > barThreshold) {
                    // DBG("Fill-in has a short bar; skipping the first fill bar");
                    eraseFrontSequence();
                    movePosition(currentBeat->quartersPerBar);
                    continue;
                }
            }
        }

        if (noteIt->timestamp > blockEnd) {
            position = blockEnd;
            break;
        }

#if 0
        DBG("Seq: " << queuedSequences.size()
            << " | Pos/BlockEnd: " << position << "/" << blockEnd
            << " | current note (index/number/time/duration) : "
            << std::distance(queuedSequences.front()->begin(), noteIt) << "/"
            << +noteIt->number << "/" << noteIt->timestamp << "/" << noteIt->duration);
#endif

        const int noteOnDelay = midiDelay(noteIt->timestamp);
        const int noteOffDelay = midiDelay(noteIt->timestamp + noteIt->duration);
        const auto potentialMergeIt = std::find_if(
            potentialNotesToMerge.begin(),
            potentialNotesToMerge.end(),
            [&](const NoteEvents& evt) -> bool { return evt.number == noteIt->number; }
        );

        const auto deferNote = [&] {
            deferredNotes.push_back({ noteOnDelay, noteIt->number, noteIt->velocity });
            deferredNotes.push_back({ noteOffDelay, noteIt->number, 0.0f });
        };
        
        if (potentialMergeIt == potentialNotesToMerge.end()) {
            deferNote();
            potentialNotesToMerge.push_back({ noteOnDelay, noteIt->number, noteIt->velocity });
        } else {
            if (noteOnDelay - potentialMergeIt->delay > mergingThreshold) {
                deferNote();
            } else {
                // DBG("Merging note with number " << +noteIt->number);
            }
    
            potentialMergeIt->delay = noteOnDelay;
        }

        position = noteIt->timestamp;
        noteIt++;
    }

    std::sort(deferredNotes.begin(), deferredNotes.end(), [](const NoteEvents& lhs, const NoteEvents& rhs) {
        return lhs.delay < rhs.delay;
    });

    auto deferredIt = deferredNotes.begin();
    while (deferredIt != deferredNotes.end() && deferredIt->delay < sampleCount) {
        float velocity = clamp(deferredIt->velocity, 0.0f, 1.0f);
        noteCallback(deferredIt->delay, deferredIt->number, velocity);
        deferredIt++;
    }

    deferredNotes.erase(deferredNotes.begin(), deferredIt);

    for (auto& evt : deferredNotes)
        evt.delay -= sampleCount;

    auto potentialMergeIt = potentialNotesToMerge.begin();
    while (potentialMergeIt != potentialNotesToMerge.end()) {
        potentialMergeIt->delay -= sampleCount;

        if (-potentialMergeIt->delay > mergingThreshold) {
            potentialMergeIt = potentialNotesToMerge.erase(potentialMergeIt);
        } else {
            potentialMergeIt++;
        }
    }
}

bool Player::enteringFillInState() const
{
    return queuedSequences.size() == 3;
}

bool Player::enteringEndingState() const
{
    return queuedSequences.size() == 2 && state == State::Ending;
}

bool Player::leavingFillInState() const
{
    return queuedSequences.size() == 2 && state != State::Ending;
}

void Player::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    mergingThreshold = quarterToSamples(mergingQuarterFraction);
}

void Player::setTempo(double bpm)
{
    this->secondsPerQuarter = 60.0 / bpm;
    mergingThreshold = quarterToSamples(mergingQuarterFraction);
}

void Player::setNoteCallback(NoteCallback cb)
{
    noteCallback = std::move(cb);
}

const char* Player::getCurrentPartName()
{
    if (!currentBeat)
        return {};

    return currentBeat->parts[partIndex].name.c_str();
}

void Player::allOff()
{
    const std::unique_lock<std::mutex> lock { callbackGuard };
    reset();
}

double Player::totalDuration(const Sequence& sequence)
{
    if (sequence.empty())
        return 0.0;

    return std::ceil(sequence.back().timestamp);
}

bool Player::isPlaying() const
{
    return state != State::Stopped;
}

int Player::quarterToSamples(double quarterFraction) const noexcept
{
    return static_cast<int>(quarterFraction * secondsPerQuarter * sampleRate);
}

double Player::samplesToQuarter(int samples) const noexcept
{
    return static_cast<double>(samples) / sampleRate / secondsPerQuarter;
}

Player::State Player::getState() const noexcept
{
    return state;
}

double Player::getBarPosition() const noexcept
{
    if (!currentBeat)
        return {};   

    return std::fmod(this->position * 4.0 / currentBeat->signature.denom, 
        static_cast<double>(currentBeat->signature.num));
}

int Player::getPartIndex() const noexcept
{
    return partIndex;
}

int Player::getFillIndex() const noexcept
{
    return fillIndex;
}

void Player::suspendCallback() noexcept
{
    callbackGuard.lock();
}

void Player::resumeCallback() noexcept
{
    callbackGuard.unlock();
}

const Sequence* Player::getCurrentSequence() const noexcept
{
    if (queuedSequences.size() == 0)
        return {};

    return queuedSequences.front();
}

double Player::getSequencePosition() const noexcept
{
    return this->position;
}


}