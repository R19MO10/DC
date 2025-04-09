/**
* @file EasyAudio.h
*/
#ifndef EASYAUDIO_H_INCLUDED
#define EASYAUDIO_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../../SoundCue/SoundCue.h"

/**
* 音声関連のコードを格納する名前空間
*/
namespace EasyAudio {

bool Initialize();
void Finalize();
void Update();

// プレイヤーを指定して再生・停止
void Play(int playerId, const char* filename, float volume = 1.0f, bool isLoop = false);
void Stop(int playerId);
void Pause(int playerId);  // 一時停止
void Resume(int playerId); // 再開(一時停止を解除)

// 音量の設定・取得
void SetVolume(int playerId, float volume);
float GetVolume(int playerId);

// ピッチの設定・取得
void SetPitch(int playerId, float pitch);
float GetPitch(int playerId);

// パンの設定・取得
void SetPan(int playerId, float pan);
float GetPan(int playerId);

// 疑似的な3D音源
void SetListenr(const vec3& position, const vec3& right);
void SetPanAndVolumeFromPosition(int playerId, const vec3& position, float volume);

// プレイヤーが再生中かどうか(再生中ならtrue、それ以外はfalse)
bool IsPlaying(int playerId);

// マスターボリュームの設定・取得
void SetMasterVolume(float volume);
float GetMasterVolume();


// 2Dオーディオ再生
void PlaySound2D(
	const char* filename, float volume = 1, float pitch = 1);
template <class SoundCueClass>
inline void PlaySound2D(float volume = 1, float pitch = 1)
{
	// サウンドキュークラスであるか
	if constexpr (std::is_base_of<SoundCue, SoundCueClass>::value) {
		SoundCueClass cue;
		PlaySound2D(
			cue.filename, cue.volume * volume, cue.pitch * pitch);
	}
}

// 3Dオーディオ再生
void PlaySoundAtLocation(
	const char* filename, const vec3& position, 
	float volume = 1, float pitch = 1, 
	const Attenuation* attenuation = nullptr);
template <class SoundCueClass>
inline void PlaySoundAtLocation(
	const vec3& position,
	float volume = 1, float pitch = 1,
	const Attenuation* attenuation = nullptr)
{
	// サウンドキュークラスであるか
	if constexpr (std::is_base_of<SoundCue, SoundCueClass>::value) {
		SoundCueClass cue;
		PlaySoundAtLocation(
			cue.filename, position, 
			cue.volume * volume, cue.pitch * pitch,
			(attenuation ? attenuation : &cue.attenuation)
		);
	}
}

} // namespace EasyAudio

#endif // EASYAUDIO_H_INCLUDED