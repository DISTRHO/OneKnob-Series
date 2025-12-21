#pragma once
#include "BeatDescription.h"
#include "atomic_queue/atomic_queue.h"
#include <atomic>
#include <mutex>

namespace batteur {

using NoteCallback = std::function<void(int, uint8_t, float)>;

class Player {
public:
    Player();
    bool loadBeatDescription(const BeatDescription& description);
    const BeatDescription* getBeatDescription() { return currentBeat; }
    const Sequence* getCurrentSequence() const noexcept;
    double getTempo() { return 60.0 / secondsPerQuarter; }
    bool start();
    bool stop();
    bool fillIn();
    bool next();
    void tick(int sampleCount);
    bool isPlaying() const;
    void allOff();
    void setSampleRate(double sampleRate);
    void setTempo(double bpm);
    void setNoteCallback(NoteCallback cb);
    const char* getCurrentPartName();
    enum class State { Stopped, Intro, Playing, Fill, Next, Ending };
    State getState() const noexcept;
    double getBarPosition() const noexcept;
    double getSequencePosition() const noexcept;
    int getPartIndex() const noexcept;
    int getFillIndex() const noexcept;
    void suspendCallback() noexcept;
    void resumeCallback() noexcept;
private:
    struct NoteEvents {
        int delay;
        uint8_t number;
        float velocity;
    };

    void updateState();
    void _start();
    void _stop();
    void _fillIn();
    void _next();

    void reset();

    enum class Message { Start = 1, Stop, Fill, Next };
    State state { State::Stopped };
    template<class T, unsigned N>
    using spsc_queue = atomic_queue::AtomicQueue<T, N, T{}, false, false, false, true>;
    spsc_queue<Message, 32> messages;
    static double totalDuration(const Sequence& sequence);
    bool enteringFillInState() const;
    bool enteringEndingState() const;
    bool leavingFillInState() const;
    const BeatDescription* currentBeat { nullptr };
    double position { 0.0 };
    std::vector<const Sequence*> queuedSequences;
    std::vector<NoteEvents> deferredNotes;
    NoteCallback noteCallback {};
    double secondsPerQuarter { 0.5 };
    double sampleRate { 48e3 };
    int fillIndex { 0 };
    int partIndex { 0 };
    std::mutex callbackGuard;

    int quarterToSamples(double quarterFraction) const noexcept;
    double samplesToQuarter(int samples) const noexcept;

    static constexpr double mergingQuarterFraction { 0.05 };
    std::vector<NoteEvents> potentialNotesToMerge;
    int mergingThreshold { static_cast<int>(mergingQuarterFraction * secondsPerQuarter * sampleRate) };
};

}