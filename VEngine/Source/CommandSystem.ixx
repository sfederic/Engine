module;
#include "Commands/ICommand.h"
import <vector>;
export module CommandSystem;

//currently just for editor undo/redo, but I guess the pattern is generic enough that it 
//could be used in other spots.
export class CommandSystem
{
public:
	void AddCommand(ICommand* command)
	{
		command->Execute();
		commands.push_back(command);
		currentCommandIndex = commands.size() - 1;
	}

	void Undo()
	{
		if (currentCommandIndex <= 0)
		{
			if (commands.size() > 0)
			{
				commands.front()->Undo();
			}

			return; //Reached start of list
		}

		commands[currentCommandIndex]->Undo();
		currentCommandIndex--;
	}

	void Redo()
	{
		if (currentCommandIndex >= commands.size() - 1)
		{
			commands.back()->Execute();
			return; //Reached end of list
		}

		currentCommandIndex++;
		commands[currentCommandIndex]->Execute();
	}

private:
	std::vector<ICommand*> commands;
	uint32_t currentCommandIndex = 0;
};

export CommandSystem gCommandSystem;
