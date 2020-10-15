#pragma once

#include "EmulatorCommon.h"

/*
* High resolution timer for accurately emulating clock cycles
*/
class GameTimer
{
public:
	GameTimer();

	/// <summary>
	/// The total time elapsed since 'Reset()' was called. Not counting any time when the timer was stopped.
	/// </summary>
	/// <returns>Total time elapsed in seconds since the last Reset()</returns>
	float TotalTime() const;

	/// <summary>
	/// The time elapsed in seconds between the last tick of the timer and the current tick
	/// </summary>
	/// <returns>Time between ticks in seconds</returns>
	float DeltaTime() const;

	/// <summary>
	/// Resets the timer to count from the current time now.
	/// Should be called before the main message loop.
	/// </summary>
	void Reset();

	/// <summary>
	/// Starts the timer counting and calculates the time spent paused.
	/// Should be called when unpausing the timer.
	/// </summary>
	void Start();
	
	/// <summary>
	/// Stops the timer counting.
	/// Should be called when pausing the timer
	/// </summary>
	void Stop();

	/// <summary>
	/// Ticks the timer. Should be called every frame
	/// </summary>
	void Tick();

private:
	// Resolution of the timer
	double m_SecondsPerCount;

	// The current delta time
	double m_DeltaTime;

	// Time when the timer started counting/ticking
	__int64 m_BaseTime;

	// Time when the timer was paused
	__int64 m_PausedTime;

	// Time when the timer was stopped
	__int64 m_StopTime;

	// Previous start time of the timer
	__int64 m_PrevTime;

	// The current elapsed time
	__int64 m_CurrentTime;

	// True if the timer is stopped and not running
	bool m_bIsStopped;
};