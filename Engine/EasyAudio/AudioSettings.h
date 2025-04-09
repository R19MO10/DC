/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

/**
* 音声再生プレイヤー番号
*/
namespace AudioPlayer {
	inline constexpr int bgm = 0; // BGMの再生に使うプレイヤー番号
	
} // namespace Player

/**
* BGM設定
*/
namespace BGM {
	inline constexpr char Title[] = "Res/Audio/BGM/ShowTimeLoops.wav"; // タイトル画面のBGM
	inline constexpr char MainGame[] = "Res/Audio/BGM/LostInJive.wav"; // メインゲームのBGM
} // namespace Bgm

/**
* 効果音設定
*/
namespace SE {
	inline constexpr char SpotLight1[] = "Res/Audio/SE/Spotlight01.wav";	// スポットライト
	inline constexpr char SpotLight2[] = "Res/Audio/SE/Spotlight02.wav"; 
	inline constexpr char SpotLight3[] = "Res/Audio/SE/Spotlight03.wav"; 
	inline constexpr char SpotLight4[] = "Res/Audio/SE/Spotlight04.wav"; 

	inline constexpr char WarningSiren01[] =	"Res/Audio/SE/WarningSiren01.wav";	// 警告
	inline constexpr char WarningSiren02[] =	"Res/Audio/SE/WarningSiren02.wav";

	inline constexpr char TimingRingSuccess[] =	"Res/Audio/SE/RingSuccess.wav";		// タイミングリング成功
	inline constexpr char RedBackMove[] =		"Res/Audio/SE/RubbingClothes.wav";	// 幕の移動
	inline constexpr char StartBuzzer[] =		"Res/Audio/SE/Buzzer.wav";			// 開始ブザー
	
	inline constexpr char PlayerMove[] =		"Res/Audio/SE/PlayerMove.wav";		// プレイヤーの歩行
	inline constexpr char PlayerDamage[] =		"Res/Audio/SE/PlayerDamage.wav";	// プレイヤーのダメージ
	inline constexpr char PlayerPowerUp[] =		"Res/Audio/SE/PowerUp.wav";			// プレイヤーのダメージ
	inline constexpr char PlayerPowerUpFinish[] ="Res/Audio/SE/PowerUpFinish.wav";	// プレイヤーのダメージ
	inline constexpr char PlayerArmorGuard[] =	"Res/Audio/SE/ArmorGuard.wav";		// プレイヤーのダメージ
	inline constexpr char SwordCutAir[] =		"Res/Audio/SE/CutAir.wav";			// 空を斬る
	
	inline constexpr char DragonFlapping[] =	"Res/Audio/SE/Flapping.wav";		// ドラゴンの羽のバサバサ
	inline constexpr char DragonMove[] =		"Res/Audio/SE/DragonMove.wav";		// ドラゴンの歩行
	inline constexpr char DragonHit[] =			"Res/Audio/SE/DragonHit.wav";		// ドラゴンのダメージ音
	inline constexpr char DragonHit_Normal[] =		"Res/Audio/SE/DragonHit_Normal.wav";	// ノーマルダメージ音
	inline constexpr char DragonHit_Critical[] =	"Res/Audio/SE/DragonHit_Critical.wav";	// クリティカルダメージ音
	inline constexpr char DragonClash[] =		"Res/Audio/SE/DragonClash.wav";		// ドラゴンのクラッシュ
	inline constexpr char DragonFalls[] =		"Res/Audio/SE/DragonFalls.wav";		// ドラゴンの倒れた時

	inline constexpr char StoneBreak[] =		"Res/Audio/SE/StoneBreak.wav";	// 石の破壊
	inline constexpr char Fireworks[] =			"Res/Audio/SE/Fireworks.wav";	// 複数の花火

	inline constexpr char GameClear[] =			"Res/Audio/SE/Clear.wav";	// ゲームクリア
} // namespace SE

#endif // AUDIOSETTINGS_H_INCLUDED