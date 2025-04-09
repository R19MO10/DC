/**
* @file EasyAudio.h
*/
#ifndef EASYAUDIO_H_INCLUDED
#define EASYAUDIO_H_INCLUDED
#include "../../Math/MyMath.h"
#include "../../SoundCue/SoundCue.h"

/**
* �����֘A�̃R�[�h���i�[���閼�O���
*/
namespace EasyAudio {

bool Initialize();
void Finalize();
void Update();

// �v���C���[���w�肵�čĐ��E��~
void Play(int playerId, const char* filename, float volume = 1.0f, bool isLoop = false);
void Stop(int playerId);
void Pause(int playerId);  // �ꎞ��~
void Resume(int playerId); // �ĊJ(�ꎞ��~������)

// ���ʂ̐ݒ�E�擾
void SetVolume(int playerId, float volume);
float GetVolume(int playerId);

// �s�b�`�̐ݒ�E�擾
void SetPitch(int playerId, float pitch);
float GetPitch(int playerId);

// �p���̐ݒ�E�擾
void SetPan(int playerId, float pan);
float GetPan(int playerId);

// �^���I��3D����
void SetListenr(const vec3& position, const vec3& right);
void SetPanAndVolumeFromPosition(int playerId, const vec3& position, float volume);

// �v���C���[���Đ������ǂ���(�Đ����Ȃ�true�A����ȊO��false)
bool IsPlaying(int playerId);

// �}�X�^�[�{�����[���̐ݒ�E�擾
void SetMasterVolume(float volume);
float GetMasterVolume();


// 2D�I�[�f�B�I�Đ�
void PlaySound2D(
	const char* filename, float volume = 1, float pitch = 1);
template <class SoundCueClass>
inline void PlaySound2D(float volume = 1, float pitch = 1)
{
	// �T�E���h�L���[�N���X�ł��邩
	if constexpr (std::is_base_of<SoundCue, SoundCueClass>::value) {
		SoundCueClass cue;
		PlaySound2D(
			cue.filename, cue.volume * volume, cue.pitch * pitch);
	}
}

// 3D�I�[�f�B�I�Đ�
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
	// �T�E���h�L���[�N���X�ł��邩
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