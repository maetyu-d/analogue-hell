#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "../src/Shared/AnalogHellDSP.h"

namespace
{
struct KWeight
{
    juce::dsp::IIR::Filter<float> hpL, hpR;
    juce::dsp::IIR::Filter<float> hsL, hsR;

    void prepare(double sampleRate)
    {
        auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 60.0, 0.5);
        auto hs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1500.0, 0.707f, juce::Decibels::decibelsToGain(4.0f));
        hpL.coefficients = hp;
        hpR.coefficients = hp;
        hsL.coefficients = hs;
        hsR.coefficients = hs;
        hpL.reset(); hpR.reset(); hsL.reset(); hsR.reset();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        auto* l = buffer.getWritePointer(0);
        auto* r = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            l[i] = hsL.processSample(hpL.processSample(l[i]));
            r[i] = hsR.processSample(hpR.processSample(r[i]));
        }
    }
};

float integratedLufsApprox(juce::AudioBuffer<float> buffer, double sampleRate)
{
    KWeight k;
    k.prepare(sampleRate);
    k.process(buffer);

    const auto* l = buffer.getReadPointer(0);
    const auto* r = buffer.getReadPointer(1);
    const int n = buffer.getNumSamples();
    const int block = juce::jmax(1, (int) std::round(sampleRate * 0.400));
    const int hop = juce::jmax(1, (int) std::round(sampleRate * 0.100));

    std::vector<double> blockEnergy;
    for (int start = 0; start + block <= n; start += hop)
    {
        double e = 0.0;
        for (int i = 0; i < block; ++i)
        {
            const int idx = start + i;
            e += 0.5 * (double(l[idx]) * double(l[idx]) + double(r[idx]) * double(r[idx]));
        }
        e /= (double) block;
        blockEnergy.push_back(juce::jmax(1.0e-12, e));
    }

    if (blockEnergy.empty())
        return -70.0f;

    auto lufsFromEnergy = [](double e) { return -0.691 + 10.0 * std::log10(juce::jmax(1.0e-12, e)); };

    // Absolute gate at -70 LUFS
    std::vector<double> absGated;
    absGated.reserve(blockEnergy.size());
    for (double e : blockEnergy)
        if (lufsFromEnergy(e) > -70.0)
            absGated.push_back(e);

    if (absGated.empty())
        return -70.0f;

    double prelim = 0.0;
    for (double e : absGated)
        prelim += e;
    prelim /= (double) absGated.size();
    const double prelimLufs = lufsFromEnergy(prelim);
    const double relGate = prelimLufs - 10.0;

    std::vector<double> relGated;
    relGated.reserve(absGated.size());
    for (double e : absGated)
        if (lufsFromEnergy(e) > relGate)
            relGated.push_back(e);

    if (relGated.empty())
        relGated = absGated;

    double integrated = 0.0;
    for (double e : relGated)
        integrated += e;
    integrated /= (double) relGated.size();

    return (float) lufsFromEnergy(integrated);
}

juce::AudioBuffer<float> makePink(double sampleRate, double seconds)
{
    const int numSamples = (int) std::round(sampleRate * seconds);
    juce::AudioBuffer<float> b(2, numSamples);
    juce::Random rng(12345);

    float rows[16] {};
    int counter = 0;

    for (int i = 0; i < numSamples; ++i)
    {
        counter++;
        int c = counter;
        int row = 0;
        while ((c & 1) == 0 && row < 16)
        {
            rows[row] = rng.nextFloat() * 2.0f - 1.0f;
            c >>= 1;
            row++;
        }

        float sum = 0.0f;
        for (float v : rows)
            sum += v;

        const float s = 0.07f * (sum / 16.0f) + 0.02f * (rng.nextFloat() * 2.0f - 1.0f);
        b.setSample(0, i, s);
        b.setSample(1, i, s * 0.98f);
    }

    return b;
}

