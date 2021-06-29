module;
#include <Windows.h>
#include <stdint.h>
import <vector>;
export module Input;

#ifdef WIN32
export enum class Keys
{
	None = 0,

	Space = VK_SPACE,
	Enter = VK_RETURN,
	Tab = VK_TAB,
	Tilde = VK_OEM_3,
	Ctrl = VK_CONTROL,
	Delete = VK_DELETE,
	BackSpace = VK_BACK,
	Alt = VK_LMENU,

	//TODO: there's all sorts of scancode shit win Win32 and keyboards
	//that won't make this work internationally.
	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',
	F = 'F',
	Z = 'Z',
	P = 'P',
	E = 'E',
	R = 'R',
	O = 'O',
	G = 'G',
	V = 'V',
	B = 'B',

	_1 = '1',
	_2 = '2',
	_3 = '3',
	_4 = '4',
	_5 = '5',
	_6 = '6',
	_7 = '7',
	_8 = '8',
	_9 = '9',
	_0 = '0',

	F1 = VK_F1,
	F2 = VK_F2,
	F3 = VK_F3,
	F11 = VK_F11,

	Up = VK_UP,
	Down = VK_DOWN,
	Right = VK_RIGHT,
	Left = VK_LEFT,

	LeftMouse = VK_LBUTTON,
	RightMouse = VK_RBUTTON,
	MiddleMouse = VK_MBUTTON
};
#endif

export class InputSystem
{
public:
	bool GetAnyKeyDown()
	{
		if (keyDown)
		{
			return true;
		}

		return false;
	}

	bool GetAnyKeyUp()
	{
		if (keyUp)
		{
			return true;
		}

		return false;
	}

	void StoreKeyDownInput(int key)
	{
		currentDownKey = key;
		currentDownKeys.push_back(key);
		keyDown = true;
	}

	void StoreKeyUpInput(int key)
	{
		//Clear any down keys that match the current 'up' value
		ClearDownKeys(key);

		currentUpKey = key;
		currentUpKeys.push_back(key);
		keyUp = true;
	}

	void StoreMouseLeftDownInput()
	{
		leftMouseDown = true;
		leftMouseUp = false;
	}

	void StoreMouseLeftUpInput()
	{
		leftMouseUp = true;
		leftMouseDown = false;
	}

	void StoreMouseRightDownInput()
	{
		rightMouseDown = true;
		rightMouseUp = false;
	}

	void StoreMouseRightUpInput()
	{
		rightMouseUp = true;
		rightMouseDown = false;
	}

	void StoreMouseMiddleDownInput()
	{
		middleMouseDown = true;
		middleMouseUp = false;
	}

	void StoreMouseMiddleUpInput()
	{
		middleMouseUp = true;
		middleMouseDown = false;
	}

	bool GetKeyDownState(Keys key)
	{
		for (int i = 0; i < currentDownKeys.size(); i++)
		{
			if ((int)key == currentDownKeys[i])
			{
				return true;
			}
		}

		return false;
	}

	bool GetKeyDownState(Keys key, Keys modifier)
	{
		if (GetAsyncKey(modifier))
		{
			return GetKeyDownState(key);
		}

		return false;
	}

	bool GetMouseLeftDownState()
	{
		if (leftMouseDown)
		{
			return true;
		}

		return false;
	}

	bool GetMouseLeftUpState()
	{
		if (leftMouseUp)
		{
			return true;
		}

		return false;
	}

	bool GetMouseRightDownState()
	{
		if (rightMouseDown)
		{
			return true;
		}

		return false;
	}

	bool GetMouseRightUpState()
	{
		if (rightMouseUp)
		{
			return true;
		}

		return false;
	}

	bool GetMouseMiddleUpState()
	{
		if (middleMouseUp)
		{
			return true;
		}

		return false;
	}

	bool GetMouseMiddleDownState()
	{
		if (middleMouseDown)
		{
			return true;
		}

		return false;
	}

	bool GetAsyncKey(Keys key)
	{
		if (GetAsyncKeyState((int)key))
		{
			return true;
		}

		return false;
	}

	bool GetKeyUpState(Keys key)
	{
		for (int i = 0; i < currentUpKeys.size(); i++)
		{
			if ((int)key == currentUpKeys[i])
			{
				return true;
			}
		}

		return false;
	}

	bool GetKeyUpState(Keys key, Keys modifier)
	{
		if (GetAsyncKey(modifier))
		{
			return GetKeyUpState(key);
		}

		return false;
	}

	void InputReset()
	{
		rightMouseDown = false;
		rightMouseUp = false;
		leftMouseDown = false;
		leftMouseUp = false;
		middleMouseUp = false;
		middleMouseDown = false;
		keyDown = false;
		keyUp = false;

		currentDownKey = (int)Keys::None;
		currentUpKey = (int)Keys::None;

		bMouseWheelDown = false;
		bMouseWheelUp = false;

		currentDownKeys.clear();
		currentUpKeys.clear();
	}

	void StoreMouseWheelUp()
	{
		bMouseWheelUp = true;
	}

	void StoreMouseWheelDown()
	{
		bMouseWheelDown = true;
	}

	bool GetMouseWheelUp()
	{
		return bMouseWheelUp;
	}

	bool GetMouseWheelDown()
	{
		return bMouseWheelDown;
	}

	void ClearDownKeys(int keyToClear)
	{
		for (int i = 0; i < currentDownKeys.size(); i++)
		{
			if (keyToClear == currentDownKeys[i])
			{
				currentDownKeys.erase(currentDownKeys.begin() + i);
				return;
			}
		}
	}

	std::vector<int> currentDownKeys;
	std::vector<int> currentUpKeys;

	int currentDownKey;
	int currentUpKey;

	bool leftMouseUp;
	bool leftMouseDown;
	bool rightMouseUp;
	bool rightMouseDown;
	bool middleMouseDown;
	bool middleMouseUp;
	bool keyUp;
	bool keyDown;
	bool bMouseWheelUp;
	bool bMouseWheelDown;
};

export InputSystem gInputSystem;
