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
		Click,      // ボタンクリック
		Select,     // 選択
		Command,    // コマンド実行
		Attack,     // 攻撃
		Damage,     // ダメージ
		Victory,    // 勝利
		LevelUp,    // レベルアップ
		Gold,       // 金獲得
		Build       // 建設完了
	};

private:
	Audio m_currentBGM;
	BGMType m_currentBGMType = BGMType::Title;
	double m_bgmVolume = 0.3;
	double m_seVolume = 0.5;
	bool m_bgmEnabled = true;
	bool m_seEnabled = true;

public:
	// BGM再生
	void PlayBGM(BGMType type, double fadeTime = 1.0)
	{
		if (!m_bgmEnabled) return;
		if (type == m_currentBGMType && m_currentBGM.isPlaying()) return;

		// 前のBGMをフェードアウト
		if (m_currentBGM.isPlaying())
		{
			m_currentBGM.fadeVolume(0.0, SecondsF(fadeTime));
		}

		// 新しいBGMを生成（シンプルな音）
		m_currentBGM = GenerateBGM(type);
		m_currentBGM.setLoop(true);
		m_currentBGM.setVolume(0.0);
		m_currentBGM.play();
		m_currentBGM.fadeVolume(m_bgmVolume, SecondsF(fadeTime));

		m_currentBGMType = type;
	}

	// 効果音再生
	void PlaySE(SEType type)
	{
		if (!m_seEnabled) return;

		Audio se = GenerateSE(type);
		se.setVolume(m_seVolume);
		se.play();
	}

	// BGM停止
	void StopBGM(double fadeTime = 1.0)
	{
		if (m_currentBGM.isPlaying())
		{
			m_currentBGM.fadeVolume(0.0, SecondsF(fadeTime));
		}
	}

	// 音量設定
	void SetBGMVolume(double volume)
	{
		m_bgmVolume = Clamp(volume, 0.0, 1.0);
		if (m_currentBGM.isPlaying())
		{
			m_currentBGM.setVolume(m_bgmVolume);
		}
	}

	void SetSEVolume(double volume)
	{
		m_seVolume = Clamp(volume, 0.0, 1.0);
	}

	// ON/OFF
	void SetBGMEnabled(bool enabled)
	{
		m_bgmEnabled = enabled;
		if (!enabled && m_currentBGM.isPlaying())
		{
			m_currentBGM.pause();
		}
		else if (enabled && !m_currentBGM.isPlaying())
		{
			m_currentBGM.play();
		}
	}

	void SetSEEnabled(bool enabled)
	{
		m_seEnabled = enabled;
	}

