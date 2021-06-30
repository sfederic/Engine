module;
import <vector>;
export module TimerSystem;

export struct TimerItem
{
	double endTime;
	double currentTime;
	void (*functionToCall)();
};

export class TimerSystem
{
public:
	void Tick(float deltaTime)
	{
		for (int timerIndex = 0; timerIndex < timerItems.size(); timerIndex++)
		{
			timerItems[timerIndex].currentTime += deltaTime;

			if (timerItems[timerIndex].currentTime > timerItems[timerIndex].endTime)
			{
				timerItems[timerIndex].functionToCall();
				timerItems.erase(timerItems.begin() + timerIndex);
			}
		}
	}

	void SetTimer(double duration, void(*functionToCall)())
	{
		TimerItem timerItem = {};
		timerItem.endTime = duration;
		timerItem.currentTime = 0.0;
		timerItem.functionToCall = functionToCall;

		timerItems.push_back(timerItem);
	}

	std::vector<TimerItem> timerItems;
};

export TimerSystem gTimerSystem;
