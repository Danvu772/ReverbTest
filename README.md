# SimpleReverb

A minimal VST3/AU reverb plugin built with JUCE. One-day project to get familiar with VST plugin development.

Implements a feedback comb filter which is the core building block of reverb. The algorithm is `y[n] = x[n] + g * y[n - D]`, where `g` is feedback gain and `D` is the delay in samples.

Two parameters exposed to the host:
- **Loudness**: feedback gain (0.0–1.0). Higher values = longer, more ringy decay.
- **Delay**: delay time (0.0–0.99 seconds). Shorter = metallic/comb effect, longer = distinct echo.

## Building

Requires [JUCE](https://juce.com) installed locally.

### macOS

Open `Builds/MacOSX/ReverbTest.xcodeproj` in Xcode and build. Targets AU and VST3.

### Windows

Open `Builds/VisualStudio2022/ReverbTest.sln` in Visual Studio 2022 and build.

### Regenerating project files

If you change `NoiseGatePlugin.jucer`, open it in Projucer and re-export.