private:
	// BGM生成（三国志風の壮大な音楽）
	Audio GenerateBGM(BGMType type)
	{
		const double duration = 12.0;
		const int sampleRate = 44100;
		const size_t sampleCount = static_cast<size_t>(duration * sampleRate);

		Array<WaveSample> samples(sampleCount);

		// 中国風ペンタトニック（五音音階）+ 壮大な和音
		Array<int> melody;
		Array<int> bass;
		double bpm = 70;

		switch (type)
		{
		case BGMType::WorldMap:
			// 壮大な行進曲風
			melody = { 60, 60, 64, 67, 69, 67, 64, 60, 62, 64, 62, 60, 57, 60, 64, 67 };
			bass = { 36, 36, 41, 43, 45, 43, 41, 36, 38, 41, 38, 36, 33, 36, 41, 43 };
			bpm = 75;
			break;
		case BGMType::City:
			// 穏やかな宮廷音楽
			melody = { 64, 67, 69, 72, 69, 67, 64, 62, 64, 67, 64, 62, 60, 62, 64, 60 };
			bass = { 40, 43, 45, 48, 45, 43, 40, 38, 40, 43, 40, 38, 36, 38, 40, 36 };
			bpm = 65;
			break;
		case BGMType::Battle:
			// 激しい戦闘曲
			melody = { 62, 65, 67, 70, 72, 70, 67, 65, 62, 60, 62, 65, 67, 65, 62, 60 };
			bass = { 38, 41, 43, 46, 48, 46, 43, 41, 38, 36, 38, 41, 43, 41, 38, 36 };
			bpm = 100;
			break;
		case BGMType::Victory:
			// 勝利ファンファーレ
			melody = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64, 60, 64, 67, 72 };
			bass = { 36, 40, 43, 48, 52, 55, 60, 55, 52, 48, 43, 40, 36, 40, 43, 48 };
			bpm = 110;
			break;
		case BGMType::Defeat:
			// 敗北
			melody = { 60, 58, 56, 55, 53, 51, 50, 48, 50, 51, 53, 55, 56, 58, 60, 58 };
			bass = { 36, 34, 32, 31, 29, 27, 26, 24, 26, 27, 29, 31, 32, 34, 36, 34 };
			bpm = 60;
			break;
		default:
			melody = { 60, 64, 67, 69, 72, 69, 67, 64 };
			bass = { 36, 40, 43, 45, 48, 45, 43, 40 };
			bpm = 70;
			break;
		}

		const double beatDuration = 60.0 / bpm;
		const double noteDuration = beatDuration * 16.0 / melody.size();

		for (size_t i = 0; i < sampleCount; ++i)
		{
			double time = static_cast<double>(i) / sampleRate;
			int noteIndex = static_cast<int>(time / noteDuration) % melody.size();

			// メロディ
			double melodyFreq = 440.0 * std::pow(2.0, (melody[noteIndex] - 69) / 12.0);
			// ベース
			double bassFreq = 440.0 * std::pow(2.0, (bass[noteIndex] - 69) / 12.0);

			// 複数の倍音で壮大な音色
			double value = 0.0;
			// メロディライン
			value += std::sin(Math::TwoPi * melodyFreq * time) * 0.25;
			value += std::sin(Math::TwoPi * melodyFreq * 2 * time) * 0.08;
			value += std::sin(Math::TwoPi * melodyFreq * 3 * time) * 0.04;
			// ベース
			value += std::sin(Math::TwoPi * bassFreq * time) * 0.15;
			value += std::sin(Math::TwoPi * bassFreq * 2 * time) * 0.05;

			// エンベロープ
			double t = std::fmod(time, noteDuration) / noteDuration;
			double attack = std::min(t * 30, 1.0);
			double release = std::min((1 - t) * 3, 1.0);
			double envelope = attack * release;

			value *= envelope * 0.2;

			// 全体のフェード
			double fadeIn = std::min(time / 2.0, 1.0);
			double fadeOut = std::min((duration - time) / 2.0, 1.0);
			value *= fadeIn * fadeOut;

			samples[i] = WaveSample(static_cast<float>(value));
		}

		return Audio(Wave(samples));
	}

	// 効果音生成
	Audio GenerateSE(SEType type)
	{
		const int sampleRate = 44100;
		size_t sampleCount = sampleRate / 10;
		double freq = 800.0;
		double decay = 10.0;

		switch (type)
		{
		case SEType::Click:
			freq = 1200.0;
			decay = 15.0;
			sampleCount = sampleRate / 20;
			break;
		case SEType::Select:
			freq = 1500.0;
			decay = 12.0;
			break;
		case SEType::Command:
			freq = 900.0;
			decay = 8.0;
			break;
		case SEType::Gold:
			freq = 1800.0;
			decay = 10.0;
			break;
		default:
			break;
		}

		Array<WaveSample> samples(sampleCount);

		for (size_t i = 0; i < sampleCount; ++i)
		{
			double t = static_cast<double>(i) / sampleRate;
			double value = std::sin(Math::TwoPi * freq * t) * 0.4;
			value += std::sin(Math::TwoPi * freq * 1.5 * t) * 0.15;
			value *= std::exp(-t * decay);
			samples[i] = WaveSample(static_cast<float>(value));
		}

		return Audio(Wave(samples));
	}
};