juce::AudioBuffer<float> makeDrumLoopLike(double sampleRate, double seconds)
{
    const int numSamples = (int) std::round(sampleRate * seconds);
    juce::AudioBuffer<float> b(2, numSamples);
    juce::Random rng(7777);

    const int kickInt = (int) std::round(sampleRate * 0.5);
    const int snareInt = (int) std::round(sampleRate * 1.0);
    const int hatInt = (int) std::round(sampleRate * 0.25);

    float kickEnv = 0.0f, snareEnv = 0.0f, hatEnv = 0.0f;
    float kickPh = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        if (i % kickInt == 0)
            kickEnv = 1.0f;
        if ((i + snareInt / 2) % snareInt == 0)
            snareEnv = 1.0f;
        if (i % hatInt == 0)
            hatEnv = 1.0f;

        kickEnv *= 0.9992f;
        snareEnv *= 0.9962f;
        hatEnv *= 0.985f;

        kickPh += (45.0f + 60.0f * kickEnv) / (float) sampleRate;
        if (kickPh > 1.0f)
            kickPh -= 1.0f;

        const float kick = std::sin(juce::MathConstants<float>::twoPi * kickPh) * kickEnv;
        const float snare = (rng.nextFloat() * 2.0f - 1.0f) * snareEnv;
        const float hat = (rng.nextFloat() * 2.0f - 1.0f) * hatEnv;

        const float mono = 0.48f * kick + 0.24f * snare + 0.13f * hat;
        b.setSample(0, i, mono + 0.03f * hat);
        b.setSample(1, i, mono - 0.03f * hat);
    }

    return b;
}

float oldTrimForModel(int model)
{
    switch (model)
    {
        case 1: return 0.86f;
        case 2: return 0.78f;
        case 3: return 0.81f;
        case 4: return 0.80f;
        case 5: return 0.74f;
        case 6: return 0.88f;
        case 7: return 0.84f;
        case 8: return 0.76f;
        case 9: return 0.83f;
        case 10: return 0.87f;
        default: return 0.85f;
    }
}

float clampTrim(float t)
{
    return juce::jlimit(0.60f, 1.10f, t);
}
}

int main()
{
    const double sampleRate = 48000.0;
    const double seconds = 16.0;

    const auto pink = makePink(sampleRate, seconds);
    const auto drum = makeDrumLoopLike(sampleRate, seconds);

    ah::Params params; // defaults used by plugin startup

    struct Entry { int model; float lufsPink; float lufsDrum; float avg; float oldTrim; float recTrim; };
    std::vector<Entry> entries;
    entries.reserve(10);

    for (int model = 1; model <= 10; ++model)
    {
        ah::ModelProcessor p1;
        p1.prepare(sampleRate, 2);
        auto b1 = pink;
        p1.process(b1, params, model);
        const float lp = integratedLufsApprox(std::move(b1), sampleRate);

        ah::ModelProcessor p2;
        p2.prepare(sampleRate, 2);
        auto b2 = drum;
        p2.process(b2, params, model);
        const float ld = integratedLufsApprox(std::move(b2), sampleRate);

        Entry e;
        e.model = model;
        e.lufsPink = lp;
        e.lufsDrum = ld;
        e.avg = 0.5f * (lp + ld);
        e.oldTrim = oldTrimForModel(model);
        e.recTrim = e.oldTrim;
        entries.push_back(e);
    }

    float target = 0.0f;
    for (const auto& e : entries)
        target += e.avg;
    target /= (float) entries.size();

    std::cout << "Target avg LUFS: " << target << "\n\n";
    std::cout << "model,pink_lufs,drum_lufs,avg_lufs,old_trim,recommended_trim\n";
    for (auto& e : entries)
    {
        const float deltaDb = target - e.avg;
        const float factor = std::pow(10.0f, deltaDb / 20.0f);
        e.recTrim = clampTrim(e.oldTrim * factor);
        std::cout << e.model << ","
                  << e.lufsPink << ","
                  << e.lufsDrum << ","
                  << e.avg << ","
                  << e.oldTrim << ","
                  << e.recTrim << "\n";
    }

    return 0;
}
