#include "GameTimer.h"

GameTimer::GameTimer()
	: m_SecondsPerCount(0), m_DeltaTime(-1.0), m_BaseTime(0), m_PausedTime(0), m_StopTime(0), m_PrevTime(0), m_CurrentTime(0), m_bIsStopped(false)
{
	__int64 countsPerSecond;
	::QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);

	m_SecondsPerCount = 1.0 / (double)countsPerSecond;
}

float GameTimer::TotalTime() const
{
	if (m_bIsStopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	return (float)(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
}

float GameTimer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void GameTimer::Reset()
{
	__int64 currentTime;
	::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_BaseTime = currentTime;
	m_PrevTime = currentTime;

	m_StopTime = 0;

	m_bIsStopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	::QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_bIsStopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PrevTime = startTime;
		m_StopTime = 0;

		m_bIsStopped = false;
	}
}

void GameTimer::Stop()
{
	if (!m_bIsStopped)
	{
		__int64 currentTime;
		::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		m_StopTime = currentTime;

		m_bIsStopped = true;
	}
}

void GameTimer::Tick()
{
	if (m_bIsStopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	__int64 currentTime;
	::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_CurrentTime = currentTime;

	m_DeltaTime = (m_CurrentTime - m_PrevTime) * m_SecondsPerCount;

	m_PrevTime = m_CurrentTime;

	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}
}