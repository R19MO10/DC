/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

/**
* �����Đ��v���C���[�ԍ�
*/
namespace AudioPlayer {
	inline constexpr int bgm = 0; // BGM�̍Đ��Ɏg���v���C���[�ԍ�
	
} // namespace Player

/**
* BGM�ݒ�
*/
namespace BGM {
	inline constexpr char Title[] = "Res/Audio/BGM/ShowTimeLoops.wav"; // �^�C�g����ʂ�BGM
	inline constexpr char MainGame[] = "Res/Audio/BGM/LostInJive.wav"; // ���C���Q�[����BGM
} // namespace Bgm

/**
* ���ʉ��ݒ�
*/
namespace SE {
	inline constexpr char SpotLight1[] = "Res/Audio/SE/Spotlight01.wav";	// �X�|�b�g���C�g
	inline constexpr char SpotLight2[] = "Res/Audio/SE/Spotlight02.wav"; 
	inline constexpr char SpotLight3[] = "Res/Audio/SE/Spotlight03.wav"; 
	inline constexpr char SpotLight4[] = "Res/Audio/SE/Spotlight04.wav"; 

	inline constexpr char WarningSiren01[] =	"Res/Audio/SE/WarningSiren01.wav";	// �x��
	inline constexpr char WarningSiren02[] =	"Res/Audio/SE/WarningSiren02.wav";

	inline constexpr char TimingRingSuccess[] =	"Res/Audio/SE/RingSuccess.wav";		// �^�C�~���O�����O����
	inline constexpr char RedBackMove[] =		"Res/Audio/SE/RubbingClothes.wav";	// ���̈ړ�
	inline constexpr char StartBuzzer[] =		"Res/Audio/SE/Buzzer.wav";			// �J�n�u�U�[
	
	inline constexpr char PlayerMove[] =		"Res/Audio/SE/PlayerMove.wav";		// �v���C���[�̕��s
	inline constexpr char PlayerDamage[] =		"Res/Audio/SE/PlayerDamage.wav";	// �v���C���[�̃_���[�W
	inline constexpr char PlayerPowerUp[] =		"Res/Audio/SE/PowerUp.wav";			// �v���C���[�̃_���[�W
	inline constexpr char PlayerPowerUpFinish[] ="Res/Audio/SE/PowerUpFinish.wav";	// �v���C���[�̃_���[�W
	inline constexpr char PlayerArmorGuard[] =	"Res/Audio/SE/ArmorGuard.wav";		// �v���C���[�̃_���[�W
	inline constexpr char SwordCutAir[] =		"Res/Audio/SE/CutAir.wav";			// ����a��
	
	inline constexpr char DragonFlapping[] =	"Res/Audio/SE/Flapping.wav";		// �h���S���̉H�̃o�T�o�T
	inline constexpr char DragonMove[] =		"Res/Audio/SE/DragonMove.wav";		// �h���S���̕��s
	inline constexpr char DragonHit[] =			"Res/Audio/SE/DragonHit.wav";		// �h���S���̃_���[�W��
	inline constexpr char DragonHit_Normal[] =		"Res/Audio/SE/DragonHit_Normal.wav";	// �m�[�}���_���[�W��
	inline constexpr char DragonHit_Critical[] =	"Res/Audio/SE/DragonHit_Critical.wav";	// �N���e�B�J���_���[�W��
	inline constexpr char DragonClash[] =		"Res/Audio/SE/DragonClash.wav";		// �h���S���̃N���b�V��
	inline constexpr char DragonFalls[] =		"Res/Audio/SE/DragonFalls.wav";		// �h���S���̓|�ꂽ��

	inline constexpr char StoneBreak[] =		"Res/Audio/SE/StoneBreak.wav";	// �΂̔j��
	inline constexpr char Fireworks[] =			"Res/Audio/SE/Fireworks.wav";	// �����̉ԉ�

	inline constexpr char GameClear[] =			"Res/Audio/SE/Clear.wav";	// �Q�[���N���A
} // namespace SE

#endif // AUDIOSETTINGS_H_INCLUDED