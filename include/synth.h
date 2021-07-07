#pragma once

#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include <queue>

namespace Benergy {
namespace GranCassa {

using namespace Steinberg;

class Synth
{
public:

	Synth();

	tresult process(Vst::ProcessData& data);

	inline void setSampleRate(Vst::SampleRate sr) { mSampleRate = sr; }
	
	static inline float dBToFactor(float dB) { return powf(10.f, dB / 20.f); }
	static inline float factorTodB(float f)  { return 20.f * log10f(f); }
	static const float sMinVolumeDB;

private:

	enum State
	{
		kRising,
		kFalling,
		kOff,
		kNumStates
	};

	std::queue<int32_t> mNoteOns; // Queue of input note-on events, only sample offsets are saved
	State mState;
	Vst::SampleRate mSampleRate;

	float mAttackTime;
	float mReleaseTime;
	float mCurrentVolume;
	float mVolume;
	float mRampMultiplier;
	uint64_t mSamplesProcessed;
};

}
}