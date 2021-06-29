module;
import <unordered_map>;
import <string>
export module Profiler;

#define PROFILE_START gProfiler.Start(__func__); 
#define PROFILE_END gProfiler.End(__func__); 

//An period of measurement between Start() and End() calls.
export struct TimeFrame
{
	TimeFrame() {}
	TimeFrame(__int64 _startTime)
	{
		startTime = _startTime;
		memset(elapsedTimes, 0.0, sizeof(double) * maxSampleSize);
	}

	void TimeFrame::SetElapsedTime()
	{
		__int64 cpuFreq;
		QueryPerformanceFrequency((LARGE_INTEGER*)&cpuFreq);
		double ticks = 1.0 / (double)cpuFreq;

		if (currentElapsedTimeIndex < maxSampleSize)
		{
			elapsedTimes[currentElapsedTimeIndex] = ticks * (double)(endTime - startTime);
			currentElapsedTimeIndex++;
		}
		else
		{
			currentElapsedTimeIndex = 0;
		}

		endTime = 0;
		startTime = 0;
	}

	double TimeFrame::GetAverageTime()
	{
		double averageTime = 0.0;

		for (int i = 0; i < maxSampleSize; i++)
		{
			averageTime += elapsedTimes[i];
		}

		double retVal = averageTime / (double)maxSampleSize;
		return retVal;
	}

	const static int maxSampleSize = 60;

	double elapsedTimes[maxSampleSize];
	int currentElapsedTimeIndex = 0;
	__int64 startTime;
	__int64 endTime;
};

//Simpler profile calls to push into function head and tails for measurement.
//Due to it working off a map, you can only have one profiling pair per function for now.
export class Profiler
{
public:
	void Init()
	{

	}

	void Reset()
	{
		timeFrames.empty();
	}

	void Start(const char* functionName)
	{
		__int64 startTime = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		auto timeFramesIt = timeFrames.find(functionName);
		if (timeFramesIt == timeFrames.end())
		{
			TimeFrame* timeFrame = new TimeFrame(startTime);
			timeFrames[functionName] = timeFrame;
		}
		else
		{
			timeFrames[functionName]->startTime = startTime;
		}
	}

	void End(const char* functionName)
	{
		__int64 endTime = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&endTime);

		auto timeFramesIt = timeFrames.find(functionName);
		assert(timeFramesIt->second && "Check for matching PROFILE_START in function.");

		TimeFrame* currentTimeFrame = timeFramesIt->second;
		currentTimeFrame->endTime = endTime;
		currentTimeFrame->SetElapsedTime();
	}

	//Each time frame is a sample between a Begin() and End() call.
	std::unordered_map<std::string, TimeFrame*> timeFrames;
};

export Profiler gProfiler;