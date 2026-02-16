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

	// コマンド実行（戻り値: 実行できたら true）
	bool ExecuteCommand()
	{
		if (!CanExecuteCommand())
		{
			return false;
		}

		commandsUsed++;
		return true;
	}

	// 既存コード互換用: コマンドを消費するラッパー
	// FacilityScene 等から `UseCommand()` が呼ばれているため、互換性のために追加。
	// 戻り値をチェックしていない呼び出し箇所があるため void として実装。
	void UseCommand()
	{
		// 成功/失敗は呼び出し元で扱われていないケースが多いので無視する。
		// 必要なら将来 bool を返すオーバーロードや別名を追加すること。
		ExecuteCommand();
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
