//-----------------------------------------------------------------------------
#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

////-----------------------------------------------------------------------------
//// Sound System Class
////-----------------------------------------------------------------------------
//class SoundSystem
//{
//public:
//	SoundSystem( float scale = 1.0f );
//	virtual ~SoundSystem();
//
//	void UpdateListener( D3DXVECTOR3 forward, D3DXVECTOR3 position, D3DXVECTOR3 velocity );
//
//	void GarbageCollection();
//
//	void SetVolume( long volume );
//
//	IDirectMusicLoader8 *GetLoader();
//	IDirectMusicPerformance8 *GetPerformance();
//
//private:
//	float m_scale; // Unit scale in meters/unit.
//	IDirectMusicLoader8 *m_loader; // DirectMusic loader.
//	IDirectMusicPerformance8 *m_performance; // DirectMusic performance.
//	IDirectSound3DListener8 *m_listener; // DirectSound 3D listener.
//};

//-----------------------------------------------------------------------------
// Sound Class
//-----------------------------------------------------------------------------
class Sound
{
public:
	Sound(char *filename);
	virtual ~Sound();

	void Play(bool loop = false, unsigned long flags = 0);// flag DMUS_SEGF_AUTOTRANSITION

	//IDirectMusicSegment8 *GetSegment();
	ALuint *GetSegment();
private:
	//IDirectMusicSegment8 *m_segment; // DirectMusic segment for the sound.

	ALuint buffer;
	ALuint source;
	ALenum error;
	ALint status;

};


////-----------------------------------------------------------------------------
//// Audio Path 3D Class
////-----------------------------------------------------------------------------
//class AudioPath3D
//{
//public:
//	AudioPath3D();
//	virtual ~AudioPath3D();
//
//	void SetPosition( D3DXVECTOR3 position );
//	void SetVelocity( D3DXVECTOR3 velocity );
//	void SetMode( unsigned long mode );
//
//	void Play( IDirectMusicSegment8 *segment, bool loop = false, unsigned long flags = DMUS_SEGF_SECONDARY );
//
//private:
//	IDirectMusicAudioPath8 *m_audioPath; // DirectMusic audio path for 3D playback.
//	IDirectSound3DBuffer8 *m_soundBuffer; // Pointer to the audiopath's sound buffer.
//};

#endif