#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <array>

namespace ah
{
struct Params
{
    float drive = 0.5f;
    float mix = 1.0f;
    float depth = 0.5f;
    float rateHz = 1.5f;
    float character = 0.5f;
    float noise = 0.2f;
    float unstable = 0.5f;
    float stereo = 0.5f;
};

struct ChannelState
{
    float memory = 0.0f;
    float hysteresis = 0.0f;
    float wowPhaseA = 0.0f;
    float wowPhaseB = 0.0f;
    float randomWalk = 0.0f;
    float flutterBurst = 0.0f;
    float dcFollower = 0.0f;
    float sag = 0.0f;
    float blocker = 0.0f;
    float sampleHold = 0.0f;
    float sampleHoldPhase = 0.0f;
    float noiseGate = 0.0f;
    float crosstalkMemory = 0.0f;
    float filterState = 0.0f;
    float filterBand = 0.0f;
    float selfOsc = 0.0f;
    float rumble = 0.0f;
    float ghostDelay[128] {};
    int ghostIndex = 0;
};

class ModelProcessor
{
public:
    void prepare(double sampleRate, int numChannels)
    {
        fs = static_cast<float>(sampleRate);
        channels = juce::jlimit(1, 2, numChannels);
        rng.setSeedRandomly();

        for (auto& s : state)
        {
            s = {};
        }
    }

    void process(juce::AudioBuffer<float>& buffer, const Params& p, int modelId)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return;

        for (int ch = 0; ch < juce::jmin(channels, buffer.getNumChannels()); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            auto& s = state[(size_t) ch];
            const float side = (ch == 0 ? -1.0f : 1.0f) * p.stereo * 0.5f;

            for (int i = 0; i < numSamples; ++i)
            {
                const float dry = data[i];
                const float n = (rng.nextFloat() * 2.0f - 1.0f);
                s.randomWalk = juce::jlimit(-1.0f, 1.0f, s.randomWalk * 0.9995f + n * 0.003f * p.unstable);

                float wet = dry;

                switch (modelId)
                {
                    case 1: wet = processTape(dry, p, s, side, n); break;
                    case 2: wet = processBbd(dry, p, s, side, n); break;
                    case 3: wet = processCore(dry, p, s, side, n); break;
                    case 4: wet = processTube(dry, p, s, side, n); break;
                    case 5: wet = processFuzz(dry, p, s, side, n); break;
                    case 6: wet = processBus(dry, p, s, side, n); break;
                    case 7: wet = processFilter(dry, p, s, side, n); break;
                    case 8: wet = processSpring(dry, p, s, side, n); break;
                    case 9: wet = processVinyl(dry, p, s, side, n); break;
                    case 10: wet = processCalLab(dry, p, s, side, n); break;
                    default: wet = dry; break;
                }

                float out = dry + (wet - dry) * p.mix;
                out *= getModelOutputTrim(modelId);
                data[i] = juce::jlimit(-1.0f, 1.0f, out);
            }
        }

        if (buffer.getNumChannels() > 1)
        {
            auto* l = buffer.getWritePointer(0);
            auto* r = buffer.getWritePointer(1);
            const float bleed = 0.02f + 0.25f * paramsSafeLast.stereo;
            for (int i = 0; i < numSamples; ++i)
            {
                const float ll = l[i];
                const float rr = r[i];
                l[i] = ll + rr * bleed * 0.05f;
                r[i] = rr + ll * bleed * 0.05f;
            }
        }

        paramsSafeLast = p;
    }

private:
    float getModelOutputTrim(int modelId) const
    {
        switch (modelId)
        {
            case 1: return 0.828f; // Tape System
            case 2: return 0.552f; // BBD Grime
            case 3: return 1.021f; // Core Bend
            case 4: return 0.680f; // Tube Misuse
            case 5: return 0.524f; // Fault Fuzz
            case 6: return 0.824f; // Bus Overload
            case 7: return 1.187f; // Unstable Filter
            case 8: return 0.538f; // Spring Chaos
            case 9: return 1.172f; // Vinyl Weapon
            case 10: return 1.200f; // CalLab Ghost
            default: return 0.85f;
        }
    }

