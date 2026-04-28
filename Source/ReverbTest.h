/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             SimpleReverbPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Noise gate audio plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        SimpleReverb

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class SimpleReverb final : public AudioProcessor
{
public:
    //==============================================================================
    SimpleReverb()
        : AudioProcessor (BusesProperties().withInput  ("Input",     AudioChannelSet::stereo())
                                           .withOutput ("Output",    AudioChannelSet::stereo()))
    {
        addParameter (loudness = new AudioParameterFloat ({ "loudness", 1 }, "Loudness", 0.0f, 1.0f, 0.5f));
        addParameter (delay = new AudioParameterFloat ({ "delay", 1 }, "Delay", 0.0f, 0.99f, 0.02f));
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // the sidechain can take any layout, the main bus needs to be the same on the input and output
        return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
                 && ! layouts.getMainInputChannelSet().isDisabled();
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        const int maxDelaySamples = static_cast<int> (sampleRate); // 1 second max
        delayBuffer.setSize (getTotalNumOutputChannels(), maxDelaySamples, false, true, true);
        delayBuffer.clear();
        writePosition = 0;
    }

    void releaseResources() override {}


    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto loudnessCopy = loudness->get(); // feedback gain
        auto delayCopy = delay->get();       // delay time in seconds

        const int delaySamples = static_cast<int> (delayCopy * getSampleRate());
        const int numChannels  = buffer.getNumChannels();
        const int numSamples   = buffer.getNumSamples();
        const int requiredBufferSize = delaySamples + numSamples;

        if (delayBuffer.getNumChannels() != numChannels || delayBuffer.getNumSamples() < requiredBufferSize)
        {
            delayBuffer.setSize (numChannels, requiredBufferSize, false, true, true);
            delayBuffer.clear();
            writePosition = 0;
        }

        const int bufferSize = delayBuffer.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const int readIndex = (writePosition - delaySamples + bufferSize) % bufferSize;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* channelData = buffer.getWritePointer(ch);
                float* delayData   = delayBuffer.getWritePointer(ch);

                const float y = channelData[sample] + loudnessCopy * delayData[readIndex];
                channelData[sample] = y;
                delayData[writePosition] = y;
            }

            writePosition = (writePosition + 1) % bufferSize;
        }
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    AudioProcessorEditor* createEditor() override            { return new GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                          { return true; }
    const String getName() const override                    { return "SimpleReverb"; }
    bool acceptsMidi() const override                        { return false; }
    bool producesMidi() const override                       { return false; }
    double getTailLengthSeconds() const override             { return delay->get(); }
    int getNumPrograms() override                            { return 1; }
    int getCurrentProgram() override                         { return 0; }
    void setCurrentProgram (int) override                    {}
    const String getProgramName (int) override               { return "None"; }
    void changeProgramName (int, const String&) override     {}
    bool isVST2() const noexcept                             { return (wrapperType == wrapperType_VST); }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*loudness);
        stream.writeFloat (*delay);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        loudness->setValueNotifyingHost (stream.readFloat());
        delay->setValueNotifyingHost     (stream.readFloat());
    }

private:
    //==============================================================================
    AudioParameterFloat* loudness;
    AudioParameterFloat* delay;
    AudioBuffer<float> delayBuffer;
    int writePosition = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleReverb)
};
