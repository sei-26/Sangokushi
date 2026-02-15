#pragma once
#include <Siv3D.hpp>

// ターン管理システム
class TurnManager
{
public:
	static constexpr int MAX_COMMANDS_PER_TURN = 4;  // 1ターン4コマンド

	int commandsUsed = 0;  // 今月使ったコマンド数

	// コマンドを実行できるか
	bool CanExecuteCommand() const
	{
		return commandsUsed < MAX_COMMANDS_PER_TURN;
	}

	// コマンド実行
	bool ExecuteCommand()
	{
		if (!CanExecuteCommand())
		{
			return false;
		}

		commandsUsed++;
		return true;
	}

	// 残りコマンド数
	int GetRemainingCommands() const
	{
		return MAX_COMMANDS_PER_TURN - commandsUsed;
	}

	// 月を進める（コマンド数をリセット）
	void AdvanceMonth()
	{
		commandsUsed = 0;
	}

	// リセット
	void Reset()
	{
		commandsUsed = 0;
	}
};
