# Analogue Hell

![](https://github.com/maetyu-d/analogue-hell/blob/main/gordon-ramsay-1-7bab40d4093b496da5efa608670c5a7d.jpg)

A suite of 10 plugins that explore analogue breakdown, malfunction, and haunted calibration aesthetics. To put another way, if they were a hotel, they'd be the kind that Gordon Ramsay visits and, for reasons unknown, insists on baring his bum in... Created using JUCE.

## Plugins

1. `AH Tape System` (`AHTapeSystem`)  
   Bias miscalibration, hysteresis/memory saturation, wow/flutter networks, ghost pre-echo, splice ticks.
2. `AH BBD Grime` (`AHBbdGrime`)  
   Clock bleed, compander breathing, sample/hold grime.
3. `AH Core Bend` (`AHCoreBend`)  
   Transformer-like core bend, low-end hysteresis, DC sensitivity.
4. `AH Tube Misuse` (`AHTubeMisuse`)  
   Blocking distortion, supply sag, microphonic ring.
5. `AH Fault Fuzz` (`AHFaultFuzz`)  
   Bias drift, brittle crossover, octave/fold behavior.
6. `AH Bus Overload` (`AHBusOverload`)  
   Slew limiting, interstage loading feel, crosstalk.
7. `AH Unstable Filter` (`AHUnstableFilter`)  
   Near-self-oscillation, per-channel mismatch, noise-modulated cutoff.
8. `AH Spring Chaos` (`AHSpringChaos`)  
   Spring boing nonlinearity, mechanical crash/hum behavior.
9. `AH Vinyl Weapon` (`AHVinylWeapon`)  
   Off-center drift, rumble, static bursts, inner-groove pinch.
10. `AH CalLab Ghost` (`AHCalLabGhost`)  
    Imperfect generator tones, VCO drift, AM/FM ghost bleed.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release -j 6
```

VST3 bundles are emitted under:

- `/Users/md/Downloads/analogue hell plugin suite/build/<Target>_artefacts/VST3/*.vst3`

## Shared Architecture

- Single reusable processor/editor implementation.
- Model selection via compile-time `AH_MODEL_ID` per plugin target.
- Shared DSP core in `src/Shared/AnalogHellDSP.h`.
- Common macro controls: `Drive`, `Mix`, `Depth`, `Rate`, `Character`, `Noise`, `Unstable`, `Stereo`.
