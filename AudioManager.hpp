#pragma once
#include <Siv3D.hpp>

// BGM・効果音管理システム
class AudioManager
{
public:
	// BGMタイプ
	enum class BGMType
	{
		Title,      // タイトル画面
		WorldMap,   // マップ画面
		City,       // 内政画面
		Battle,     // 戦闘画面
		Victory,    // 勝利
		Defeat      // 敗北
	};

	// 効果音タイプ
	enum class SEType
	{
		Click,      // クリック
		Select,     // 選択
		Command,    // コマンド
		Attack,     // 攻撃
		Damage,     // ダメージ
		Victory,    // 勝利
		LevelUp,    // レベルアップ
		Gold,       // 金獲得
		Build       // 建設
	};

private:
	// ファイル読み込み用
	HashTable<BGMType, Audio> m_fileBGM;
	HashTable<SEType, Audio> m_fileSE;

	// 生成されたバックアップ用
	Audio m_currentGeneratedBGM;

	BGMType m_currentBGMType = BGMType::Title;
	Audio* m_pCurrentPlaying = nullptr; // 現在再生中のAudioへのポインタ

	double m_bgmVolume = 0.3;
	double m_seVolume = 0.5;
	bool m_bgmEnabled = true;
	bool m_seEnabled = true;

public:
	AudioManager()
	{
		// 起動時にファイルをロードしようとする
		LoadAssets();
	}

	void LoadAssets()
	{
		// ここにあるファイル名を探しに行きます
		if (FileSystem::Exists(U"Audio/BGM/Title.mp3")) m_fileBGM[BGMType::Title] = Audio(U"Audio/BGM/Title.mp3");
		if (FileSystem::Exists(U"Audio/BGM/Map.mp3"))   m_fileBGM[BGMType::WorldMap] = Audio(U"Audio/BGM/Map.mp3");
		if (FileSystem::Exists(U"Audio/BGM/City.mp3"))  m_fileBGM[BGMType::City] = Audio(U"Audio/BGM/City.mp3");
		if (FileSystem::Exists(U"Audio/BGM/Battle.mp3")) m_fileBGM[BGMType::Battle] = Audio(U"Audio/BGM/Battle.mp3");
		if (FileSystem::Exists(U"Audio/BGM/Victory.mp3")) m_fileBGM[BGMType::Victory] = Audio(U"Audio/BGM/Victory.mp3");
		if (FileSystem::Exists(U"Audio/BGM/Defeat.mp3")) m_fileBGM[BGMType::Defeat] = Audio(U"Audio/BGM/Defeat.mp3");

		// ループ設定
		for (auto& [type, audio] : m_fileBGM) audio.setLoop(true);
		// 勝利と敗北はループしない
		if (m_fileBGM.contains(BGMType::Victory)) m_fileBGM[BGMType::Victory].setLoop(false);
		if (m_fileBGM.contains(BGMType::Defeat)) m_fileBGM[BGMType::Defeat].setLoop(false);

		// SE読み込み
		if (FileSystem::Exists(U"Audio/SE/Click.wav")) m_fileSE[SEType::Click] = Audio(U"Audio/SE/Click.wav");
		if (FileSystem::Exists(U"Audio/SE/Select.wav")) m_fileSE[SEType::Select] = Audio(U"Audio/SE/Select.wav");
		if (FileSystem::Exists(U"Audio/SE/Command.wav")) m_fileSE[SEType::Command] = Audio(U"Audio/SE/Command.wav");
		if (FileSystem::Exists(U"Audio/SE/Attack.wav")) m_fileSE[SEType::Attack] = Audio(U"Audio/SE/Attack.wav");
		if (FileSystem::Exists(U"Audio/SE/Damage.wav")) m_fileSE[SEType::Damage] = Audio(U"Audio/SE/Damage.wav");
		if (FileSystem::Exists(U"Audio/SE/Victory.wav")) m_fileSE[SEType::Victory] = Audio(U"Audio/SE/Victory.wav");
	}

	void PlayBGM(BGMType type, double fadeTime = 1.0)
	{
		if (!m_bgmEnabled) return;

		// 同じ曲が既に再生中なら何もしない
		if (m_currentBGMType == type && m_pCurrentPlaying && m_pCurrentPlaying->isPlaying()) return;

		StopBGM(fadeTime); // 前の曲を止める

		m_currentBGMType = type;

		// 1. ファイルがあるか確認
		if (m_fileBGM.contains(type))
		{
			m_pCurrentPlaying = &m_fileBGM[type];
		}
		// 2. なければ自動生成する（バックアップ）
		else
		{
			m_currentGeneratedBGM = GenerateBGM(type);
			m_currentGeneratedBGM.setLoop(true);
			if (type == BGMType::Victory || type == BGMType::Defeat) m_currentGeneratedBGM.setLoop(false);
			m_pCurrentPlaying = &m_currentGeneratedBGM;
		}

		if (m_pCurrentPlaying)
		{
			m_pCurrentPlaying->setVolume(0.0);
			m_pCurrentPlaying->play();
			m_pCurrentPlaying->fadeVolume(m_bgmVolume, SecondsF(fadeTime));
		}
	}

	void PlaySE(SEType type)
	{
		if (!m_seEnabled) return;

		// ファイルがあればそれを使う
		if (m_fileSE.contains(type))
		{
			m_fileSE[type].setVolume(m_seVolume);
			m_fileSE[type].playOneShot();
		}
		else
		{
			// なければ生成して鳴らす
			Audio se = GenerateSE(type);
			se.setVolume(m_seVolume);
			se.play();
		}
	}

	void StopBGM(double fadeTime = 1.0)
	{
		if (m_pCurrentPlaying && m_pCurrentPlaying->isPlaying())
		{
			// フェードアウト
			m_pCurrentPlaying->fadeVolume(0.0, SecondsF(fadeTime));
		}
	}

	// 音量調整などのセッター
	void SetBGMVolume(double volume)
	{
		m_bgmVolume = Clamp(volume, 0.0, 1.0);
		if (m_pCurrentPlaying) m_pCurrentPlaying->setVolume(m_bgmVolume);
	}

	void SetSEVolume(double volume)
	{
		m_seVolume = Clamp(volume, 0.0, 1.0);
	}

private:
	// === バックアップ用の簡易自動生成（ファイルがない時だけ動く） ===
	Audio GenerateBGM(BGMType type)
	{
		const int sampleRate = 44100;
		const double duration = 8.0;
		Array<WaveSample> samples(static_cast<size_t>(duration * sampleRate));
		double pitch = (type == BGMType::Battle) ? 440.0 : 330.0;
		if (type == BGMType::Victory) pitch = 550.0;

		for (size_t i = 0; i < samples.size(); ++i) {
			double t = static_cast<double>(i) / sampleRate;
			// 簡単なサイン波
			samples[i] = WaveSample(static_cast<float>(std::sin(t * pitch * 6.28) * 0.1));
		}
		return Audio(Wave(samples));
	}

	Audio GenerateSE(SEType type)
	{
		const int sampleRate = 44100;
		size_t count = sampleRate / 2;
		Array<WaveSample> samples(count);
		for (size_t i = 0; i < samples.size(); ++i) {
			double t = static_cast<double>(i) / sampleRate;
			double v = std::sin(t * 880.0 * 6.28) * std::exp(-t * 10.0);
			samples[i] = WaveSample(static_cast<float>(v * 0.3));
		}
		return Audio(Wave(samples));
	}
};
