#include "../include/synth.h"

#include "pluginterfaces/vst/ivstevents.h"

//#ifndef M_PI
//#define M_PI 3.14159265358979323846264338327950288f
//#endif

#ifndef M_PI_MUL_2_F
#define M_PI_MUL_2_F 6.283185307179586476925286766559f
#endif

namespace Benergy {
namespace GranCassa {

const float Synth::sMinVolumeDB = -80.f;

Synth::Synth()
	: mState(kOff)
	, mSampleRate(48000.0)
	, mAttackTime(0.01f)
	, mReleaseTime(0.1f)
	, mCurrentVolume(dBToFactor(sMinVolumeDB))
	, mVolume(0.9f)
	, mSamplesProcessed(0)
	, mRampMultiplier(0.f)
{}

tresult Synth::process(Vst::ProcessData& data)
{
	// ProcessData members:
	//------------------------------------------------------------------------
	//int32 processMode;			///< processing mode - value of \ref ProcessModes
	//int32 symbolicSampleSize;   ///< sample size - value of \ref SymbolicSampleSizes
	//int32 numSamples;			///< number of samples to process
	//int32 numInputs;			///< number of audio input busses
	//int32 numOutputs;			///< number of audio output busses
	//AudioBusBuffers* inputs;	///< buffers of input busses
	//AudioBusBuffers* outputs;	///< buffers of output busses

	//IParameterChanges* inputParameterChanges;	///< incoming parameter changes for this block
	//IParameterChanges* outputParameterChanges;	///< outgoing parameter changes for this block (optional)
	//IEventList* inputEvents;				///< incoming events for this block (optional)
	//IEventList* outputEvents;				///< outgoing events for this block (optional)
	//ProcessContext* processContext;			///< processing context (optional, but most welcome)
	//------------------------------------------------------------------------

	// Check if input events are present
	if (data.inputEvents)
	{
		int32_t numEvents = data.inputEvents->getEventCount();
		if (numEvents > 0)
		{
			int32_t eventIndex = 0;
			Vst::Event e;
			data.inputEvents->getEvent(eventIndex, e);

			while (eventIndex < numEvents)
			{
				// keep queue of noteOn events with sampleOffsets
				if (e.type == Vst::Event::kNoteOnEvent)
				{
					if (e.sampleOffset < data.numSamples)
					{
						mNoteOns.push(e.sampleOffset);
					}
				}

				if (data.inputEvents->getEvent(++eventIndex, e) != kResultOk)
				{
					break;
				}
			}
		}
	}

	// Do audio synthesis
	float** out = data.outputs[0].channelBuffers32;
	const int32_t numChannels = data.outputs[0].numChannels;
	const int32_t numSamples = data.numSamples;

	for (int32_t c = 0; c < numChannels; ++c)
	{
		memset(out[c], 0, sizeof(float) * numSamples);
	}

	for (int32_t s = 0; s < numSamples; ++s)
	{
		switch (mState)
		{
		case kOff:
			if (!mNoteOns.empty() && s == mNoteOns.front())
			{
				mNoteOns.pop();
				mState = kRising;
				mRampMultiplier = (logf(mVolume) - logf(mCurrentVolume)) / (mAttackTime * float(mSampleRate));
				mSamplesProcessed = 0;
			}
			// no break here, so this sample already gets processed
		case kRising:
			// if rising and new note-on coming in, retrigger with volume = 0.0
			if (!mNoteOns.empty() && s == mNoteOns.front())
			{
				mNoteOns.pop();
				mCurrentVolume = dBToFactor(sMinVolumeDB);
				mSamplesProcessed = 0;
			}
			// if mAttackTime is reached, change into release/falling
			else if (++mSamplesProcessed >= mAttackTime * mSampleRate)
			{
				mState = kFalling;
				mRampMultiplier = (logf(dBToFactor(sMinVolumeDB)) - logf(mCurrentVolume)) / (mReleaseTime * float(mSampleRate));
			}
			// if here just process sample
			else
			{
				for (int32_t c = 0; c < numChannels; ++c)
				{
					out[c][s] = mCurrentVolume * sinf(M_PI_MUL_2_F * 100.f * float(mSamplesProcessed) / float(mSampleRate));
				}
			}

			mCurrentVolume += mRampMultiplier * mCurrentVolume;

			break;
		case kFalling:
			// if falling and new note-on coming in, retrigger with volume = 0.0
			if (!mNoteOns.empty() && s == mNoteOns.front())
			{
				mNoteOns.pop();
				mCurrentVolume = dBToFactor(sMinVolumeDB);
				mSamplesProcessed = 0;
				mState = kRising;
			}
			// if mReleaseTime is reached or volume is very low, switch to off
			else if (++mSamplesProcessed >= (mAttackTime + mReleaseTime) * float(mSampleRate) ||
				factorTodB(mCurrentVolume) <= sMinVolumeDB)
			{
				mState = kOff;
				mRampMultiplier = 0.f;
			}
			// if here, just process sample
			else
			{
				for (int32_t c = 0; c < numChannels; ++c)
				{
					out[c][s] = mCurrentVolume * sinf(M_PI_MUL_2_F * 100.f * float(mSamplesProcessed) / float(mSampleRate));
				}
			}

			mCurrentVolume += mRampMultiplier * mCurrentVolume;

			break;
		}
	}

	// clean-up note-on event queue, in case some were missed (shouldn't happen)
	while (!mNoteOns.empty())
		mNoteOns.pop();

	return kResultOk;
}

}
}