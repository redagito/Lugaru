#pragma once

// LoadingClock paces the loading overlay drawn by Game::LoadingScreen against
// the wall clock.
//
// The overlay used to advance its 0 -> 100 brightness ramp, and bleed off the
// red flash, by the game-wide frame delta. That delta was measured against a
// function-local timestamp starting at zero, so the very first sample counted
// the whole process uptime, and it was throttled to at most one redraw every
// 50ms. Together those made the ramp saturate in a handful of redraws and pin
// the flash at full alpha for the entire load.
//
// This struct owns no GL state, calls no platform API and holds no reference to
// the game globals: the caller feeds it measured real seconds and reads back a
// ramp and a flash decay that depend on how much time passed, never on how
// often it was called.

struct LoadingClock
{
	// Seconds of real time for a full 0 -> 100 ramp. A level load is on screen
	// for a second or two, so the ramp is spread across two seconds: long
	// enough to read as a slow pulse instead of a strobe, short enough that a
	// typical load still reaches full brightness before it ends.
	static constexpr float rampSeconds = 2.0f;

	// Alpha per second taken off the red flash. One unit per second bleeds a
	// full-strength flash off in one second.
	static constexpr float flashDecayPerSecond = 1.0f;

	// Adds `seconds` of real elapsed time and returns how much was added, which
	// is zero on the very first call: that call only establishes the baseline,
	// so a delta measured against a zero-initialised timestamp can never be
	// counted. Deltas that are not positive finite durations (the clock
	// standing still, or stepping backwards) also add nothing.
	float advance(float seconds)
	{
		if (!primed_) {
			primed_ = true;
			return 0.0f;
		}
		if (!(seconds > 0.0f) || seconds > maxStepSeconds) {
			return 0.0f;
		}
		elapsed_ += seconds;
		return seconds;
	}

	// True once advance() has been called at least once, i.e. once there is a
	// baseline to measure against.
	bool primed() const
	{
		return primed_;
	}

	// Starts a fresh 0 -> 100 ramp, dropping the time banked so far. The
	// baseline is left alone: the caller keeps supplying measured deltas, so the
	// next advance() must still be counted.
	void reset()
	{
		elapsed_ = 0.0f;
	}

	// Total real seconds counted since the baseline was taken.
	float elapsed() const
	{
		return elapsed_;
	}

	// Brightness ramp in 0 -> 100, saturating at 100 once rampSeconds have
	// passed.
	float ramp() const
	{
		if (elapsed_ >= rampSeconds) {
			return 100.0f;
		}
		return elapsed_ / rampSeconds * 100.0f;
	}

	// `flash` (0 -> 1 alpha) after `seconds` of decay. Only ever decreases, and
	// clamps at exactly zero rather than going negative.
	static float decayFlash(float flash, float seconds)
	{
		if (!(seconds > 0.0f) || seconds > maxStepSeconds) {
			return flash;
		}
		const float decayed = flash - seconds * flashDecayPerSecond;
		if (decayed < 0.0f) {
			return 0.0f;
		}
		if (decayed > flash) {
			return flash;
		}
		return decayed;
	}

private:
	// Guards against a single absurd delta (a NaN, or a timestamp that wrapped)
	// poisoning the accumulator, which would end the ramp for good.
	static constexpr float maxStepSeconds = 3600.0f;

	float elapsed_ = 0.0f;
	bool primed_ = false;
};