    float processTape(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float bias = (p.character - 0.5f) * 1.3f + s.randomWalk * 0.2f;
        s.memory = s.memory * 0.995f + x * 0.005f;
        s.hysteresis = 0.985f * s.hysteresis + 0.015f * (x - s.memory);

        s.wowPhaseA += (0.12f + p.rateHz * 0.2f) / fs;
        s.wowPhaseB += (0.03f + p.rateHz * 0.09f) / fs;
        if (s.wowPhaseA > 1.0f) s.wowPhaseA -= 1.0f;
        if (s.wowPhaseB > 1.0f) s.wowPhaseB -= 1.0f;
        const float wow = std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseA)
                        + 0.6f * std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseB + side);

        s.flutterBurst = juce::jmax(0.0f, s.flutterBurst - 1.5f / fs);
        if (std::abs(n) > 0.995f && p.unstable > 0.4f)
            s.flutterBurst = 1.0f;

        const float flutter = s.flutterBurst * std::sin(juce::MathConstants<float>::twoPi * (70.0f / fs) * sampleCounter++);
        const float speed = 1.0f + 0.012f * p.depth * wow + 0.008f * flutter;

        float y = x * speed;
        y -= y * juce::jlimit(0.0f, 0.85f, (0.5f - p.character) * 0.75f + 0.15f * std::abs(y));
        y = std::tanh((y + bias + s.hysteresis * 0.4f) * (1.0f + p.drive * 7.0f));

        const int preDelay = 90;
        const int idx = s.ghostIndex;
        const int ghostRead = (idx + 128 - preDelay) % 128;
        const float ghost = s.ghostDelay[ghostRead] * (0.06f + p.depth * 0.08f);
        s.ghostDelay[idx] = x;
        s.ghostIndex = (idx + 1) % 128;

        const float spliceTick = (std::abs(n) > 0.998f && p.unstable > 0.65f) ? (0.02f * n) : 0.0f;
        return y + ghost + spliceTick + n * p.noise * 0.02f;
    }

    float processBbd(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float shortDelayStress = 1.0f - p.depth;
        const float holdRate = juce::jmax(0.001f, (50.0f + p.rateHz * 800.0f) / fs);

        s.sampleHoldPhase += holdRate;
        if (s.sampleHoldPhase >= 1.0f)
        {
            s.sampleHoldPhase -= 1.0f;
            s.sampleHold = x;
        }

        const float bleed = std::sin(juce::MathConstants<float>::twoPi * (2600.0f / fs) * sampleCounter++)
                          * (0.002f + 0.03f * shortDelayStress * p.noise);
        const float companderEnv = 0.995f * s.noiseGate + 0.005f * std::abs(x);
        s.noiseGate = companderEnv;
        const float breathing = (0.08f + 0.45f * p.character) * (1.0f - companderEnv);

        float y = s.sampleHold + bleed + n * (0.0005f + breathing * 0.015f);
        y = std::tanh(y * (1.0f + p.drive * 9.0f));
        y += std::sin(y * (4.0f + 12.0f * p.character + side * 2.0f)) * 0.08f * p.depth;
        return y;
    }

    float processCore(float x, const Params& p, ChannelState& s, float side, float n)
    {
        s.dcFollower = 0.9992f * s.dcFollower + 0.0008f * x;
        const float dcSkew = s.dcFollower * (0.7f + p.unstable);
        const float lowBend = std::tanh((x + s.hysteresis * 0.4f) * (1.0f + p.drive * 6.0f));

        s.hysteresis = 0.97f * s.hysteresis + 0.03f * lowBend;
        float y = lowBend + std::sin((lowBend + dcSkew) * (2.0f + 10.0f * p.character + side));
        y *= 0.65f;
        y += (x - y) * (0.1f + 0.4f * p.depth);
        y += n * p.noise * 0.01f;
        return y;
    }

    float processTube(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float in = x * (1.0f + p.drive * 20.0f);
        const float bias = (p.character - 0.5f) * 0.9f + s.randomWalk * 0.08f;
        const float gridV = in + bias;

        // Positive grid conduction creates hard blocking recovery after peaks.
        const float gridCurrent = juce::jmax(0.0f, gridV - 0.18f) * (0.9f + p.drive * 2.6f);
        const float over = juce::jmax(0.0f, std::abs(gridV) - 0.62f);

        const float blockAttack = 0.020f + 0.035f * p.depth;
        const float blockRelease = 1.0f - (0.35f + 2.3f * (1.0f - p.depth)) / fs;
        s.blocker = juce::jmax(0.0f, s.blocker * blockRelease + (gridCurrent + over * 0.45f) * blockAttack);

        const float sagTarget = juce::jlimit(0.0f, 1.0f, 0.24f * std::abs(gridV) + 0.55f * gridCurrent);
        const float sagCoeff = (sagTarget > s.sag) ? (0.010f + 0.014f * p.drive) : (0.00030f + 0.0012f * (1.0f - p.depth));
        s.sag = juce::jlimit(0.0f, 1.0f, s.sag + (sagTarget - s.sag) * sagCoeff);

        const float blockingBias = s.blocker * (0.45f + 1.2f * p.depth);
        const float powerGain = 2.7f - 1.9f * s.sag;
        const float staged = (gridV - blockingBias) * powerGain;

        // Asymmetric transfer for more obvious tube-like odd/even interaction.
        const float pos = std::tanh(staged * (2.8f + p.drive * 6.5f));
        const float neg = std::tanh(staged * (1.6f + p.drive * 3.8f));
        float y = (staged >= 0.0f ? pos : neg);

        const float odd = std::sin(y * (6.0f + 13.0f * p.character)) * (0.05f + 0.16f * p.drive);
        const float choke = juce::jlimit(0.20f, 1.0f, 1.0f - s.blocker * (0.38f + 0.42f * p.depth));
        y = (y + odd) * choke;

        // Real modulation movement for microphonics/ringing.
        s.wowPhaseA += (170.0f + 1400.0f * p.depth) / fs;
        s.wowPhaseB += (420.0f + 2300.0f * p.unstable) / fs;
        if (s.wowPhaseA > 1.0f) s.wowPhaseA -= 1.0f;
        if (s.wowPhaseB > 1.0f) s.wowPhaseB -= 1.0f;

        const float transient = juce::jmax(0.0f, std::abs(in - s.memory) - 0.04f);
        s.memory = in;
        s.filterBand = s.filterBand * (0.989f - 0.006f * p.depth) + transient * (0.050f + 0.070f * p.depth);
        const float ring = std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseA + side)
                         + 0.52f * std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseB - side * 0.35f);
        const float microphonic = ring * s.filterBand * (0.06f + 0.26f * p.depth);

        return y + microphonic + n * p.noise * 0.010f;
    }

    float processFuzz(float x, const Params& p, ChannelState& s, float side, float n)
    {
        s.randomWalk = juce::jlimit(-1.0f, 1.0f, s.randomWalk + n * 0.0007f * (0.5f + p.unstable));
        const float bias = 0.2f * s.randomWalk + (p.character - 0.5f) * 0.7f;

        const float a = std::abs(x + bias);
        const float c = (x + bias > 0.0f ? 1.0f : -1.0f);
        float y = c * std::pow(juce::jlimit(0.0f, 1.0f, a), 0.25f + 0.7f * (1.0f - p.drive));

        const float crossover = juce::jlimit(-1.0f, 1.0f, y + 0.25f * std::sin(22.0f * y));
        const float octave = std::abs(y) * 2.0f - 1.0f;
        y = juce::jmap(p.depth, crossover, 0.7f * crossover + 0.7f * octave * std::sin(side + 0.7f));

        return std::tanh((y + n * p.noise * 0.02f) * (1.0f + p.drive * 4.0f));
    }

    float processBus(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float target = x * (1.0f + p.drive * 10.0f);
        const float slew = 0.003f + 0.15f * (1.0f - p.character);
        const float delta = juce::jlimit(-slew, slew, target - s.memory);
        s.memory += delta;

        s.crosstalkMemory = 0.995f * s.crosstalkMemory + 0.005f * (s.memory + side * 0.05f);
        float y = s.memory + s.crosstalkMemory * (0.01f + p.stereo * 0.03f);
        y = std::tanh(y);
        y += n * p.noise * 0.007f;
        return y;
    }

    float processFilter(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float f = juce::jlimit(0.001f, 0.35f,
                                     (0.01f + 0.18f * p.rateHz / 10.0f) * (1.0f + side * 0.08f + n * p.unstable * 0.01f));
        const float q = 0.2f + p.drive * 2.7f;

        s.filterBand += f * (x - s.filterState - q * s.filterBand);
        s.filterState += f * s.filterBand;

        s.selfOsc = 0.9995f * s.selfOsc + 0.0005f * std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * sampleCounter / fs);
        const float ringing = (p.character > 0.7f ? s.selfOsc * (p.character - 0.7f) * 2.5f : 0.0f);

        return std::tanh((s.filterState + ringing) * (1.0f + p.depth * 4.0f)) + n * p.noise * 0.008f;
    }

    float processSpring(float x, const Params& p, ChannelState& s, float side, float n)
    {
        const float decay = 0.985f + p.character * 0.012f;
        s.filterBand = decay * s.filterBand + x * (0.05f + 0.18f * p.depth);
        s.filterState = 0.97f * s.filterState + s.filterBand;

        const float boing = std::sin((180.0f + 2200.0f * std::abs(x)) * sampleCounter / fs + side) * (0.03f + p.drive * 0.1f);
        const float crash = (std::abs(x) > 0.95f && p.unstable > 0.5f) ? n * 0.2f : 0.0f;

        float y = s.filterState + boing + crash;
        y += n * (0.001f + p.noise * 0.02f);
        y += 0.01f * std::sin(2.0f * juce::MathConstants<float>::pi * 50.0f * sampleCounter / fs);
        return std::tanh(y * 1.4f);
    }

    float processVinyl(float x, const Params& p, ChannelState& s, float side, float n)
    {
        s.wowPhaseA += (0.55f / fs);
        if (s.wowPhaseA > 1.0f)
            s.wowPhaseA -= 1.0f;

        const float offCenter = std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseA + side) * 0.02f * p.depth;
        const float rumbleIn = std::sin(2.0f * juce::MathConstants<float>::pi * 27.0f * sampleCounter / fs + side) * 0.03f;
        s.rumble = 0.995f * s.rumble + 0.005f * rumbleIn;

        const float pinch = juce::jlimit(0.0f, 0.95f, p.character * 0.7f + std::abs(x) * 0.35f);
        float y = std::tanh((x + offCenter) * (1.0f + p.drive * 5.0f));
        y -= y * pinch * 0.25f;

        const float staticBurst = (std::abs(n) > 0.998f ? n * 0.15f : 0.0f);
        return y + s.rumble + staticBurst + n * p.noise * 0.01f;
    }

    float processCalLab(float x, const Params& p, ChannelState& s, float side, float n)
    {
        s.wowPhaseA += (p.rateHz * 0.15f + 0.04f) / fs;
        if (s.wowPhaseA > 1.0f)
            s.wowPhaseA -= 1.0f;

        const float vco = std::sin(juce::MathConstants<float>::twoPi * s.wowPhaseA + s.randomWalk * 0.2f);
        const float triangle = 2.0f * std::abs(2.0f * s.wowPhaseA - 1.0f) - 1.0f;
        const float source = juce::jmap(p.character, vco, triangle);

        const float am = x * (1.0f + source * (0.1f + p.depth * 0.7f));
        const float fmGhost = std::sin((1200.0f + 800.0f * source) * sampleCounter / fs + side) * 0.05f;
        const float heterodyne = std::sin((2200.0f + 400.0f * n) * sampleCounter / fs) * (0.02f + p.noise * 0.08f);

        return std::tanh((am + fmGhost + heterodyne) * (1.0f + p.drive * 2.0f));
    }

    float fs = 44100.0f;
    int channels = 2;
    uint64_t sampleCounter = 0;
    juce::Random rng;
    std::array<ChannelState, 2> state {};
    Params paramsSafeLast {};
};
} // namespace ah
