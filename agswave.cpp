////*

#ifdef WIN32
#define WINDOWS_VERSION
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(disable : 4244)
#endif

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <iostream>
#include <SDL_mixer.h>
#include <sstream>
#include <cstring>
#include <math.h>
#include "audio.h"


#if defined(PSP_VERSION)
#include <pspsdk.h>
#include <pspmath.h>
#include <pspdisplay.h>
#define sin(x) vfpu_sinf(x)
#endif

#include "plugin/agsplugin.h"

#if defined(BUILTIN_PLUGINS)
namespace agswave {
#endif

#if defined(__GNUC__)
inline unsigned long _blender_alpha16_bgr(unsigned long y) __attribute__((always_inline));
inline void calc_x_n(unsigned long bla) __attribute__((always_inline));
#endif


float nmax(float a, float b)
{
	return (((a) > (b)) ? (a) : (b));
}
float nmin(float a, float b)
{
	return (((a) < (b)) ? (a) : (b));
}         


const unsigned int Magic = 0xACAB0000;
const unsigned int Version = 1;
const unsigned int SaveMagic = Magic + Version;
const float PI = 3.14159265f;

int screen_width = 640;
int screen_height = 360;
int screen_color_depth = 32;

AGSCharacter*playerCharacter=NULL;

typedef int (*SCAPI_CHARACTER_GETX)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_GETY)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_ID) (AGSCharacter *ch);

SCAPI_CHARACTER_GETX    Character_GetX = NULL;
SCAPI_CHARACTER_GETY    Character_GetY = NULL;
SCAPI_CHARACTER_ID  Character_ID= NULL;

IAGSEngine* engine;



// Imported script functions


Mix_Music *musiceffect[80];//1 music channel


struct getMus
{
	char musicPath[1024];
};
getMus MusicLoads[80];



//WAVE SOUNDS FILES
struct Soundeffect
{
	Mix_Chunk*chunk;
	int repeat;
	int volume;
	int playing;
	int allow;
	int channel;
	int filter;
};
Soundeffect SFX[500];

struct Aud
{
	int NumOfChannels;
	bool Initialized;
	bool Disabled;
	int FilterFrequency;
	int SoundValue;
};

Aud GeneralAudio;


struct Mus
{
	int ID;
	int FadeTime;
	float FadeRate;
	float FadeVolume;
	int Channel;
	bool Switch;
	bool HaltedZero;
	bool HaltedOne;
};
Mus MFXStream;

int currentMusic=-1;
int currentMusicRepeat=-1;
int currentMusicFadein=0;

double xv[3];
double yv[3];
double xvOGG[3];
double yvOGG[3];



struct Particle{
int x;
int y;
int transp;
int life;
bool active;
int dx;
int dy;
int mlay;
int timlay;
int movedport;
int translay;
int translayHold;
int width;
int height;
int fx;
int fy;
bool doingcircle;
float angle;
float radius;
int doingCircleChance;
float angleLay;
int frame;
float anglespeed;
};

Particle particles[110];
Particle particlesF[10];
Particle particles2[12];
int WForceX[400];
int WForceY[400];

int raysizeF=4;
int dsizeF=0;
int raysize=100;
int dsize=0;
int raysize2=12;
int dsize2=0;
int creationdelayf=0;
int ww;
int hh;
int proom;
int prevroom;


struct RainParticle
{
  int x;
  int y;
  int fx;
  int fy;
  int life;
  int trans;
  bool active;
  int translay;
  int transhold;
};

RainParticle RainParticles[400];
RainParticle RainParticlesFore[400];
RainParticle RainParticlesBack[800];


bool OGG_Filter=false;
#include "stb_vorbis.c"
#define SDL_AUDIO_ALLOW_CHANGES SDL_AUDIO_ALLOW_ANY_CHANGE

struct MusicStream
{
	int volume;
	const char* Filename;
	int repeat;
	stb_vorbis*Vorbis;
	bool fix_click;
};

SDL_AudioSpec spec[2];
MusicStream globalStream[2];
SDL_AudioDeviceID getDevice[2];
bool AudioEnabled=false;
float ix, iy, ua;
float b_time[5];
float d_time;

#define texWidth 240
#define texHeight 240
#define screenWidth 640
#define screenHeight 360

// Y-coordinate first because we use horizontal scanlines
Uint32 texture[texHeight][texWidth];
int distanceTable[screenHeight][screenWidth];
int angleTable[screenHeight][screenWidth];

bool generateonce=false;
struct DustParticle{
int x;
int y;
int transp;
int life;
bool active;
int dx;
int dy;
int mlay;
int timlay;
int movedport;
int translay;
int translayHold;
};

DustParticle dusts[200];
int waitBy=6;
int raysizeDust=200;
int dsizeDust=0;
int creationdelay=0;
int Walkbehind[20];
char*GameDatavalue[40000];
char*Token[10000];
int TokenUnUsed[10000];
int usedTokens=0;

int dY[30];
int tDy[30];
int direction[30];




void StartingValues()
{
	GeneralAudio.NumOfChannels=0;
	GeneralAudio.Initialized=false;
	GeneralAudio.Disabled=false;
	GeneralAudio.FilterFrequency=10;
	GeneralAudio.SoundValue=0;	
	MFXStream.ID=0;
	MFXStream.Channel=-1;
	MFXStream.Switch=false;
	MFXStream.FadeTime=0;
	MFXStream.FadeRate=0.0;
	MFXStream.FadeVolume=0.0;
	MFXStream.HaltedZero=false;
	MFXStream.HaltedOne=false;

	int j=0;
	while (j <2)
	{		
		globalStream[j].Filename=NULL;
		globalStream[j].repeat=0;
		globalStream[j].volume=0;
		globalStream[j].Vorbis=NULL;
		globalStream[j].fix_click=false;
	  j++;
   }
}



void getLPCoe(int samplerate, double cutoff, double ax[], double by[])
{
	//CALCULATES LOWPASS COEFFICIENTS
   double PI      = 3.1415926535897932385;
    double sqrt2 = 1.4142135623730950488;
    double QcRaw  = (2 * PI * cutoff) / samplerate; // Find cutoff frequency in [0..PI]
    double QcWarp = tan(QcRaw); // Warp cutoff frequency

    double gain = 1 / (1+sqrt2/QcWarp + 2/(QcWarp*QcWarp));
    by[2] = (1 - sqrt2/QcWarp + 2/(QcWarp*QcWarp)) * gain;
    by[1] = (2 - 2 * 2/(QcWarp*QcWarp)) * gain;
    by[0] = 1;//
    ax[0] = 1 * gain;
    ax[1] = 2 * gain;
    ax[2] = 1 * gain;
}


void LPEffect(int chan, void *stream, int len, void *udata)
{
	//APPLIES THE SAME LOW PASS TO SOUND EFFECTS
	int CUTOFF=GeneralAudio.FilterFrequency;
	float SAMPLE_RATE=48000.0;


	short* samples = (short*)stream;
	short* samplesINPUT = (short*)stream;


	double RC = 1.0/(CUTOFF*16*3.14);
    double dt = 1.0/(SAMPLE_RATE);
    double alpha = dt/(RC+dt);



    double ax[3];
	double by[3];
	getLPCoe(SAMPLE_RATE, CUTOFF*12, ax, by);

	for(int i = 0; i < (len/2); i++)//len/2
    {
	   xv[2] = xv[1];
	   xv[1] = xv[0];
       xv[0] = samples[i];
       yv[2] = yv[1];
	   yv[1] = yv[0];

       yv[0] =   (ax[0] * xv[0] + ax[1] * xv[1] + ax[2] * xv[2]
                    - by[1] * yv[0]
                    - by[2] * yv[1]);

       samples[i] = yv[0];
    }
}




void OGGAudioCallbackZero(void* userData, Uint8* stream, int len)
{
	//MUSIC 1 CHANNEL [CALLED ZERO] for PLAYING AN OGG
	int getMID=0;
	if (!AudioEnabled || globalStream[getMID].Vorbis==NULL)
	{
		//SDL_memclear
		return;
	}

    SDL_memset(stream, 0, len);

	short* samples = (short*)stream;
	short* samplesINPUT = (short*)stream;
	//ADD REPEAT



    int pf=stb_vorbis_get_samples_short_interleaved(globalStream[getMID].Vorbis, 2, samples, len/sizeof(short));

	for(int i = 0; i < (len/2); i++)
    {
		samples[i] = (samplesINPUT[i]*globalStream[getMID].volume)/100;
	   int nf=0;
	   if (globalStream[getMID].fix_click) nf=4095;
	   if (pf<=nf && globalStream[getMID].repeat==1)
	   {
		   stb_vorbis_seek(globalStream[getMID].Vorbis,0);
		   pf=stb_vorbis_get_samples_short_interleaved(globalStream[getMID].Vorbis, 2, samples, len/sizeof(short));
		   i=0;
		   continue;
	   }
    }



	if (OGG_Filter&& globalStream[getMID].volume>1)
	{
	int CUTOFF=50;//GeneralAudio.FilterFrequency;
	float SAMPLE_RATE=48000.0;


	samplesINPUT = samples;


	double RC = 1.0/(CUTOFF*16*3.14);
    double dt = 1.0/(SAMPLE_RATE);
    double alpha = dt/(RC+dt);




    double ax[3];
	double by[3];
	getLPCoe(SAMPLE_RATE, CUTOFF*12, ax, by);

	for(int i = 0; i < (len/2); i++)//len/2
    {
	   xvOGG[2] = xvOGG[1];
	   xvOGG[1] = xvOGG[0];
       xvOGG[0] = samples[i];
       yvOGG[2] = yvOGG[1];
	   yvOGG[1] = yvOGG[0];

       yvOGG[0] =   (ax[0] * xvOGG[0] + ax[1] * xvOGG[1] + ax[2] * xvOGG[2]
                    - by[1] * yvOGG[0]
                    - by[2] * yvOGG[1]);

       samples[i] = yvOGG[0];
    }
	}



}

void OGGAudioCallbackOne(void* userData, Uint8* stream, int len)
{
	//MUSIC 2 CHANNEL [CALLED ONE for PLAYING AN OGG, STREAMING AN OGG VORBIS.
	int getMID=1;
	if (!AudioEnabled || globalStream[getMID].Vorbis==NULL)
	{
		//SDL_memclear
		return;
	}

    SDL_memset(stream, 0, len);

	short* samples = (short*)stream;
	short* samplesINPUT = (short*)stream;
	//ADD REPEAT



    int pf=stb_vorbis_get_samples_short_interleaved(globalStream[getMID].Vorbis, 2, samples, len/sizeof(short));

	for(int i = 0; i < (len/2); i++)
    {
	   samples[i] = (samplesINPUT[i]*globalStream[getMID].volume)/100;
	   int nf=0;
	   if (globalStream[getMID].fix_click) nf=4095;
	   if (pf<=nf && globalStream[getMID].repeat==1)
	   {
		   stb_vorbis_seek(globalStream[getMID].Vorbis,0);
		   pf=stb_vorbis_get_samples_short_interleaved(globalStream[getMID].Vorbis, 2, samples, len/sizeof(short));
		   i=0;
		   continue;
	   }
    }



	if (OGG_Filter && globalStream[getMID].volume>1)
	{
	int CUTOFF=50;//GeneralAudio.FilterFrequency;
	float SAMPLE_RATE=48000.0;


	samplesINPUT = samples;


	double RC = 1.0/(CUTOFF*16*3.14);
    double dt = 1.0/(SAMPLE_RATE);
    double alpha = dt/(RC+dt);




    double ax[3];
	double by[3];
	getLPCoe(SAMPLE_RATE, CUTOFF*12, ax, by);

	for(int i = 0; i < (len/2); i++)//len/2
    {
	   xvOGG[2] = xvOGG[1];
	   xvOGG[1] = xvOGG[0];
       xvOGG[0] = samples[i];
       yvOGG[2] = yvOGG[1];
	   yvOGG[1] = yvOGG[0];

       yvOGG[0] =   (ax[0] * xvOGG[0] + ax[1] * xvOGG[1] + ax[2] * xvOGG[2]
                    - by[1] * yvOGG[0]
                    - by[2] * yvOGG[1]);

       samples[i] = yvOGG[0];
    }
	}



}

void OGGinitAudio(int setID)
{
	//INITIATES THE AUDIO DEVICE.
	//THIS ONLY HAPPENS THE FIRST TIME THE STREAM IS ACTIVATED
    spec[setID].freq = 48000;
    spec[setID].format = MIX_DEFAULT_FORMAT;
    spec[setID].channels = 2;
    spec[setID].samples = 4096;
	if (setID==0) spec[setID].callback = OGGAudioCallbackZero;
	else spec[setID].callback = OGGAudioCallbackOne;
	spec[setID].userdata = globalStream[setID].Vorbis;

	getDevice[setID]=SDL_OpenAudioDevice(NULL, 0, &spec[setID], NULL, SDL_AUDIO_ALLOW_CHANGES);
	if (getDevice[setID]!=0)
	{
		AudioEnabled=true;

		if (AudioEnabled) SDL_PauseAudioDevice(getDevice[setID], 0);
		else SDL_PauseAudioDevice(getDevice[setID], 1);
	}


}


void OGGplayMusic(const char*filename, int volume, int repeat, int id, bool fixclick)
{
	//THIS PLAY AN OGG FILE AS A STREAM[ONLY FOR MUSIC], WITH A CERTAIN VOLUME[volume 0-100]
	//AND IF IT'S -1 IT WILL REPEAT INDEFINITELY OTHERWISE IT WILL REPEAT FROM 0-X, whereas 0 means no repetitions, only plays once,
	//and x is an integer that's higher than 0 and will repeat x amount of times.
	if (repeat==-1) repeat=1;
	else repeat=0;

	SDL_LockAudioDevice(getDevice[id]);

	globalStream[id].repeat=repeat;
	globalStream[id].volume=volume;
	globalStream[id].Filename=filename;
	globalStream[id].fix_click=fixclick;
    if (globalStream[id].Vorbis!=NULL)
	{
		stb_vorbis_close(globalStream[id].Vorbis);
		globalStream[id].Vorbis=NULL;
	}
	globalStream[id].Vorbis = stb_vorbis_open_filename(filename, NULL, NULL);
    SDL_UnlockAudioDevice(getDevice[id]);

}




void GetPath(const char* destinationPath, std::string Folder,std::string Extension, int file)
{
	char fullPath[1000];
	std::string MusicName=Folder;
	//ADDS THE NUMBER
	std::string out_string;
	std::stringstream ss;
	ss << file;
	out_string = ss.str();
	MusicName=MusicName.append(out_string);
	//ADDS THE NUMBER
	MusicName=MusicName.append(Extension);
	char MFXN[1024];
	std::strcpy(MFXN, MusicName.c_str());
	engine->GetPathToFileInCompiledFolder(MFXN, fullPath);
  //char* anarray = (char*) malloc(1024 * sizeof(char));
	std::strcpy((char*)destinationPath, fullPath);
	return;
}

void GetMusicPath(const char* destinationPath, int j)
{
	//THIS BASICALLY SETS A CONST CHAR VALUE TO A PATH, IN THIS CASE IT'S THE MUSIC FOLDER AND ALSO POINTS TO A MUSICXX.MFX FILE, WHICH IS AN OGG FILE JUST RENAMED.
#ifdef WIN32
	 GetPath(destinationPath, "Music\\music",".mfx",j);
#else
	GetPath(destinationPath, "Music/music",".mfx",j);
#endif
}

void GetSoundPath(const char* destinationPath, int j)
{
	//THIS BASICALLY SETS A CONST CHAR VALUE TO A PATH, IN THIS CASE IT'S THE SOUND FOLDER AND ALSO POINTS TO A SOUNDXX.SFX FILE, WHICH IS AN OGG FILE JUST RENAMED.
#ifdef WIN32
	GetPath(destinationPath, "Sounds\\sound",".sfx",j);
#else
	GetPath(destinationPath, "Sounds/sound",".sfx",j);
#endif


}



void ApplyFilter(int SetFrequency)
{
	//THIS TURNS ON THE LOW PASS FILTER
	OGG_Filter=true;
	GeneralAudio.FilterFrequency=SetFrequency;
	SetFilterFrequency(SetFrequency);


	int i=0;
	while (i < 500)
	{
		if (SFX[i].playing && SFX[i].filter==1 && SFX[i].volume>1)
		{
			Mix_RegisterEffect(SFX[i].channel, LPEffect, NULL, NULL);
		}
		i++;
	}
	#ifdef WIN32
	#else	
	Mix_RegisterEffect(MIX_CHANNEL_POST, LPEffect, NULL, NULL);
	#endif

}



void RemoveFilter()
{
	//THIS TURNS OFF THE LOW PASS FILTER
	SetFilterFrequency(-1);
	OGG_Filter=false;
	int i=0;
	while (i < GeneralAudio.NumOfChannels)
	{
		Mix_UnregisterAllEffects(i);
		i++;
	}

	#ifdef WIN32
	#else		
	Mix_UnregisterAllEffects(MIX_CHANNEL_POST);
	#endif
}


void UnloadSFX(int i)
{
	//INTERNAL USAGE
	//THIS UNLOADS A SOUND FILE FROM MEMORY
	if (SFX[i].chunk!=NULL)
	{
		Mix_FreeChunk(SFX[i].chunk);
		SFX[i].chunk=NULL;
	}
}

void LoadSFX(int i)
{
	//INTERNAL USAGE, THIS LOADS A SOUND FILE TO MEMORY
	char musicPath[1024];
	GetSoundPath(musicPath,i);
	SFX[i].chunk = Mix_LoadWAV(musicPath);
}

void SetAudioDriver(const char*name)
{
	//SETS THE AUDIO DRIVER'S ENVIRONMENT VALUE
	//SEE BELOW IN SDLMAIN
	SDL_setenv("SDL_AUDIODRIVER", name , 1);
}

void SDLMain()
{
	

	#ifdef WIN32
	    if (SDL_AudioInit("alsa")==0)	//WASAPI 0
		{
			SetAudioDriver("alsa");
		}
		else if (SDL_AudioInit("pulseaudio")==0)	//DUMMY 4
		{
			SetAudioDriver("pulseaudio");
		}
		else if (SDL_AudioInit("dsp")==0)	//DUMMY 4
		{
			SetAudioDriver("dsp");
		} 
	    else if (SDL_AudioInit("xaudio")==0)
		{
			SetAudioDriver("xaudio");
		}
		else if (SDL_AudioInit("directsound")==0)	//DIRECTSOUND 1
		{
			SetAudioDriver("directsound");
		}
		else if (SDL_AudioInit("winmm")==0)	//WINMM 2
		{
			SetAudioDriver("winmm");
		}
		else if (SDL_AudioInit("wasapi")==0)	//WASAPI 0
		{
			SetAudioDriver("wasapi");
		}
		
		else
		{
			GeneralAudio.Disabled=true;
		}
	#endif



       if (!GeneralAudio.Disabled)
	   {
		   SDL_Init(SDL_INIT_AUDIO);
		   #ifdef WIN32
		   SDL_AudioInit(SDL_GetCurrentAudioDriver());
		   #endif
		   if (Mix_OpenAudio(48000,MIX_DEFAULT_FORMAT,2,4096) <0)
		   {
			   GeneralAudio.Disabled=true;
			   SDL_AudioQuit();
			   return;
			   //RETURN ERROR
		   }

		   GeneralAudio.NumOfChannels=60;
		   Mix_AllocateChannels(GeneralAudio.NumOfChannels);
		   
		   int i=0;
		   while (i<500-1)
		   {
			   SFX[i].repeat=0;
			   SFX[i].playing=0;
			   SFX[i].volume=128;
			   SFX[i].allow=0;
			   SFX[i].filter=1;
			   SFX[i].channel=-2;
			   i++;
		   }
		   initAudio();
	   }

}



void SFXSetVolume(int SoundToAdjust,int vol)
{
	//INDIVIDUAL VOLUME PER SFX
	if (SFX[SoundToAdjust].chunk!=NULL)
	{
		Mix_VolumeChunk(SFX[SoundToAdjust].chunk,vol);
		SFX[SoundToAdjust].volume=vol;
	}
}

int SFXGetVolume(int SoundToAdjust)
{
	//INDIVIDUAL VOLUME GET PER SFX
	if (SFX[SoundToAdjust].chunk==NULL)
	{
		return 0;
	}
	return Mix_VolumeChunk(SFX[SoundToAdjust].chunk,-1);
}

void SFXFilter(int SoundToFilter,int enable)
{
	//THIS ENABLES/DISABLES the SFX LOW PASS FILTER, 
	//I think by default all sound effects are affected by low pass, but there are some that i've manually disabled from being affected by it with this command
	SFX[SoundToFilter].filter=enable;
}

void SFXAllowOverlap(int SoundToAllow,int allow)
{
	//ALLOWS SOUND FILES TO STACK. MEANING IF U PLAY THE SAME SOUND IT WILL NOT RESTART, BUT INSTEAD WILL PLAY THE SAME SOUND ON A DIFFERENT CHANNEL
	SFX[SoundToAllow].allow=allow;
}

void PlaySFXNoLowPass(int i,int volume)
{
	//USED FOR LINUX.
	if (GeneralAudio.Disabled)
	{
		return;
	}
	
	char soundPath[1024];
	GetSoundPath(soundPath,i);
	
	playSound(soundPath,volume);

}

void PlaySFX(int SoundToPlay, int repeat)
{
	//PLAYS A SOUND FOUL. REPEAT-1 MEANS INDEFINITE REPEATS, 0 MEANS NO REPEAT, 1 MEANS 1 REPEAT AND SO FORTH.
	//PlaySFX(5,-1) will play SOUND5.SFX in tHE SOUNDS folder.
	if (GeneralAudio.Disabled)
	{
		return;
	}
	#ifdef WIN32
	#else
	if (OGG_Filter && SFX[SoundToPlay].filter==0)
	{
		int snd=SoundToPlay+1000;
		if (snd == 1034 || snd==1036 || snd==1037 || snd==1038 || snd==1039 || snd==1040
			|| snd==1053 || snd==1125 || snd==1210 || snd==1237 || snd==1249 || snd==1261
			|| snd==1423)
		{
			PlaySFXNoLowPass(snd,GeneralAudio.SoundValue);
		}
		return;
	}
	#endif
    if (SFX[SoundToPlay].chunk==NULL)
	{
		LoadSFX(SoundToPlay);
	}

	if (SFX[SoundToPlay].chunk!=NULL)
	{
		int i=0;
		int id=-1;
		while (i < GeneralAudio.NumOfChannels)
		{
			if (Mix_Playing(i)!=0 &&Mix_GetChunk(i)!=NULL && Mix_GetChunk(i)==SFX[SoundToPlay].chunk)//
			{
				id=i;
			}
			i++;
	    }


		if (SFX[SoundToPlay].allow==1)
		{
			id=-1;
		}
		else 
		{
			if (SFX[SoundToPlay].playing)
			{
				id=20;
			}
		}

		if (id==-1)
		{

		SFX[SoundToPlay].volume=SFXGetVolume(SoundToPlay);

		Mix_VolumeChunk(SFX[SoundToPlay].chunk,SFX[SoundToPlay].volume);
		int rep=repeat;
		if (repeat!=-1)repeat=0;
		int grabChan=Mix_PlayChannel(-1,SFX[SoundToPlay].chunk,repeat);	//-1
		SFX[SoundToPlay].channel=grabChan;
		Mix_Volume(grabChan,GeneralAudio.SoundValue);
		Mix_UnregisterAllEffects(grabChan);

		if (OGG_Filter && SFX[SoundToPlay].filter&& SFX[SoundToPlay].volume>1)
		{
			Mix_RegisterEffect(grabChan, LPEffect, NULL, NULL);
		}


		SFX[SoundToPlay].repeat=repeat;
		SFX[SoundToPlay].playing=1;
		//SFX[SoundToPlay].position=0;
		}
	}
}






void SFXSetPosition(int SoundToSet,int xS,int yS,int intensity)
{
  if (SFX[SoundToSet].chunk!=NULL)
  {

  int i=0;
  int id=-1;
  while (i < GeneralAudio.NumOfChannels)
  {
	  if (Mix_Playing(i)!=0 && Mix_GetChunk(i)!=NULL && Mix_GetChunk(i)==SFX[SoundToSet].chunk)
	  {
		  id=i;
	  }
	  i++;
  }

  if (id!=-1)
  {
      int angle=0;
	  int dist=0;


	  if (xS!=0 && yS!=0)
	  {
	  int pid=engine->GetPlayerCharacter();
	  playerCharacter = engine->GetCharacter(pid);

      int x1=Character_GetX(playerCharacter);
	  int y1=Character_GetY(playerCharacter);

	  int x2=xS;
	  int y2=yS;

	  int defx = (x1-x2)*(x1-x2);
	  int defy = (y1-y2)*(y1-y2);

	  float SquareRoot=sqrt (float(defx + defy));
	  dist=int(SquareRoot)-intensity;
	  if (dist >255) dist=255;
	  if (dist <0)  dist=0;

	  float xDiff = float(x2 - x1);
	  float yDiff = float(y2 - y1);
	  float at2= atan2(yDiff,xDiff);
	  //float angles= (at2 * 180.0 / PI);
      //angle=int(angles)%360;

	  float angles= (at2 * 360.0 / PI);
      angle=int(angles);//%360;


	  }




	  Mix_SetPosition(id,angle,dist);
  }
  }
}


void SFXStop(int SoundToStop,int fadems)
{
	if (SFX[SoundToStop].chunk!=NULL)
	{

		int i=0;
		while (i < GeneralAudio.NumOfChannels)
		{
			if (Mix_Playing(i)!=0 && Mix_GetChunk(i)!=NULL && Mix_GetChunk(i)==SFX[SoundToStop].chunk)
			{
				SFX[SoundToStop].playing=0;
				SFX[SoundToStop].repeat=0;
				SFX[SoundToStop].channel=-2;
				//SFX[SoundToStop].position=0;
				Mix_FadeOutChannel(i, fadems);	
				if (fadems==0) UnloadSFX(SoundToStop);
			}
			i++;
		}
	}
}



void MusicSetVolume(int vol)
{
	Mix_VolumeMusic(vol);
}

int MusicGetVolume()
{
	return Mix_VolumeMusic(-1);
}




void MusicPlay(int MusicToPlay, int repeat, int fadeinMS,int fadeoutMS,int pos,bool forceplay,bool fixclick)
{
	if (GeneralAudio.Disabled)
	{
		return;
	}

    bool samefile=currentMusic!=MusicToPlay;
    if (forceplay) samefile=true;

	if (samefile)
	{
	currentMusicRepeat=repeat;
	currentMusicFadein=fadeinMS;
	currentMusic=MusicToPlay;
	#ifdef WIN32
	if (!MFXStream.Switch)
	{
		MFXStream.Channel=0;
		if (!GeneralAudio.Initialized)
		{
			OGGinitAudio(0);
		}
		OGGplayMusic(MusicLoads[MusicToPlay].musicPath, 0,repeat,0,fixclick);
		MFXStream.ID=MusicToPlay;		
		MFXStream.FadeTime=(fadeinMS/1000)*40;
		MFXStream.FadeRate=float(MusicGetVolume())/float(MFXStream.FadeTime);
		MFXStream.FadeVolume=0.0;
		MFXStream.HaltedZero=false;
		//MusicVolCanBeAdjusted=true;
	}
	else
	{
		MFXStream.HaltedOne=false;
		MFXStream.Channel=1;
		if (!GeneralAudio.Initialized)
		{
			OGGinitAudio(1);
			GeneralAudio.Initialized=true;
		}
		OGGplayMusic(MusicLoads[MusicToPlay].musicPath, 0,repeat,1,fixclick);

		MFXStream.ID=MusicToPlay;
		MFXStream.FadeTime=(fadeoutMS/1000)*40;
		MFXStream.FadeVolume=0.0;//float(MusicGetVolume());
		MFXStream.FadeRate=float(MusicGetVolume())/float(MFXStream.FadeTime);		
		//MusicVolCanBeAdjusted=false;
	}
	MFXStream.Switch=!MFXStream.Switch;
	#else	
	MFXStream.ID=MusicToPlay;

	MFXStream.FadeTime=((fadeinMS/2)/1000)*40;

	MFXStream.FadeVolume=0.0;//float(MusicGetVolume());

	MFXStream.FadeRate=float(MusicGetVolume())/float(MFXStream.FadeTime);
	MFXStream.Channel=0;		
	Mix_FadeOutMusic(fadeinMS/4);
	#endif
	}

}



void GlitchFix()
{
	#ifdef WIN32
	if (MFXStream.Channel!=-1)
	{
	if (MFXStream.Channel==0 || MFXStream.Channel==1)
	{
		MFXStream.FadeVolume=float(MusicGetVolume());		
		if (MFXStream.Channel==0) 
		{
			if (!MFXStream.HaltedZero)globalStream[0].volume=MFXStream.FadeVolume;//0-100
			if (!MFXStream.HaltedOne)globalStream[1].volume=float(MusicGetVolume())-globalStream[0].volume;//100-0
		}
		else 
		{
			if (!MFXStream.HaltedOne)globalStream[1].volume=MFXStream.FadeVolume;//0-100
			if (!MFXStream.HaltedZero)globalStream[0].volume=float(MusicGetVolume())-globalStream[1].volume;//100-0
		}
		MFXStream.FadeTime=0;
		MFXStream.Channel=-1;		
	}
	else if (MFXStream.Channel==2 || MFXStream.Channel==3)
	{
		MFXStream.FadeTime=0;
		MFXStream.FadeVolume=0.0;		
		globalStream[MFXStream.Channel-2].volume=MFXStream.FadeVolume;
		if (MFXStream.Channel==2) MFXStream.HaltedZero=true;
		else MFXStream.HaltedOne=true;
		MFXStream.Channel=-1;		
	}
	}
	#endif
}


void SFXSetGlobalVolume(int Vol)
{
	GeneralAudio.SoundValue=Vol;
}



void Update()
{
	if (GeneralAudio.Disabled)
	{
		return;
	}

	#ifdef WIN32
	if (MFXStream.Channel==0 || MFXStream.Channel==1)
	{
		MFXStream.FadeTime--;
		MFXStream.FadeVolume+=MFXStream.FadeRate;
		if (MFXStream.FadeVolume > float(MusicGetVolume()))
		{
			MFXStream.FadeVolume=float(MusicGetVolume());
		}
		if (MFXStream.Channel==0) 
		{
			if (!MFXStream.HaltedZero)globalStream[0].volume=MFXStream.FadeVolume;//0-100
			if (!MFXStream.HaltedOne)globalStream[1].volume=float(MusicGetVolume())-globalStream[0].volume;//100-0
		}
		else 
		{
			if (!MFXStream.HaltedOne)globalStream[1].volume=MFXStream.FadeVolume;//0-100
			if (!MFXStream.HaltedZero)globalStream[0].volume=float(MusicGetVolume())-globalStream[1].volume;//100-0
		}
		

		if (MFXStream.FadeTime<=0)
		{
			MFXStream.Channel=-1;
		}
	}
	if (MFXStream.Channel==2 || MFXStream.Channel==3)
	{
		MFXStream.FadeTime--;
		MFXStream.FadeVolume-=MFXStream.FadeRate;
		if (MFXStream.FadeVolume < 0.0)
		{
			MFXStream.FadeVolume=0.0;
		}
		globalStream[MFXStream.Channel-2].volume=MFXStream.FadeVolume;//0-100

		if (MFXStream.FadeTime<=0)
		{
			if (MFXStream.Channel==2) MFXStream.HaltedZero=true;
			else MFXStream.HaltedOne=true;
			
			MFXStream.Channel=-1;
		}
	}
	if (MFXStream.Channel==-1 )//&& MusicVolCanBeAdjusted)
	{
		if (MFXStream.Switch)
		{
			if (!MFXStream.HaltedZero)globalStream[0].volume=MusicGetVolume();
			else globalStream[0].volume=0;
			globalStream[1].volume=0;
		}
		else 
		{
			if (!MFXStream.HaltedOne)globalStream[1].volume=MusicGetVolume();
			else globalStream[1].volume=0;
			globalStream[0].volume=0;
		}
	}
	#else
	if (MFXStream.Channel==0)
	{
		MFXStream.FadeTime--;

		MFXStream.FadeVolume+=MFXStream.FadeRate;

		if (MFXStream.FadeVolume >= float(MusicGetVolume()))

		{
		  MFXStream.FadeVolume=float(MusicGetVolume());
		  Mix_FadeInMusic(musiceffect[currentMusic],currentMusicRepeat,currentMusicFadein);
		  MFXStream.Channel=-1;

		}								  
		
	}
	#endif
	int j=0;
	while (j < 500-1)
	{

		int i=0;
		int id=-1;
		while (i < GeneralAudio.NumOfChannels)
		{
			if (Mix_Playing(i) && Mix_GetChunk(i)!=NULL && Mix_GetChunk(i)==SFX[j].chunk)
			{
				if (SFX[j].playing==1)Mix_Volume(i,GeneralAudio.SoundValue);
				id=i;
		    }
			if (!Mix_Playing(i) && Mix_GetChunk(i)!=NULL && SFX[j].chunk!=NULL && Mix_GetChunk(i)==SFX[j].chunk && SFX[j].playing==0
				&& SFX[j].repeat==0)
			{
				//UnloadSFX(j);
			}

			i++;
	    }
		if (id!= -1)
		{
			//SOUND IS PLAYING			
		}
		else
		{
			//sound is not playing
			//IF REPEAT PLAY SOUND
			if (SFX[j].repeat!=0 && SFX[j].repeat!=-1)
			{
				//REDUCE REPEAT BY 1
				if (SFX[j].repeat>0) SFX[j].repeat-=1;
				SFXSetVolume(j,SFX[j].volume);
				int grabChan=SFX[j].channel;
			    Mix_PlayChannel(grabChan,SFX[j].chunk,0);
				Mix_Volume(grabChan,GeneralAudio.SoundValue);
				Mix_UnregisterAllEffects(grabChan);
				if (SFX[j].filter==1 && OGG_Filter && SFX[j].volume>1)
				{
					Mix_RegisterEffect(grabChan, LPEffect, NULL, NULL);
				}
				SFX[j].playing=1;
			}
			else 
			{
				SFX[j].channel=-2;
				SFX[j].playing=0;
			}
			//IF NOT DO NOTHING
		}
		j++;
	}



}



void CastWave(int delayMax, int PixelsWide,int n)
{

  tDy[n]++;
  if (tDy[n] >delayMax)
  {
    tDy[n]=0;
    if (direction[n]==0) dY[n]++;
	if (direction[n]==1) dY[n]--;
	if ( (dY[n]>PixelsWide && direction[n]==0) || (dY[n]<(-PixelsWide) && direction[n]==1) )
	{
		if (direction[n]==0){dY[n]=PixelsWide;direction[n]=1;}
		else {dY[n]=-PixelsWide;direction[n]=0;}
	}
  }


}



int getRcolor(int color) {
	return ((color >> 16) & 0xFF);
}
int getGcolor(int color) {
	return ((color >> 8) & 0xFF);
}
int getBcolor(int color) {
	return ((color >> 0) & 0xFF);
}
int getAcolor(int color)
{
	return ((color >> 24) & 0xFF);
}


float clamp(float x, float min, float max) {
    float value=x;
	if(value < min) value= min;
    if(value > max) value= max;
    return value;
}

int ConvertColorToGrayScale(int color)
{
	int r=getRcolor(color);
	int g=getGcolor(color);
	int b=getBcolor(color);

	float d=float ((r * r + g * g + b * b) /3);
	int gr = int(sqrt(d));

	return ((gr << 16) | (gr << 8) | (gr << 0) | (255 << 24));
}


int Random(int value)
{
	return (rand() % (value+1));
}

void CreateParticle(int xx, int yy, int ForceX, int ForceY)
{
  int h=0;
  bool foundparticle=false;
  int fid=-1;
  while (h <= dsize && !foundparticle)
  {
    if (particles[h].active==false)
    {
      foundparticle=true;
      fid=h;
    }
    h++;
  }

  if (foundparticle)
  {
    int d=fid;
    particles[d].x=xx;
    particles[d].y=yy;
    particles[d].dx=0;
    particles[d].dy=0;
    particles[d].life=20000;
    particles[d].transp=55+Random(10);
    particles[d].active=true;
    particles[d].mlay=4+Random(2);
    particles[d].timlay=0;
    particles[d].translay=0;
    particles[d].translayHold=19+Random(15);
    particles[d].width=2+Random(2);
    particles[d].height=particles[d].width;
    particles[d].fx=0;
    particles[d].fy=0;
    particles[d].doingcircle=false;
    particles[d].angle=0.0;
    particles[d].radius=4.0+float(Random(6));
    particles[d].doingCircleChance=Random(200);
    particles[d].angleLay=0.0;
    particles[d].frame=0;
    particles[d].anglespeed=float(Random(20))/100.0;
    WForceX[d]=ForceX;
    WForceY[d]=ForceY;
    if (dsize<(raysize-1)) dsize++;
  }
}

void CreateParticle2(int xx, int yy, int ForceX, int ForceY)
{
  int h=0;
  bool foundparticle=false;
  int fid=-1;
  while (h <= dsize2 && !foundparticle)
  {
    if (particles2[h].active==false)
    {
      foundparticle=true;
      fid=h;
    }
    h++;
  }

  if (foundparticle)
  {
    int d=fid;
    particles2[d].x=xx;
    particles2[d].y=yy;
    particles2[d].dx=0;
    particles2[d].dy=0;
    particles2[d].life=20000;
    particles2[d].transp=65+Random(15);
    particles2[d].active=true;
    particles2[d].mlay=4+Random(2);
    particles2[d].timlay=0;
    particles2[d].translay=0;
    particles2[d].translayHold=19+Random(15);
    particles2[d].width=16;
    particles2[d].height=particles[d].width;
    particles2[d].fx=0;
    particles2[d].fy=0;
    particles2[d].doingcircle=false;
    particles2[d].angle=0.0;
    particles2[d].radius=4.0+float(Random(6));
    particles2[d].doingCircleChance=Random(200);
    particles2[d].angleLay=0.0;
    particles2[d].frame=0;
    particles2[d].anglespeed=float(Random(20))/100.0;
    WForceX[d+200]=ForceX;
    WForceY[d+200]=ForceY;
    if (dsize2<(raysize2-1)) dsize2++;
  }
}

void CreateParticleF(int xx, int yy, int ForceX, int ForceY)
{
  int h=0;
  bool foundparticle=false;
  int fid=-1;
  while (h <= dsizeF && !foundparticle)
  {
    if (particlesF[h].active==false)
    {
      foundparticle=true;
      fid=h;
    }
    h++;
  }

  if (foundparticle)
  {
    int d=fid;
    particlesF[d].x=xx;
    particlesF[d].y=yy;
    particlesF[d].dx=(-1)+Random(1);
    particlesF[d].dy=(-1)+Random(1);
    particlesF[d].life=20000;
    particlesF[d].transp=45+Random(10);
    particlesF[d].active=true;
    particlesF[d].mlay=4+Random(2);
    particlesF[d].timlay=0;
    particlesF[d].translay=0;
    particlesF[d].translayHold=19+Random(15);
    particlesF[d].width=8+Random(2);
    particlesF[d].height=particlesF[d].width;
    particlesF[d].fx=0;
    particlesF[d].fy=0;
    particlesF[d].doingcircle=false;
    particlesF[d].angle=0.0;
    particlesF[d].radius=4.0+float(Random(6));
    particlesF[d].doingCircleChance=Random(200);
    particlesF[d].angleLay=0.0;
    WForceX[d+100]=ForceX;
    WForceY[d+100]=ForceY;
    particlesF[d].frame=0;
    if (dsizeF<(raysizeF-1)) dsizeF++;

  }
}



void SetWindValues(int w,int h,int pr,int prev)
{
	ww=w;
	hh=h;
	proom=pr;
	prevroom=prev;
}
int SetColorRGBA(int r,int g,int b,int a)
{
	r = int(clamp(float(r),0.0,255.0));
	g = int(clamp(float(g),0.0,255.0));
	b = int(clamp(float(b),0.0,255.0));
	a = int(clamp(float(a),0.0,255.0));
	return int ((r << 16) | (g << 8) | (b << 0) | (a << 24));
}

void WindUpdate(int ForceX, int ForceY, int Transparency,int sprite)
{
  BITMAP* src = engine->GetSpriteGraphic(sprite);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


    int by=0;
    while (by <2)
    {
		int dnx=Random(ww+250)-250;
		int dny=Random(hh);
		CreateParticle(dnx, dny, ForceX, ForceY);
		by++;
    }

	int dnx;
	if (ForceX >0) dnx=(Random(ww+250)-250)-(50 + Random(100));
	else dnx=Random(ww+250)-250;
	//
    int dny=Random(hh);
	CreateParticle2(dnx, dny, ForceX, ForceY);


    dnx=-(20+Random(50));//Random(ww);
    if (dnx<-160) dnx=-160;
    if (dnx > ww+160) dnx=ww+160;

    dny=Random(hh);
    CreateParticleF(dnx, dny, ForceX, ForceY);

  int h=dsize-1;

  if (h < dsizeF-1)
  {
    h=dsizeF-1;
  }

  int setByx=0;
  if (proom==3 && prevroom==14)
  {
    setByx=640;
  }
  if (proom==4 && prevroom==8)
  {
    setByx-=480;
  }
  while (h>0)
  {
    if (particles[h].life>0)
    {
      particles[h].life-=int(3.0);
      particles[h].doingCircleChance-=2;
      int df=100-particles[h].transp;
      df=10-(df/4);

      int pwidth=particles[h].width+df;
      int pheight=particles[h].height+df;

      int px=particles[h].x-(pwidth/2);
      int py=particles[h].y-(pheight/2);
      int tp=particles[h].transp+Transparency;

      if (tp>100) tp=100;

      int pgraph=0;
      int SplitBetweenTwo=Random(100);
      if (SplitBetweenTwo<=50) pgraph=813;
      else pgraph=4466;

      if (tp!=100)
	  {

		  BITMAP* src2 = engine->GetSpriteGraphic(pgraph+particles[h].frame);


		  int src2_width=640;
		  int src2_height=360;
		  int src2_depth=32;
		  engine->GetBitmapDimensions(src2,&src2_width,&src2_height,&src2_depth);
		  unsigned int** sprite_pixels2 = (unsigned int**)engine->GetRawBitmapSurface(src2);
		  engine->ReleaseBitmapSurface(src2);

		  int startx=px+setByx;
		  int endx=px+setByx+src2_width;
		  int starty=py;
		  int endy=py+src2_height;



		  int x,y;
		  int ny=0;
		  for (y = starty; y < endy; y++)
		  {
			  int nx=0;
			  for (x = startx; x < endx; x++)
			  {
				  int setX=nx;
				  int setY=ny;
				  if (setX < 0)setX =0;
				  if (setX > src2_width-1) setX=src2_width-1;
				  if (setY< 0) setY =0;
				  if (setY> src2_height-1) setY=src2_height-1;

				  int netX=x;
				  int netY=y;


				  if (netX < 0) netX =0;
				  if (netX > src_width-1) netX=src_width-1;
				  if (netY <0) netY=0;
				  if (netY > src_height-1) netY=src_height-1;

				  int clr=sprite_pixels2[setY][setX];
				  int rv=getRcolor(clr);
				  int gv=getGcolor(clr);
				  int bv=getBcolor(clr);
				  int av=getAcolor(clr);

				  av = int (float((av*(100-tp)))/100.0);

				  sprite_pixels[netY][netX]=SetColorRGBA(rv,gv,bv,av);
				  nx++;
			  }
			  ny++;
		  }

      }
      particles[h].timlay+=int(6.0);
      if (particles[h].timlay> particles[h].mlay)
      {
        particles[h].frame++;
        if (particles[h].frame>6) particles[h].frame=0;
        particles[h].timlay=0;
        particles[h].x += particles[h].dx+particles[h].fx;
        particles[h].y += particles[h].dy+particles[h].fy;//Random(1);
      }
      particles[h].translay+=2;
      if (particles[h].translay>=particles[h].translayHold)
      {
        if (particles[h].transp<=99) particles[h].transp++;
        else
        {
          particles[h].life=0;
        }
      }
      if (particles[h].x>=(ww-90)+setByx || particles[h].x<90+setByx)
      {
        if (particles[h].transp<=99)particles[h].transp++;
        else
        {
          particles[h].life=0;
        }
      }

      if (!particles[h].doingcircle && particles[h].angle==0.0
      && particles[h].doingCircleChance<=0)
      {
        particles[h].doingcircle=true;
      }
      if (particles[h].doingcircle)
      {
        particles[h].angleLay+= float(1+WForceX[h])*1.5;
        if (particles[h].angleLay> 12.0)
        {
          particles[h].angleLay=0.0;
          particles[h].angle+=particles[h].anglespeed;
          int Y=particles[h].y + int((sin(particles[h].angle)* particles[h].radius));
          int X=particles[h].x + int((cos(particles[h].angle)* particles[h].radius));
          particles[h].x=X;
          particles[h].y=Y;
        }
      }
      particles[h].fx=ForceX;
      particles[h].fy=ForceY;

    }
    else
    {
      particles[h].active=false;
    }






    if (h<=5 && particlesF[h].life>0)
    {
      int pwidth=particlesF[h].width;
      int pheight=particlesF[h].height;
      int px=particlesF[h].x-(pwidth/2);
      int py=particlesF[h].y-(pheight/2);
      int pgraph=0;
      int SplitBetweenTwo=Random(100);
      if (SplitBetweenTwo<=50) pgraph=806;
      else pgraph=4459;

      int tp=particlesF[h].transp+Transparency;
      if (tp>100) tp=100;


      if (tp!=100)
	  {

		  BITMAP* src2 = engine->GetSpriteGraphic(pgraph+particlesF[h].frame);
		  int src2_width=640;
		  int src2_height=360;
		  int src2_depth=32;
		  engine->GetBitmapDimensions(src2,&src2_width,&src2_height,&src2_depth);
		  unsigned int** sprite_pixels2 = (unsigned int**)engine->GetRawBitmapSurface(src2);
		  engine->ReleaseBitmapSurface(src2);

		  int startx=px+setByx;
		  int endx=px+setByx+src2_width;
		  int starty=py;
		  int endy=py+src2_height;


		  int x,y;
		  int ny=0;
		  for (y = starty; y < endy; y++)
		  {
			  int nx=0;
			  for (x = startx; x < endx; x++)
			  {
				  int setX=nx;
				  int setY=ny;
				  if (setX < 0)setX =0;
				  if (setX > src2_width-1) setX=src2_width-1;
				  if (setY< 0) setY =0;
				  if (setY> src2_height-1) setY=src2_height-1;

				  int netX=x;
				  int netY=y;


				  if (netX < 0) netX =0;
				  if (netX > src_width-1) netX=src_width-1;
				  if (netY <0) netY=0;
				  if (netY > src_height-1) netY=src_height-1;

				  int clr=sprite_pixels2[setY][setX];
				  int rv=getRcolor(clr);
				  int gv=getGcolor(clr);
				  int bv=getBcolor(clr);
				  int av=getAcolor(clr);

				  av = int (float((av*(100-tp)))/100.0);

				  sprite_pixels[netY][netX]=SetColorRGBA(rv,gv,bv,av);

				  nx++;
			  }
			  ny++;
		  }




       // drawt.DrawImage(px+setByx, py, , tp, pwidth, pheight);
      }
      particlesF[h].timlay+=int(6.0);
      if (particlesF[h].timlay> particlesF[h].mlay)
      {
        particlesF[h].frame++;
        if (particlesF[h].frame>6)  particlesF[h].frame=0;
        particlesF[h].timlay=0;
        particlesF[h].x += particlesF[h].dx + ForceX;
        particlesF[h].y += particlesF[h].dy + ForceY;
      }


      if (particlesF[h].x>=ww-90 || particlesF[h].x<90)
      {
        particlesF[h].translay+=2;
        if (particlesF[h].translay>=particlesF[h].translayHold)
        {
          if (particlesF[h].transp<=99) particlesF[h].transp++;
          else
          {
            particlesF[h].life=0;
          }
        }
      }
    }
    else
    {
      if (h<=9)  particlesF[h].active=false;
    }


//SECOND PARTICLES
	if (h <=10)
	{
	if (particles2[h].life>0)
    {
      particles2[h].life-=int(3.0);
      particles2[h].doingCircleChance-=1;
      int df=100-particles2[h].transp;//45-0
      df=10-(df/4);//10-0

      int pwidth=particles2[h].width+df;
      int pheight=particles2[h].height+df;

      int px=particles2[h].x-(pwidth/2);
      int py=particles2[h].y-(pheight/2);
      int tp=particles2[h].transp+Transparency;

      if (tp>100) tp=100;

      int pgraph=5224;

      if (tp!=100)
	  {

		  BITMAP* src2 = engine->GetSpriteGraphic(pgraph+particles2[h].frame);


		  int src2_width=640;
		  int src2_height=360;
		  int src2_depth=32;
		  engine->GetBitmapDimensions(src2,&src2_width,&src2_height,&src2_depth);
		  unsigned int** sprite_pixels2 = (unsigned int**)engine->GetRawBitmapSurface(src2);
		  engine->ReleaseBitmapSurface(src2);

		  int startx=px+setByx;
		  int endx=px+setByx+src2_width;
		  int starty=py;
		  int endy=py+src2_height;



		  int x,y;
		  int ny=0;
		  for (y = starty; y < endy; y++)
		  {
			  int nx=0;
			  for (x = startx; x < endx; x++)
			  {
				  int setX=nx;
				  int setY=ny;
				  if (setX < 0)setX =0;
				  if (setX > src2_width-1) setX=src2_width-1;
				  if (setY< 0) setY =0;
				  if (setY> src2_height-1) setY=src2_height-1;

				  int netX=x;
				  int netY=y;


				  if (netX < 0) netX =0;
				  if (netX > src_width-1) netX=src_width-1;
				  if (netY <0) netY=0;
				  if (netY > src_height-1) netY=src_height-1;

				  int clr=sprite_pixels2[setY][setX];
				  int rv=getRcolor(clr);
				  int gv=getGcolor(clr);
				  int bv=getBcolor(clr);
				  int av=getAcolor(clr);

				  av = int (float((av*(100-tp)))/100.0);

				  sprite_pixels[netY][netX]=SetColorRGBA(rv,gv,bv,av);
				  nx++;
			  }
			  ny++;
		  }


      }
      particles2[h].timlay+=int(6.0);
      if (particles2[h].timlay> particles2[h].mlay)
      {
        particles2[h].frame++;
        if (particles2[h].frame>7) particles2[h].frame=0;
        particles2[h].timlay=0;
        particles2[h].x += particles2[h].dx+particles2[h].fx;
        particles2[h].y += particles2[h].dy+particles2[h].fy;//Random(1);
      }
      particles2[h].translay+=2;
      if (particles2[h].translay>=particles2[h].translayHold)
      {
        if (particles2[h].transp<=99) particles2[h].transp++;
        else
        {
          particles2[h].life=0;
        }
      }
      if (particles2[h].x>=(ww-90)+setByx || particles2[h].x<90+setByx)
      {
        if (particles2[h].transp<=99)particles2[h].transp++;
        else
        {
          particles2[h].life=0;
        }
      }

      if (!particles2[h].doingcircle && particles2[h].angle==0.0
      && particles2[h].doingCircleChance<=0)
      {
        particles2[h].doingcircle=true;
      }
      if (particles2[h].doingcircle)
      {
        particles2[h].angleLay+= float((1+WForceX[h+200]));
        if (particles2[h].angleLay> 12.0)
        {
          particles2[h].angleLay=0.0;
          particles2[h].angle+=particles2[h].anglespeed;
          int Y=particles2[h].y + int((sin(particles2[h].angle)* particles2[h].radius));
          int X=particles2[h].x + int((cos(particles2[h].angle)* particles2[h].radius));
          particles2[h].x=X;
          particles2[h].y=Y;
        }
      }
      particles2[h].fx=int(float(ForceX)*3.5);
      particles2[h].fy=int(float(ForceY)*3.5);

    }
    else
    {
      particles2[h].active=false;
    }
	}

	//SECOND PARTICLES


    h--;
  }

  engine->ReleaseBitmapSurface(src);

}



void CreateRainParticleMid(int x, int y, int fx, int fy, int maxpart)
{
  int s=0;

  while (s <maxpart)
  {
    if (!RainParticles[s].active)
    {
      RainParticles[s].active=true;
      RainParticles[s].x=x;
      RainParticles[s].y=y;
      RainParticles[s].fx=fx;
      RainParticles[s].fy=fy;
      RainParticles[s].life=2000;
      RainParticles[s].trans=70+Random(25);
      RainParticles[s].transhold=Random(3);
      RainParticles[s].translay=0;
      return;
    }
    s++;
  }
}


void CreateRainParticleFore(int x, int y, int fx, int fy, int maxpart)
{
  int s=0;

  while (s <maxpart)
  {
    if (!RainParticlesFore[s].active)
    {
      RainParticlesFore[s].active=true;
      RainParticlesFore[s].x=x;
      RainParticlesFore[s].y=y;
      RainParticlesFore[s].fx=fx;//int(1.5*float(fx));
      RainParticlesFore[s].fy=fy;//int(1.5*float(fy));
      RainParticlesFore[s].life=2000;
      RainParticlesFore[s].trans=75+Random(15);
      RainParticlesFore[s].transhold=Random(3);
      RainParticlesFore[s].translay=0;
      return;
    }
    s++;
  }
}

void CreateRainParticleBack(int x, int y, int fx, int fy, int maxpart)
{
  int s=0;

  while (s <maxpart)
  {
    if (!RainParticlesBack[s].active)
    {
      RainParticlesBack[s].active=true;
      RainParticlesBack[s].x=x;
      RainParticlesBack[s].y=y;
      if (fx==0) fx=1;
      if (fy==0) fy=1;
      RainParticlesBack[s].fx=fx/2;
      RainParticlesBack[s].fy=fy/2;
      RainParticlesBack[s].life=2000;
      RainParticlesBack[s].trans=70+Random(15);
      RainParticlesBack[s].transhold=2+Random(3);
      RainParticlesBack[s].translay=0;
      return;
    }
    s++;
  }
}




void DrawLineCustom(int x1, int y1, int x2, int y2, int graphic,int setR,int setG,int setB, int setA, int TranDif)
{
  int ALine=0;
  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  int DiffA=-26;

  int x, y;
  int xe;
  int ye;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int dx1 = abs(dx);
  int dy1 = abs(dy);
  int px = (2 * dy1) - dx1;
  int py = (2 * dx1) - dy1;
  if (dy1 <= dx1)
  {
    if (dx >= 0)
    {
      x = x1;
      y = y1;
      xe = x2;
    }
    else
    {
      x = x2; y = y2; xe = x1;
    }

	int xx2=x-320;
	int yy2=y;

	if (xx2 < 0 || xx2 > src_width-1 || yy2 > src_height-1|| yy2 < 0)
	{
	}
	else
	{
		sprite_pixels[yy2][xx2]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	}

	int xx3=x+320;
	int yy3=y;

	if (xx3 < 0 || xx3 > src_width-1 || yy3 > src_height-1|| yy3 < 0)
	{
	}
	else
	{
		sprite_pixels[yy3][xx3]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	}

	int xx=x;
	int yy=y;

	if (xx < 0 || xx > src_width-1 || yy > src_height-1|| yy < 0)
	{
	}
	else
	{
		sprite_pixels[yy][xx]=SetColorRGBA(setR,setG,setB,setA+(ALine*TranDif));
		ALine++;
	}



    int i=0;
    while (x < xe)
    {
      x = x + 1;
      if (px < 0)
      {
        px = px + 2 * dy1;
      }
      else
      {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
        {
          y = y + 1;
        }
        else
        {
          y = y - 1;
        }
        px = px + 2 * (dy1 - dx1);
      }

	  xx2=x-320;
	  yy2=y;
	  if (xx2 < 0 || xx2 > src_width-1 || yy2 > src_height-1|| yy2 < 0)
	  {
	  }
	  else
	  {
		  sprite_pixels[yy2][xx2]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	  }
	  xx3=x+320;
	  yy3=y;
	  if (xx3 < 0 || xx3 > src_width-1 || yy3 > src_height-1|| yy3 < 0)
	  {
	  }
	  else
	  {
		  sprite_pixels[yy3][xx3]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	  }

      xx=x;
	  yy=y;
	  if (xx < 0 || xx > src_width-1 || yy > src_height-1|| yy < 0)
	  {
	  }
	  else
	  {
		  sprite_pixels[yy][xx]=SetColorRGBA(setR,setG,setB,setA+(ALine*TranDif));
		  ALine++;
	  }

      i++;
    }
  }
  else
  {
    if (dy >= 0)
    {
      x = x1;
      y = y1;
      ye = y2-1;
    }
    else
    {
      // Line is drawn top to bottom
      x = x2;
      y = y2;
      ye = y1-1;
    }

	int xx2=x-320;
	int yy2=y;

	if (xx2 < 0 || xx2 > src_width-1 || yy2 > src_height-1|| yy2 < 0)
	{
	}
	else
	{
		sprite_pixels[yy2][xx2]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	}

	int xx3=x+320;
	int yy3=y;

	if (xx3 < 0 || xx3 > src_width-1 || yy3 > src_height-1|| yy3 < 0)
	{
	}
	else
	{
		sprite_pixels[yy3][xx3]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	}
    int xx=x;
	int yy=y;

	if (xx < 0 || xx > src_width-1 || yy > src_height-1|| yy < 0)
	{
	}
	else
	{
		sprite_pixels[yy][xx]=SetColorRGBA(setR,setG,setB,setA+(ALine*TranDif));
		ALine++;
	}

     int i=0;
     while (y < ye)
     {
       y = y + 1;
       if (py <= 0)
       {
         py = py + (2 * dx1);
       }
       else
       {
         if ((dx < 0 && dy<0) || (dx > 0 && dy > 0))
         {
           x = x + 1;
         }
         else
         {
           x = x - 1;
         }
         py = py + 2 * (dx1 - dy1);
       }
	   xx2=x-320;
	   yy2=y;
	  if (xx2 < 0 || xx2 > src_width-1 || yy2 > src_height-1|| yy2 < 0)
	  {
	  }
	  else
	  {
		  sprite_pixels[yy2][xx2]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	  }
	  xx3=x+320;
	  yy3=y;
	  if (xx3 < 0 || xx3 > src_width-1 || yy3 > src_height-1|| yy3 < 0)
	  {
	  }
	  else
	  {
		  sprite_pixels[yy3][xx3]=SetColorRGBA(setR,setG,setB,setA+DiffA+(ALine*TranDif));
	  }
	   xx=x;
	   yy=y;
	   if (xx < 0 || xx > src_width-1 || yy > src_height-1|| yy < 0)
	   {
	   }
	   else
	   {
		   sprite_pixels[yy][xx]=SetColorRGBA(setR,setG,setB,setA+(ALine*TranDif));
		   ALine++;
	   }
       i++;
     }
   }

   engine->ReleaseBitmapSurface(src);
 }


void BlendTwoSprites(int graphic, int refgraphic)
{
  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  BITMAP* refsrc = engine->GetSpriteGraphic(refgraphic);
  int refsrc_width=640;
  int refsrc_height=360;
  int refsrc_depth=32;
  engine->GetBitmapDimensions(refsrc,&refsrc_width,&refsrc_height,&refsrc_depth);
  unsigned int** refsprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(refsrc);
  engine->ReleaseBitmapSurface(refsrc);


  int x, y;

  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)//
	{
		int getColor=sprite_pixels[y][x];
		int rn=getRcolor(getColor);
		int gn=getGcolor(getColor);
		int bn=getBcolor(getColor);
		int an=getAcolor(getColor);




		if (an > 0.0 && rn >4 && gn>4 && bn>4 )
		{
			int getColor2=refsprite_pixels[y][x];
			int rj=getRcolor(getColor2);
			int gj=getGcolor(getColor2);
			int bj=getBcolor(getColor2);
			int aj=getAcolor(getColor2);

			if (rj > 100 || gj > 100 | bj>100)
			{
				sprite_pixels[y][x]=SetColorRGBA(rj,gj,bj,aj);
			}
		}
	}
  }
  engine->ReleaseBitmapSurface(src);


}


int BlendColor (int Ln,int Bn, int perc)
{
	return ((Ln < 128) ? (2 * Bn * Ln/ perc):(perc - 2 * (perc - Bn) * (perc - Ln) / perc));
}

int BlendColorScreen(int Ln,int Bn, int perc)
{
	return (Bn == perc) ? Bn:nmin(perc, (Ln * Ln / (perc - Bn)));
}


void Blend(int graphic, int refgraphic, bool screen,int perc)
{
  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  BITMAP* refsrc = engine->GetSpriteGraphic(refgraphic);
  int refsrc_width=640;
  int refsrc_height=360;
  int refsrc_depth=32;
  engine->GetBitmapDimensions(refsrc,&refsrc_width,&refsrc_height,&refsrc_depth);
  unsigned int** refsprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(refsrc);
  engine->ReleaseBitmapSurface(refsrc);

  int x, y;


  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)//
	{
		int getColor=sprite_pixels[y][x];
		int rn=getRcolor(getColor);
		int gn=getGcolor(getColor);
		int bn=getBcolor(getColor);
		int an=getAcolor(getColor);




		if (an >= 0.0 && rn >4 && gn>4 && bn>4 )
		{
			int getColor2=refsprite_pixels[y][x];
			int rj=getRcolor(getColor2);
			int gj=getGcolor(getColor2);
			int bj=getBcolor(getColor2);
			int aj=getAcolor(getColor2);

			if (!screen)
			{
				rj=BlendColor(rn,rj, perc);
				gj=BlendColor(gn,gj, perc);
				bj=BlendColor(bn,bj, perc);
				aj=BlendColor(an,aj, perc);
			}
			else
			{
				rj=BlendColorScreen(rn,rj, perc);
				gj=BlendColorScreen(gn,gj, perc);
				bj=BlendColorScreen(bn,bj, perc);
				aj=BlendColorScreen(an,aj, perc);
			}


			sprite_pixels[y][x]=SetColorRGBA(rj,gj,bj,aj);

		}
	}
  }
  engine->ReleaseBitmapSurface(src);


}

void ReverseTransparency(int graphic)
{
  BITMAP* noisesrc = engine->GetSpriteGraphic(graphic);
  int noisesrc_width=640;
  int noisesrc_height=360;
  int noisesrc_depth=32;
  engine->GetBitmapDimensions(noisesrc,&noisesrc_width,&noisesrc_height,&noisesrc_depth);
  unsigned int** noise_pixels = (unsigned int**)engine->GetRawBitmapSurface(noisesrc);
  engine->ReleaseBitmapSurface(noisesrc);


  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  int x, y;


  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)//
	{
		int getColors=noise_pixels[y][x];
		int redClr=getRcolor(getColors);
		int greenClr=getGcolor(getColors);
		int blueClr=getBcolor(getColors);
		int TranClr=getAcolor(getColors);



	    if (TranClr < 254) 
		{
			//PIXEL IS TRANSPARENT
			sprite_pixels[y][x]=SetColorRGBA(255,255,255,255);
		}
		else 
		{
			//PIXEL IS VISIBLE
			sprite_pixels[y][x]=SetColorRGBA(0,0,0,0);
		}		
		
	}
  }
  engine->ReleaseBitmapSurface(src);


}




void NoiseCreator(int graphic, int setA)
{
  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  int x, y;


  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)//
	{
		int getColor=sprite_pixels[y][x];
		int r=rand()%255;
		int g=rand()%255;
		int b=rand()%255;
		int a=setA;

	    sprite_pixels[y][x]=SetColorRGBA(r,g,b,a);	
		
	}
  }
  engine->ReleaseBitmapSurface(src);


}

void Dissolve(int graphic, int noisegraphic, int disvalue)
{
  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);


  BITMAP* noisesrc = engine->GetSpriteGraphic(noisegraphic);
  int noisesrc_width=640;
  int noisesrc_height=360;
  int noisesrc_depth=32;
  engine->GetBitmapDimensions(noisesrc,&noisesrc_width,&noisesrc_height,&noisesrc_depth);
  unsigned int** noise_pixels = (unsigned int**)engine->GetRawBitmapSurface(noisesrc);
  engine->ReleaseBitmapSurface(noisesrc);

  int x, y;


  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)//
	{
		int getColor=noise_pixels[y][x];
		int gn=getRcolor(getColor);

		
		int getColorx=sprite_pixels[y][x];
		int rj=getRcolor(getColorx);
		int gj=getGcolor(getColorx);
		int bj=getBcolor(getColorx);
		int originalA=getAcolor(getColorx);
		int aj=0;

		//disvalue 0-255
		//FOR EACH PIXEL IN THE NOISE GRAPHIC THAT IS < DISVALUE
		if (gn < disvalue) 
		{
			if (gn > disvalue-2)
			{
				rj=193+Random(20);
				gj=132+Random(20);
				bj=255+Random(20);
				aj=originalA;
			}
			else if (gn > disvalue-3)
			{
				rj=128+Random(20);
				gj=0+Random(20);
				bj=255+Random(20);
				aj=150;
			}
			else 
			{
				aj=0;
			}
		}
		else aj=originalA;

		//if (originalA 


        if (originalA>50)
		{
			sprite_pixels[y][x]=SetColorRGBA(rj,gj,bj,aj);
		}
		
	}
  }
  engine->ReleaseBitmapSurface(src);


}


int IntersectLines(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
  // check a
  if (x1 == x2 && y1 == y2) return -1;
  // check b
  if (x3 == x4 && y3 == y4) return -1;
  float den = (y4-y3)*(x2-x1)-(x4-x3)*(y2-y1);
  float num12 = (x4-x3)*(y1-y3)-(y4-y3)*(x1-x3);
  float num34 = (x2-x1)*(y1-y3)-(y2-y1)*(x1-x3);

  if (den == 0.0) {  // no intersection
    if (num12 == 0.0 && num34 == 0.0) return 2;
    return 0;
  }
  ua = num12/den;
  ix = x1 + ua*(x2-x1);
  iy = y1 + ua*(y2-y1);
  return 1;
}

float min4(float m1, float m2, float m3, float m4) {
  float a=nmin(m1, m2);
  float b=nmin(m3, m4);
  return nmin(a,b);
}
float max4(float m1, float m2, float m3, float m4) {
  float a=nmax(m1, m2);
  float b=nmax(m3, m4);
  return nmax(a,b);
}

int ReturnWidth(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  float ax = float(x1), ay = float(y1);
  float bx = float(x2), by = float(y2);
  float cx = float(x3), cy = float(y3);
  float dx = float(x4), dy = float(y4);

  return (int(max4(ax, bx, cx, dx))+1);
}

int ReturnHeight(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  float ax = float(x1), ay = float(y1);
  float bx = float(x2), by = float(y2);
  float cx = float(x3), cy = float(y3);
  float dx = float(x4), dy = float(y4);

  return (int(max4(ay, by, cy, dy))+1);
}

int newheight;
int newwidth;

int ReturnNewHeight()
{
	return newheight;
}

int ReturnNewWidth()
{
	return newwidth;
}

int y2;
int x3;
int y3;
int x4;
int y4;

void SetWarper(int y2x,int x3x,int y3x,int x4x,int y4x)
{
	y2=y2x;
	x3=x3x;
	y3=y3x;
	x4=x4x;
	y4=y4x;
}

void Warper(int swarp,int sadjust,int x1, int y1, int x2)
{
  ix=0.0;
  iy=0.0;
  ua=0.0;
  // some precautions against non-positive values for width and height

  float ax = float(x1), ay = float(y1);
  float bx = float(x2), by = float(y2);
  float cx = float(x3), cy = float(y3);
  float dx = float(x4), dy = float(y4);

  int w = int(max4(ax, bx, cx, dx))+1;
  int h = int(max4(ay, by, cy, dy))+1;

  BITMAP*refsrc = engine->GetSpriteGraphic(swarp);
  int refsrc_width=640;
  int refsrc_height=360;
  int refsrc_depth=32;
  engine->GetBitmapDimensions(refsrc,&refsrc_width,&refsrc_height,&refsrc_depth);
  unsigned int** refsprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(refsrc);
  engine->ReleaseBitmapSurface(refsrc);


  // create temporary sprite holding the warped version
  BITMAP*resizeb=engine->GetSpriteGraphic(sadjust);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(resizeb,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(resizeb);


  int ow = refsrc_width, oh = refsrc_height;

  int x, y;  // pixel coords
  float fx, fy; // original sprite's in between pixel coords

  int il;

  // calculate intersections of opposing sides
  float orx_x, orx_y, ory_x, ory_y;
  bool xp, yp; // parallel sides?

  // AC and BD to get intersection of all "vertical lines"

  il = IntersectLines(ax, ay, cx, cy, bx, by, dx, dy);
  if (il == 0) {
    // parallel sides, store directional vector
    orx_x = cx-ax;
    orx_y = cy-ay;
    xp = true;
  }
  else {
    // store intersection of sides
    orx_x = ix;
    orx_y = iy;
  }
  // AB and CD to get intersection of all "horizontal lines"
  il = IntersectLines(ax, ay, bx, by, cx, cy, dx, dy);
  if (il == 0) {
    // parallel sides, store directional vector
    ory_x = bx-ax;
    ory_y = by-ay;
    yp = true;
  }
  else {
    // store intersection of sides
    ory_x = ix;
    ory_y = iy;
  }

  int xm = int(min4(ax, bx, cx, dx)); // x loop starts here

  y = int(min4(ay, by, cy, dy));
  while (y < h) {
    x = xm;
    while (x < w) {

      // calculate original pixel

      // x:
      if (xp) il = IntersectLines(ax, ay, bx, by, float(x), float(y), float(x)+orx_x, float(y)+orx_y);
      else il = IntersectLines(ax, ay, bx, by, float(x), float(y), orx_x, orx_y);
      fx = float(ow-1)*ua;

      float ux = ua;

      // y:
      if (yp) il = IntersectLines(ax, ay, cx, cy, float(x), float(y), float(x)+ory_x, float(y)+ory_y);
      else il = IntersectLines(ax, ay, cx, cy, float(x), float(y), ory_x, ory_y);
      fy = float(oh-1)*ua;

      // only draw if within original sprite
      if (ux >= 0.0 && ux <= 1.0 && ua >= 0.0 && ua <= 1.0)
	  {
	    int refY=int(clamp(fy,0.0,float(refsrc_height-1)));
		int refX=int(clamp(fx,0.0,float(refsrc_width-1)));

        int setcolor=refsprite_pixels[refY][refX];

		int setY=int(clamp(float(y),0.0,float(src_height-1)));
		int setX=int(clamp(float(x),0.0,float(src_width-1)));

		sprite_pixels[setY][setX]=setcolor;
      }

      x++;
    }

    y++;
  }



  newwidth=w;
  newheight=h;
  engine->ReleaseBitmapSurface(resizeb);

}




void RainUpdate(int rdensity, int FX, int FY,int RW,int RH, int graphic, float perc)
{
  bool drawBack=true;
  bool drawMid=true;
  bool drawFore=true;
  int h=0;

  int cdelay=0;
  while (cdelay <rdensity)
  {
    if (drawMid) CreateRainParticleMid(Random(640*4)-640, -(20+Random(50)), FX, FY,int( (400.0*perc)/100.0));
    if (drawFore) CreateRainParticleFore(Random(640*4)-640, -(20+Random(50)), FX, FY,int( (40.0*perc)/100.0));
    if (drawBack)
	{
		CreateRainParticleBack(Random(640*4)-640, -(20+Random(50)), FX, FY,int( (800.0*perc)/100.0));
		CreateRainParticleBack(Random(640*4)-640, -(20+Random(50)), FX, FY,int( (800.0*perc)/100.0));
	}
    cdelay++;
  }

  BITMAP* src = engine->GetSpriteGraphic(graphic);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);



  int rotAngle=6;
  int rotTrans=60+Random(40+60);//Random(103)+122;
  int rotX=-50;
  int rotY=120;
  int totalTrans=0;

  int maxPart=800;
  if (!drawBack) maxPart=400;
  if (!drawMid) maxPart=400;

  while (h < maxPart)
  {
	  if (h < 400 && drawMid)RainParticles[h].x=RainParticles[h].x-RW;
	  if (h < 400 && drawFore)RainParticlesFore[h].x=RainParticlesFore[h].x-RW;
	  RainParticlesBack[h].x=RainParticlesBack[h].x-RW;

	  h++;
  }


  h=0;
  //BACK
    while (h < maxPart)
    {
      //FORE
    if (h < 400 && drawFore)
    {
      if (RainParticlesFore[h].life>0 && RainParticlesFore[h].active)
      {
        RainParticlesFore[h].life-=4;
        RainParticlesFore[h].translay+=2;
        if (RainParticlesFore[h].translay>RainParticlesFore[h].transhold)
        {
          RainParticlesFore[h].translay=0;
          RainParticlesFore[h].trans+=2;
        }

        int setRainTrans = RainParticlesFore[h].trans+8+Random(10)+totalTrans;
        if (setRainTrans>100)
        {
          setRainTrans=100;
        }

        if (RainParticlesFore[h].y>RH+30
        || RainParticlesFore[h].trans==100)
        {
          RainParticlesFore[h].active=false;
        }
        else
        {
          //int thick =3;
		  //DRAW LINE
		  int alpha = int (float((255*(100-setRainTrans)))/100.0);




		  int x1=RainParticlesFore[h].x;
		  int y1=RainParticlesFore[h].y;
		  int x2=RainParticlesFore[h].x+(RainParticlesFore[h].fx*2);
		  int y2=RainParticlesFore[h].y+(RainParticlesFore[h].fy*2);



		  DrawLineCustom(x1,y1,x2,y2,graphic,255-120,255-120,255-120,alpha-80,6);
		  DrawLineCustom(x1-1,y1,x2-1,y2,graphic,255-120,255-120,255-120,alpha-80,6);


		  DrawLineCustom((x1-rotX),y1-rotY,(x2-rotX)-rotAngle,y2-rotY,graphic,255-120,255-120,255-120,(alpha-80)-rotTrans,6);
		  DrawLineCustom((x1-1)-rotX,y1-rotY,((x2-1)-rotX)-rotAngle,y2-rotY,graphic,255-120,255-120,255-120,(alpha-80)-rotTrans,6);


		  RainParticlesFore[h].x += RainParticlesFore[h].fx;
          RainParticlesFore[h].y += RainParticlesFore[h].fy;
        }
      }
      else
      {
        RainParticlesFore[h].life=0;
        RainParticlesFore[h].active=false;
      }
    }
    //FORE


    //MID
  if (h < drawMid)
  {
    if (RainParticles[h].life>0 && RainParticles[h].active)
    {
      RainParticles[h].life-=4;

      RainParticles[h].translay+=2;
      if (RainParticles[h].translay>RainParticles[h].transhold)
      {
        RainParticles[h].translay=0;
        RainParticles[h].trans+=3;
      }


      int setRainTrans = RainParticles[h].trans+4+Random(5)+totalTrans;
      if (setRainTrans>100)
      {
        setRainTrans=100;
      }

      if (RainParticles[h].y>RH+30
      || RainParticles[h].trans==100)
      {
        RainParticles[h].active=false;
      }
      else
      {
        //int thick=2;
        //DRAW LINE
		int alpha = int (float((255*(100-setRainTrans)))/100.0);

        int x1=RainParticles[h].x;
		int y1=RainParticles[h].y;
		int x2=RainParticles[h].x+RainParticles[h].fx;
		int y2=RainParticles[h].y+RainParticles[h].fy;

		DrawLineCustom(x1,y1,x2,y2,graphic,255-40,255-40,255-40,alpha,6);
		DrawLineCustom(x1-1,y1,x2-1,y2,graphic,255-40,255-40,255-40,alpha,6);

		DrawLineCustom((x1)-rotX,y1-rotY,(x2-rotX)-rotAngle,y2-rotY,graphic,255-40,255-40,255-40,alpha-rotTrans,6);
		DrawLineCustom((x1-1)-rotX,y1-rotY,((x2-1)-rotX)-rotAngle,y2-rotY,graphic,255-40,255-40,255-40,alpha-rotTrans,6);

        RainParticles[h].x += RainParticles[h].fx;
        RainParticles[h].y += RainParticles[h].fy;
      }

    }
    else
    {
      RainParticles[h].life=0;
      RainParticles[h].active=false;
    }
  }
  //MID
   if (h< 800 && drawBack)
   {
      if (RainParticlesBack[h].life>0 && RainParticlesBack[h].active)
      {
        RainParticlesBack[h].life-=4;
        RainParticlesBack[h].translay+=2;
        if (RainParticlesBack[h].translay>RainParticlesBack[h].transhold)
        {
          RainParticlesBack[h].translay=0;
          RainParticlesBack[h].trans++;
        }

        int setRainTrans = RainParticlesBack[h].trans+totalTrans;//+8+Random(10);
        if (setRainTrans>100)
        {
          setRainTrans=100;
        }

        if (RainParticlesBack[h].y>RH+30
        || RainParticlesBack[h].trans==100)
        {
          RainParticlesBack[h].active=false;
        }
        else
        {
          //int thick =1;
          //DRAW LINE
		  int x1=RainParticlesBack[h].x;
		  int y1=RainParticlesBack[h].y;
		  int x2=RainParticlesBack[h].x+RainParticlesBack[h].fx;
		  int y2=RainParticlesBack[h].y+RainParticlesBack[h].fy;

		  int alpha = int (float((255*(100-setRainTrans)))/100.0);
		  DrawLineCustom(x1,y1,x2,y2,graphic,255-80,255-80,255-80,alpha,3);
		  DrawLineCustom((x1-rotX),y1-rotY,(x2-rotX)-rotAngle,y2-rotY,graphic,255-80,255-80,255-80,alpha-rotTrans,3);

          RainParticlesBack[h].x += RainParticlesBack[h].fx;
          RainParticlesBack[h].y += RainParticlesBack[h].fy;
        }
      }
      else
      {
        RainParticlesBack[h].life=0;
        RainParticlesBack[h].active=false;
      }
   }
   h++;
   }
    //BACK

  engine->ReleaseBitmapSurface(src);
}




void ReadWalkBehindIntoSprite(int sprite,int bgsprite,int walkbehindBaseline)
{
	BITMAP* src = engine->GetSpriteGraphic(sprite);
	BITMAP* bgsrc = engine->GetSpriteGraphic(bgsprite);
	int src_width=640;
	int src_height=360;
	int src_depth=32;
	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
	BITMAP* wbh =engine->GetRoomMask(MASK_WALKBEHIND);

	unsigned int** sprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(src);
	unsigned int** bgsprite_pixels = (unsigned int**)engine->GetRawBitmapSurface(bgsrc);
	unsigned char **walk_pixels = engine->GetRawBitmapSurface (wbh); //8bit


	engine->ReleaseBitmapSurface(wbh);
	engine->ReleaseBitmapSurface(bgsrc);



	 //WE GRAB ALL OF THEM INTO A BITMAP and thus we know where they are drawn
	 int x,y;
	 for (y = 0; y < src_height; y++)
	 {
		 for (x = 0; x < src_width; x++)
		 {
			 //READ COLOR

			 if (walk_pixels[y][x]>0)
			 {
				 int grabBaseline=engine->GetWalkbehindBaseline(walk_pixels[y][x]);

				 if (grabBaseline==walkbehindBaseline)
				 {
					 sprite_pixels[y][x] = bgsprite_pixels[y][x];
				 }
			 }
		 }
	 }

	 engine->ReleaseBitmapSurface(src);
}

void Grayscale(int sprite)
{
	BITMAP* src = engine->GetSpriteGraphic(sprite);
	unsigned int** pixels = (unsigned int**)engine->GetRawBitmapSurface(src);

	int src_width=640;
	int src_height=360;
	int src_depth=32;

	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

	int x,y;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int color = ConvertColorToGrayScale(pixels[y][x]);
			pixels[y][x]= color;
		}
	}


	engine->ReleaseBitmapSurface(src);


}

void DrawBlur(int spriteD, int radius)
{
	int spriteD2=spriteD;
	BITMAP* src = engine->GetSpriteGraphic(spriteD);
	BITMAP* src2 = engine->GetSpriteGraphic(spriteD2);

	unsigned int** pixelb = (unsigned int**)engine->GetRawBitmapSurface(src);
	unsigned int** pixela = (unsigned int**)engine->GetRawBitmapSurface(src2);
	engine->ReleaseBitmapSurface(src2);
	int src_width=640;
	int src_height=360;
	int src_depth=32;

	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);


    int x,y;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int totalRed=0;
			int totalGreen=0;
			int totalBlue=0;

			int vx=-(radius);
			int pixels_parsed=0;

			int setY=y;
			if (setY<0) setY=0;
			if (setY>src_height-1) setY=src_height-1;

			while (vx < (radius) +1)
			{
				int setX=x+vx;
				if (setX<0) setX=0;
				if (setX>src_width-1) setX=src_width-1;


				int color = pixela[setY][setX];

				totalRed+=getRcolor(color);
				totalGreen+=getGcolor(color);
				totalBlue+=getBcolor(color);

				pixels_parsed++;
				vx++;
			}

			int rN=totalRed/pixels_parsed;
			int gN=totalGreen/pixels_parsed;
			int bN=totalBlue/pixels_parsed;

			int r=int(clamp(rN,0.0,255.0));
			int g=int(clamp(gN,0.0,255.0));
			int b=int(clamp(bN,0.0,255.0));


			pixelb[y][x]= ((r << 16) | (g << 8) | (b << 0) | (255 << 24));



		}
	}


	engine->ReleaseBitmapSurface(src);
	src = engine->GetSpriteGraphic(spriteD);

	x=0;
	y=0;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int totalRed=0;
			int totalGreen=0;
			int totalBlue=0;

			int pixels_parsed=0;
			int setX=x;
			if (setX<0) setX=0;
			if (setX>src_width-1) setX=src_width-1;

			int vy=-(radius);
			while (vy <(radius)+1)
			{
				int setY=y+vy;
				if (setY<0) setY=0;
				if (setY>src_height-1) setY=src_height-1;

				int color = pixela[setY][setX];

				totalRed+=getRcolor(color);
				totalGreen+=getGcolor(color);
				totalBlue+=getBcolor(color);

				pixels_parsed++;

			    vy++;
			}

			int rN=totalRed/pixels_parsed;
			int gN=totalGreen/pixels_parsed;
			int bN=totalBlue/pixels_parsed;

			int r=int(clamp(rN,0.0,255.0));
			int g=int(clamp(gN,0.0,255.0));
			int b=int(clamp(bN,0.0,255.0));


			pixelb[y][x]= ((r << 16) | (g << 8) | (b << 0) | (255 << 24));



		}
	}

	engine->ReleaseBitmapSurface(src);

}





float fracts(float value)
{
	return value-floor(value);
}

float lerp(float x, float y, float fn)
{
    return x*(1.0-fn)+y*fn;
}


float hasher( float n ) 
{ 
	return fracts(sin(n)*153.5453123);
}
float noiseField( float tx,float ty,float tz)
{
    float px = floor(tx);
    float fx = fracts(tx);
    float py = floor(ty);
    float fy = fracts(ty);
    float pz = floor(tz);
    float fz = fracts(tz);
    fx = fx*fx*(3.0-2.0*fx);
	fy = fy*fy*(3.0-2.0*fy);
	fz = fz*fz*(3.0-2.0*fz);
	
    float n = px + py*157.0 + 113.0*pz;
    return lerp(lerp(lerp( hasher(n+  0.0), hasher(n+  1.0),fx),
                   lerp( hasher(n+157.0), hasher(n+158.0),fx),fy),
               lerp(lerp( hasher(n+113.0), hasher(n+114.0),fx),
                   lerp( hasher(n+270.0), hasher(n+271.0),fx),fy),fz);
}



void DrawForceField(int spriteD, int scale, float speed,int id)
{
	if (id <0 || id >4)
        {
          return;
        }
        if (b_time[id] == NULL) b_time[id]=1.0;
	if (b_time[id]<1.0) b_time[id]=1.0;
	b_time[id]+=speed;
	BITMAP* src = engine->GetSpriteGraphic(spriteD);
	
	unsigned int** pixelb = (unsigned int**)engine->GetRawBitmapSurface(src);
	
	int src_width=640;
	int src_height=360;
	int src_depth=32;

	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
	

    int x,y;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int setY=y;
			if (setY<0) setY=0;
			int setX=x;
			if (setX<0) setX=0;
			
			float uvx=float(x)/float(scale);
			float uvy=float(y)/float(scale);

			float jx=uvx;
			float jy=uvy+b_time[id]*3.14;
			float jz=sin(b_time[id]);
			float jyy=uvy+b_time[id];
			float jzz=cos(b_time[id]+3.0);

			float af = abs(noiseField(jx,jy,jz)-noiseField(jx,jyy,jzz));
			float newR=0.5-pow(af, float(0.2))/2.0;
			float newG=0.0;
			float newB=0.4-pow(af, float(0.4));

			int Rd=int(newR*255.0);
			int Gd=int(newG*255.0);
			int Bd=int(newB*255.0);
			int na=int(1.0*255.0);//pixelb[setY][setX];//int(1.0*255.0);

			int highest=0;
			if (Rd > Gd)
			{
				if (Rd > Bd) highest=Rd;
				else highest=Bd;
			}
			else 
			{
				if (Gd > Bd) highest=Gd;
				else highest=Bd;
			}

			int grabA=getAcolor(pixelb[setY][setX]);

			if (highest <= 40)
			{
				na=int( (float(highest*2)/100.0)*255.0);
			}
			else 
			{
				na=grabA;
			}
			pixelb[setY][setX]= SetColorRGBA(Rd,Gd,Bd,na);//
			
			
		}
	}
    

	engine->ReleaseBitmapSurface(src);

}




void DrawCylinder(int spriteD, int ogsprite)
{
	BITMAP* src = engine->GetSpriteGraphic(spriteD);
	unsigned int** pixela = (unsigned int**)engine->GetRawBitmapSurface(src);
	int src_width=640;
	int src_height=640;
	int src_depth=32;
    engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

	BITMAP* src2 = engine->GetSpriteGraphic(ogsprite);
	unsigned int** pixelb = (unsigned int**)engine->GetRawBitmapSurface(src2);
	engine->ReleaseBitmapSurface(src2);
    int height=src_height;
	int width=src_width;

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			//convertPoint(x,y,width,height);

			//center the point at 0,0
			float pcx=x-width/2;
			float pcy=y-height/2;
			
			//these are your free parameters
			float f = width/2;
			float r = width;
			
			float omega = width/2;
			float z0 = f - sqrt(r*r-omega*omega);			
			float zc = (2*z0+sqrt(4*z0*z0-4*(pcx*pcx/(f*f)+1)*(z0*z0-r*r)))/(2* (pcx*pcx/(f*f)+1)); 
			
			float finalpointx=pcx*zc/f;
			float finalpointy=pcy*zc/f;
			finalpointx += width/2;
			finalpointy += height/2;


			int cposx=finalpointx;
			int cposy=finalpointy;
			if(cposx < 0 ||
				cposx > width-1 ||
				cposy < 0 ||
				cposy > height-1)
			{
				pixela[y][x]=SetColorRGBA(0,0,0,0);
			}
			else 
			{
				pixela[y][x] = pixelb[cposy][cposx];
			}
		}
	}
	
    engine->ReleaseBitmapSurface(src);
}






void DrawTunnel(int spriteD, float scale, float speed)
{
	d_time=speed;
    BITMAP* src = engine->GetSpriteGraphic(spriteD);
	unsigned int** pixela = (unsigned int**)engine->GetRawBitmapSurface(src);
	int src_width=640;
	int src_height=360;
	int src_depth=32;
    engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

    BITMAP* src2 = engine->GetSpriteGraphic(int(scale));
	unsigned int** pixelb = (unsigned int**)engine->GetRawBitmapSurface(src2);
int h=screenHeight;
  int w=screenWidth;
	if (!generateonce)
	{
		generateonce=true;
	//generate texture
  for(int y = 0; y < texHeight; y++)
  {
  for(int x = 0; x < texWidth; x++)
  {
    texture[y][x] = pixelb[y][x];
  }
  }
  
  //generate non-linear transformation table
  for(int y = 0; y < h; y++)
  {
  for(int x = 0; x < w; x++)
  {
    int angle, distance;
    float ratio = 32.0;
    distance = int(ratio * texHeight / sqrt((x - w / 2.0) * (x - w / 2.0) + (y - h / 2.0) * (y - h / 2.0))) % texHeight;
    angle = (unsigned int)(0.5 * texWidth * atan2(y - h / 2.0, x - w / 2.0) / 3.1416);
    distanceTable[y][x] = distance;///4.0;
    angleTable[y][x] = angle;
  }
  }
	}

	int shiftX = int(texWidth * 0.75 * d_time);
    int shiftY = int(texHeight * 1.0 * d_time);

    for(int y = 0; y < h; y++)
	{
    for(int x = 0; x < w; x++)
    {
      //get the texel from the texture by using the tables, shifted with the animation values
      int color = texture[(unsigned int)(distanceTable[y][x] + shiftX)  % texWidth][(unsigned int)(angleTable[y][x] + shiftY) % texHeight];

	  pixela[y][x]=color;
    }
	}
    

	

    engine->ReleaseBitmapSurface(src2);
    engine->ReleaseBitmapSurface(src);
}




bool IsPixelTransparent(int color)
{
	int rd=getRcolor(color);
	int gd=getGcolor(color);
	int bd=getBcolor(color);
	int ad=getAcolor(color);


	if (ad < 255)
	{
		return true;
	}
	else return false;
}


void Outline(int sprite,int red,int ged,int bed,int aed)
{
	BITMAP* src = engine->GetSpriteGraphic(sprite);
	unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);

	int src_width=640;
	int src_height=360;
	int src_depth=32;
	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

	//OUTLINE
	engine->ReleaseBitmapSurface(src);


	BITMAP*dst = engine->GetSpriteGraphic(sprite);
	unsigned int** pixel_dst = (unsigned int**)engine->GetRawBitmapSurface(dst);

	int x,y;
	for (x = 0; x < src_width; x++)
	{
		//y=0;
		for (y = 0; y < src_height; y++)
		{
			if (!IsPixelTransparent(pixel_src[y][x]))
			{
			}
			else
			{
				int pcount=0;
				int gy=-1;
				while (gy < 2)
				{
					int gx=-1;
					while (gx < 2)
					{
						int sx=x+gx;
						int sy=y+gy;

						if (sx <0) sx=0;
						if (sy <0) sy=0;
						if (sx > src_width-1) sx=src_width-1;
						if (sy > src_height-1) sy=src_height-1;

						if (!IsPixelTransparent(pixel_src[sy][sx]))
						{
							pcount++;
						}

						gx++;
					}
					gy++;
				}

				if (pcount >=2)
				{
					int colorLeft=SetColorRGBA(red,ged,bed,aed);
					pixel_dst[y][x]=colorLeft;
				}


			}
		}

	}


	//OUTLINE
	engine->ReleaseBitmapSurface(dst);
}




void OutlineOnly(int sprite,int refsprite, int red,int ged,int bed,int aed, int trans)
{
	BITMAP* src = engine->GetSpriteGraphic(refsprite);
	unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);

	int src_width=640;
	int src_height=360;
	int src_depth=32;
	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

	//OUTLINE
	engine->ReleaseBitmapSurface(src);


	BITMAP*dst = engine->GetSpriteGraphic(sprite);
	unsigned int** pixel_dst = (unsigned int**)engine->GetRawBitmapSurface(dst);

	int x,y;
	for (x = 0; x < src_width; x++)
	{
		//y=0;
		for (y = 0; y < src_height; y++)
		{
			if (!IsPixelTransparent(pixel_src[y][x]))
			{
				int colorLeft=SetColorRGBA(red,ged,bed,trans);
				pixel_dst[y][x]=colorLeft;
			}
			else
			{
				int pcount=0;
				int gy=-1;
				while (gy < 2)
				{
					int gx=-1;
					while (gx < 2)
					{
						int sx=x+gx;
						int sy=y+gy;

						if (sx <0) sx=0;
						if (sy <0) sy=0;
						if (sx > src_width-1) sx=src_width-1;
						if (sy > src_height-1) sy=src_height-1;

						if (!IsPixelTransparent(pixel_src[sy][sx]))
						{
							pcount++;
						}

						gx++;
					}
					gy++;
				}

				if (pcount >=2)
				{
					int colorLeft=SetColorRGBA(red,ged,bed,aed);
					pixel_dst[y][x]=colorLeft;
				}
			}
		}

	}


	//OUTLINE
	engine->ReleaseBitmapSurface(dst);
}





void CreateDustParticle(int xx, int yy)
{
  int h=0;
  bool founddust=false;
  int fid=-1;
  while (h <= dsizeDust && !founddust)
  {
    if (dusts[h].active==false)
    {
      founddust=true;
      fid=h;
    }
    h++;
  }

  if (founddust)
  {
    int d=fid;
    dusts[d].x=xx;
    dusts[d].y=yy;
    dusts[d].dx=(-1)+Random(1);
    dusts[d].dy=(-1)+Random(1);
    dusts[d].life=20000;
    dusts[d].transp=55+Random(10);
    dusts[d].active=true;
    dusts[d].mlay=4+Random(2);
    dusts[d].timlay=0;
    dusts[d].translay=0;
    dusts[d].translayHold=19+Random(15);
    if (dsizeDust<(raysizeDust-1)) dsizeDust++;
  }
}


int GetWalkbehindBaserine(int id)
{
  return Walkbehind[id];
}

void SetWalkbehindBaserine(int id,int base)
{
  Walkbehind[id]=base;
}

void FireUpdate(int getDynamicSprite, bool Fire2Visible)
{


  BITMAP* src = engine->GetSpriteGraphic(getDynamicSprite);
  unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);
  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

	//OUTLINE



  creationdelay+=int(2.0);
  if (creationdelay > 4 && Fire2Visible)
  {


    int by=0;
    while (by <6)
    {
    int dnx=95+(Random(535-95));
    int dny=Random(236);

	int (*sfGetRegionXY)(int,int);
	sfGetRegionXY = ((int(*)(int,int)) engine->GetScriptFunctionAddress("GetRegionAt"));
	int getID=sfGetRegionXY(dnx,dny);


    while (getID!=10)
    {
      dnx=95+(Random(535-95));
      dny=Random(236);
	  getID=sfGetRegionXY(dnx,dny);
    }
    CreateDustParticle(dnx, dny);
    by++;
    }

    creationdelay=0;
  }
  int h=dsizeDust-1;
  while (h>0)
  {
    if (dusts[h].life>0)
    {
      dusts[h].life-=int(2.0);

      int setX=dusts[h].x;
	  int setY=dusts[h].y;

	  if (setX <0) setX=0;
	  if (setX > src_width) setX=src_width;

	  if (setY <0) setY=0;
	  if (setY > src_height) setY=src_height;

	  int Rf=Random(100);
	  int rv,gv,bv,av;

	  if (Rf<50) {rv=255;gv=128;bv=0;}
	  else {rv=231;gv=71;bv=24;}

	  av =int((float(255*(150-dusts[h].transp)))/100.0);


	  pixel_src[setY][setX]=SetColorRGBA(rv,gv,bv,av);

      //drawt.DrawImage(dusts[h].x, dusts[h].y, sg, dusts[h].transp);
      dusts[h].timlay+=int(8.0);
      if (dusts[h].timlay> dusts[h].mlay)
      {
        dusts[h].timlay=0;
        dusts[h].x += dusts[h].dx+Random(1);
        dusts[h].y += dusts[h].dy-(Random(1));
      }
      dusts[h].translay+=2;
      if (dusts[h].translay>=dusts[h].translayHold)
      {
        if (dusts[h].transp<=99) dusts[h].transp++;
        else dusts[h].life=0;
      }
    }
    else
    {
      dusts[h].active=false;
    }
    h--;
  }



  engine->ReleaseBitmapSurface(src); 

}

void TintProper(int sprite,int lightx,int lighty, int radi,int rex,int grx,int blx)
{


	BITMAP* src = engine->GetSpriteGraphic(sprite);
	BITMAP* src2 = engine->GetSpriteGraphic(lightx);

	unsigned int** pixelb = (unsigned int**)engine->GetRawBitmapSurface(src);
	unsigned int** pixela = (unsigned int**)engine->GetRawBitmapSurface(src2);
	engine->ReleaseBitmapSurface(src2);
	int src_width=640;
	int src_height=360;
	int src_depth=32;

	engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);


    int x,y;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int totalRed=0;
			int totalGreen=0;
			int totalBlue=0;

			int vx=-(radi);
			int pixels_parsed=0;

			int setY=y;
			if (setY<0) setY=0;
			if (setY>src_height-1) setY=src_height-1;

			while (vx < (radi) +1)
			{
				int setX=x+vx;
				if (setX<0) setX=0;
				if (setX>src_width-1) setX=src_width-1;


				int color = pixela[setY][setX];


				totalRed+=getRcolor(color);
				totalGreen+=getGcolor(color);
				totalBlue+=getBcolor(color);

				pixels_parsed++;

				vx++;
			}

			int rN=totalRed/pixels_parsed;
			int gN=totalGreen/pixels_parsed;
			int bN=totalBlue/pixels_parsed;

			int r=int(clamp(rN,0.0,255.0));
			int g=int(clamp(gN,0.0,255.0));
			int b=int(clamp(bN,0.0,255.0));

			if (r > rex && g>grx && b>blx)
			{


			pixelb[y][x]= ((r << 16) | (g << 8) | (b << 0) | (255 << 24));

			}
			else pixelb[y][x]=SetColorRGBA(rex,grx,blx,0);



		}
	}


	engine->ReleaseBitmapSurface(src);
	src = engine->GetSpriteGraphic(sprite);

	x=0;
	y=0;
	for (y = 0; y < src_height; y++)
	{
		for (x = 0; x < src_width; x++)
		{
			int totalRed=0;
			int totalGreen=0;
			int totalBlue=0;

			int pixels_parsed=0;
			int setX=x;
			if (setX<0) setX=0;
			if (setX>src_width-1) setX=src_width-1;

			int vy=-(radi);
			while (vy <(radi)+1)
			{
				int setY=y+vy;
				if (setY<0) setY=0;
				if (setY>src_height-1) setY=src_height-1;

				int color = pixela[setY][setX];

				totalRed+=getRcolor(color);
				totalGreen+=getGcolor(color);
				totalBlue+=getBcolor(color);

				pixels_parsed++;


			    vy++;
			}

			int rN=totalRed/pixels_parsed;
			int gN=totalGreen/pixels_parsed;
			int bN=totalBlue/pixels_parsed;

			int r=int(clamp(rN,0.0,255.0));
			int g=int(clamp(gN,0.0,255.0));
			int b=int(clamp(bN,0.0,255.0));

			if (r > rex && g>grx && b>blx)
			{
				pixelb[y][x]= ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
			}
			else pixelb[y][x]=SetColorRGBA(rex,grx,blx,0);



		}
	}

	engine->ReleaseBitmapSurface(src);


}


void AdjustSpriteFont(int sprite,int rate,int outlineRed,int outlineGreen,int outlineBlue)
{


  BITMAP* src = engine->GetSpriteGraphic(sprite);
  unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);

  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

  int x, y;

  int px=1;
  bool found=false;
  for (y = 0; y < src_height; y++)
  {
	  if (found)
	  {
		  px++;
	  }
	//if (px >12) px=12;
	  bool havefound=false;
    for (x = 0; x < src_width; x++)//
	{
		int getColor=pixel_src[y][x];
		int red=getRcolor(getColor);
		int green=getGcolor(getColor);
		int blue=getBcolor(getColor);
		int alpha=getAcolor(getColor);

		if (alpha < 255.0 || (red <=10 && green<=10 && blue<=10))
		{
			//px=1;
			if (alpha == 255 && (red <=10 && green<=10 && blue<=10))
			{
				pixel_src[y][x]=SetColorRGBA(outlineRed,outlineGreen,outlineBlue,255);
			}
		}
		else
		{
			havefound=true;
			found=true;
			red-=(px*rate);
			green-=(px*rate);
			blue-=(px*rate);



			pixel_src[y][x]=SetColorRGBA(red,green,blue,255);
		}
	}

	if (havefound==false)
	{
		if (found)
		{
			px=1;
			found=false;
		}
	}
  }

  engine->ReleaseBitmapSurface(src);
}


void SpriteGradient(int sprite,int rate,int toy)
{


  BITMAP* src = engine->GetSpriteGraphic(sprite);
  unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);

  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);

  int x, y;
  int setA=0;

  for (y = toy; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)
	{
		int getColor=pixel_src[y][x];
		int red=getRcolor(getColor);
		int green=getGcolor(getColor);
		int blue=getBcolor(getColor);
		int alpha=getAcolor(getColor)+setA;
		if (alpha>250) alpha=250;

		if (red >10 && green>10 && blue > 10)
		{
		pixel_src[y][x]=SetColorRGBA(red,green,blue,alpha);
		}

	}
	setA+=rate;
  }

  engine->ReleaseBitmapSurface(src);
}



void SpriteSkew(int sprite,float xskewmin, float yskewmin,float xskewmax, float yskewmax)
{


  BITMAP* src = engine->GetSpriteGraphic(sprite);
  unsigned int** pixel_src = (unsigned int**)engine->GetRawBitmapSurface(src);

  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src,&src_width,&src_height,&src_depth);
  engine->ReleaseBitmapSurface(src);

  BITMAP* dest = engine->GetSpriteGraphic(sprite);
  unsigned int** pixel_dest = (unsigned int**)engine->GetRawBitmapSurface(dest);

  int x, y;

  float raty = abs(yskewmin-yskewmax)/float(src_height*src_width);
  float ratx = abs(xskewmin-xskewmax)/float(src_height*src_width);
  float yskew=yskewmin;
  float xskew=xskewmin;

  for (y = 0; y < src_height; y++)
  {
    for (x = 0; x < src_width; x++)
	{
		int ry=int((float(x)*yskew) + float(y));
		int rx=int(float(x) + (float(y)*xskew));

		if (ry > src_height-1) ry=src_height-1;
		if (rx > src_width-1) rx=src_width-1;
		if (rx <0) rx=0;
		if (ry <0) ry=0;

		int getColor=pixel_src[ry][rx];
		int red=getRcolor(getColor);
		int green=getGcolor(getColor);
		int blue=getBcolor(getColor);
		int alpha=getAcolor(getColor);



        pixel_dest[y][x] = SetColorRGBA(red,green,blue,alpha);

		if (xskewmin<xskewmax) xskew+=ratx;
		else xskew-=ratx;

		if (yskewmin<yskewmax) yskew+=raty;
		else yskew-=raty;
	}
  }

  engine->ReleaseBitmapSurface(dest);
}


void DrawEffect(int sprite_a,int sprite_b,int id,int n)
{
  int x, y;

  BITMAP* src_a = engine->GetSpriteGraphic(sprite_a);
  BITMAP* src_b = engine->GetSpriteGraphic(sprite_b);

  unsigned int** pixel_a = (unsigned int**)engine->GetRawBitmapSurface(src_a);
  unsigned int** pixel_b = (unsigned int**)engine->GetRawBitmapSurface(src_b);

  int src_width=640;
  int src_height=360;
  int src_depth=32;
  engine->GetBitmapDimensions(src_a,&src_width,&src_height,&src_depth);



  for (y = 0; y < src_height; y++)
  {
	if (id==1) CastWave(15,1,n);
	if (id==0 || id==9||id==2 || id==3 || id==6 || id==8) CastWave(2,1,n);
	if (id==4) CastWave(15,4,n);
	if (id==5 || id==7 || id==10)
	{
		//x=0;
		CastWave(3,1,n);
	}
	if (id==11)
	{
		//x=0;
		CastWave(3,2,n);
	}
	if (id==16) CastWave(4,1,n);
	if (id==17) CastWave(6,1,n);


    for (x = 0; x < src_width; x++)
    {
	  unsigned int colorfromB = pixel_b[y][x];
	  int getX=x;
	  int getY=y;

	  if (id==0)
	  {
	  	  getX=x-(rand() % 2)-2;
		  getY=y+dY[n];
	  }
	  if (id==1 || id==4)
	  {
	  	  getX=x;
		  getY=y+dY[n];
	  }
	  if (id==2)
	  {
	  	  getX=x+dY[n];
		  getY=y-(rand() % 2)-2;
	  }
	  if (id==3)
	  {
	  	  getX=x;
		  getY=y-(rand() % 2)-2;
	  }
	  if (id==5)
	  {
		  getX=x+dY[n];
		  getY=y-(rand() % 2)-2;
	  }
	  if (id==6 || id==16)
	  {
	  	  getX=x+dY[n];
		  getY=y-(rand() % 1)-1;
	  }
	  if (id==7 || id==17)
	  {
		  getX=x+dY[n];
		  getY=y-(rand() % 1)-1;
	  }
	  if (id==8)
	  {
	  	  getX=x+dY[n];
		  getY=y+(rand() % 2)-2;
	  }
	  if (id==10 || id==9 || id==11)
	  {
		  getX=x+dY[n];
		  getY=y;
	  }

	  if (getX < 0) getX=0;
	  if (getX > src_width-1) getX = src_width-1;
	  if (getY > src_height-1) getY = src_height-1;
	  if (getY < 0) getY = 0;


	  pixel_a[getY][getX]=colorfromB;	  //
    }
  }

  engine->ReleaseBitmapSurface(src_a);
  engine->ReleaseBitmapSurface(src_b);

}






void SaveVariable(char*value,int id)
{
	if (GameDatavalue[id] != NULL)
	{
		free(GameDatavalue[id]);
	}
	if (value != NULL)
	{
		GameDatavalue[id]=strdup(value);
	}
	else
	{
		GameDatavalue[id]=NULL;
	}
}

const char* ReadVariable(int id)
{

	if (GameDatavalue[id]==NULL)
	{
		return engine->CreateScriptString("");
	}
	else
	{
		return engine->CreateScriptString(GameDatavalue[id]);
	}
}




void SetGDState(char*value,bool setvalue)
{
	int id=-1;
	for (int i = 0; i <= usedTokens; i++)
	{
		if (Token[i]!=NULL && strcmp(Token[i], value) == 0)
		{
			id=i;
			TokenUnUsed[i]=setvalue;
			i=usedTokens+1;
		}
	}
	if (id==-1)
	{
		//it doesn't find it while trying to set its state
		//create the thing with said state
		id=usedTokens;
		TokenUnUsed[id]=setvalue;
		if (Token[id]!=NULL)free(Token[id]);
		Token[id]=strdup(value);
		usedTokens++;

	}
}

bool GetGDState(char*value)
{
	int id=-1;

	for (int i = 0; i <= usedTokens; i++)
	{
		if (Token[i]!=NULL && strcmp(Token[i], value) == 0)
		{
			id=i;
			i=usedTokens+1;
		}
	}

	if (id==-1)
	{
		return true;
	}
	else
	{
		return TokenUnUsed[id];
	}
}


void ResetAllGD()
{
	for (int i = 0; i <= usedTokens; i++)
	{
		if (Token[i]!=NULL)free(Token[i]);
		Token[i]=NULL;
		TokenUnUsed[i]=true;
	}
	usedTokens=0;
}

int GameDoOnceOnly(char*value)
{
	if (GetGDState(value)==true)
	{
		//set state to false
		SetGDState(value,false);
		return true;
	}
	else
	{
		return false;
	}
}



// ********************************************
// ************  AGS Interface  ***************
// ********************************************


void DrawScreenEffect(int sprite,int sprite_prev,int ide,int n)
{
	DrawEffect(sprite,sprite_prev,ide,n);
}



void Music_Play(int MFX, int repeat,int fadeinMS,int fadeoutMS,int Position,bool fixclick)
{
	MusicPlay(MFX,repeat,fadeinMS,fadeoutMS,Position,false,fixclick);
}

void SFX_Play(int SFX, int repeat)
{
	PlaySFX(SFX,repeat);
}

void SFX_Stop(int SFX,int fademsOUT)
{
	SFXStop(SFX,fademsOUT);
}
void SFX_SetPosition(int SFX,int x,int y,int intensity)
{
	SFXSetPosition(SFX,x,y,intensity);
}

void SFX_SetVolume(int SFX,int volume)
{
	SFXSetVolume(SFX,volume);
}

int SFX_GetVolume(int SFX)
{
	return SFXGetVolume(SFX);
}
void SFX_AllowOverlap(int SFX,int allow)
{
	SFXAllowOverlap(SFX,allow);
}
void SFX_Filter(int SFX,int enable)
{
	SFXFilter(SFX,enable);
}


void Music_SetVolume(int volume)
{
	MusicSetVolume(volume);
}

int Music_GetVolume()
{
	return MusicGetVolume();
}

void SFX_SetGlobalVolume(int volume)
{
	SFXSetGlobalVolume(volume);
}



void Unload_SFX(int SFX)
{
	UnloadSFX(SFX);
}

void Load_SFX(int SFX)
{
	LoadSFX(SFX);
}

void Audio_Apply_Filter(int Frequency)
{
	GlitchFix();
	ApplyFilter(Frequency);
}
void Audio_Remove_Filter()
{
	RemoveFilter();
}




void AGS_EngineStartup(IAGSEngine *lpEngine)
{
  engine = lpEngine;

  if (engine->version < 13)
    engine->AbortGame("Engine interface is too old, need newer version of AGS.");

   StartingValues();

  Character_GetX = (SCAPI_CHARACTER_GETX)engine->GetScriptFunctionAddress("Character::get_X");
  Character_GetY = (SCAPI_CHARACTER_GETY)engine->GetScriptFunctionAddress("Character::get_Y");
  Character_ID = (SCAPI_CHARACTER_ID)engine->GetScriptFunctionAddress("Character::ID");


  SDLMain();
  engine->RegisterScriptFunction("DrawScreenEffect", (void*)&DrawScreenEffect);
  engine->RegisterScriptFunction("SFX_Play",(void*)&SFX_Play); 
  engine->RegisterScriptFunction("SFX_SetVolume",(void*)&SFX_SetVolume);
  engine->RegisterScriptFunction("SFX_GetVolume",(void*)&SFX_GetVolume);
  engine->RegisterScriptFunction("Music_Play",(void*)&Music_Play);
  engine->RegisterScriptFunction("Music_GetVolume",(void*)&Music_GetVolume);
  engine->RegisterScriptFunction("Music_SetVolume",(void*)&Music_SetVolume);
  engine->RegisterScriptFunction("SFX_Stop",(void*)&SFX_Stop);
  engine->RegisterScriptFunction("SFX_SetPosition",(void*)&SFX_SetPosition);  
  engine->RegisterScriptFunction("SFX_SetGlobalVolume",(void*)&SFX_SetGlobalVolume);
  engine->RegisterScriptFunction("Load_SFX",(void*)&Load_SFX);
  engine->RegisterScriptFunction("Audio_Apply_Filter",(void*)&Audio_Apply_Filter);
  engine->RegisterScriptFunction("Audio_Remove_Filter",(void*)&Audio_Remove_Filter);
  engine->RegisterScriptFunction("SFX_AllowOverlap",(void*)&SFX_AllowOverlap);
  engine->RegisterScriptFunction("SFX_Filter",(void*)&SFX_Filter);
  engine->RegisterScriptFunction("DrawBlur",(void*)&DrawBlur);
  engine->RegisterScriptFunction("DrawTunnel",(void*)&DrawTunnel);
  engine->RegisterScriptFunction("DrawCylinder",(void*)&DrawCylinder);
  engine->RegisterScriptFunction("DrawForceField",(void*)&DrawForceField);
  engine->RegisterScriptFunction("Grayscale",(void*)&Grayscale);
  engine->RegisterScriptFunction("ReadWalkBehindIntoSprite",(void*)&ReadWalkBehindIntoSprite);
  engine->RegisterScriptFunction("AdjustSpriteFont",(void*)&AdjustSpriteFont);
  engine->RegisterScriptFunction("SpriteGradient",(void*)&SpriteGradient);
  engine->RegisterScriptFunction("Outline",(void*)&Outline);
  engine->RegisterScriptFunction("OutlineOnly",(void*)&OutlineOnly);
  engine->RegisterScriptFunction("SaveVariable",(void*)&SaveVariable);
  engine->RegisterScriptFunction("ReadVariable",(void*)&ReadVariable);
  engine->RegisterScriptFunction("GameDoOnceOnly",(void*)&GameDoOnceOnly);
  engine->RegisterScriptFunction("SetGDState",(void*)&SetGDState);
  engine->RegisterScriptFunction("GetGDState",(void*)&GetGDState);
  engine->RegisterScriptFunction("ResetAllGD",(void*)&ResetAllGD);
  engine->RegisterScriptFunction("SpriteSkew",(void*)&SpriteSkew);
  engine->RegisterScriptFunction("FireUpdate",(void*)&FireUpdate);
  engine->RegisterScriptFunction("WindUpdate",(void*)&WindUpdate);
  engine->RegisterScriptFunction("SetWindValues",(void*)&SetWindValues);
  engine->RegisterScriptFunction("ReturnWidth",(void*)&ReturnWidth);
  engine->RegisterScriptFunction("ReturnHeight",(void*)&ReturnHeight);
  engine->RegisterScriptFunction("ReturnNewHeight",(void*)&ReturnNewHeight);
  engine->RegisterScriptFunction("ReturnNewWidth",(void*)&ReturnNewWidth);
  engine->RegisterScriptFunction("Warper",(void*)&Warper);
  engine->RegisterScriptFunction("SetWarper",(void*)&SetWarper);
  engine->RegisterScriptFunction("RainUpdate",(void*)&RainUpdate);
  engine->RegisterScriptFunction("BlendTwoSprites",(void*)&BlendTwoSprites);
  engine->RegisterScriptFunction("Blend",(void*)&Blend);
  engine->RegisterScriptFunction("Dissolve",(void*)&Dissolve);
  engine->RegisterScriptFunction("ReverseTransparency",(void*)&ReverseTransparency);
  engine->RegisterScriptFunction("NoiseCreator",(void*)&NoiseCreator);
  engine->RegisterScriptFunction("TintProper",(void*)&TintProper);
  engine->RegisterScriptFunction("GetWalkbehindBaserine",(void*)&GetWalkbehindBaserine);
  engine->RegisterScriptFunction("SetWalkbehindBaserine",(void*)&SetWalkbehindBaserine);



  engine->RequestEventHook(AGSE_PREGUIDRAW);
  engine->RequestEventHook(AGSE_PRESCREENDRAW);
  engine->RequestEventHook(AGSE_SAVEGAME);
  engine->RequestEventHook(AGSE_RESTOREGAME);
  engine->RequestEventHook(AGSE_ENTERROOM);

  
  int j=0;
  while (j < 80)
  {
	  GetMusicPath(MusicLoads[j].musicPath,j);
	  char musicPath[1024];
	  GetMusicPath(musicPath,j);
	  musiceffect[j]=Mix_LoadMUS(musicPath);
	  j++;
  }

}


void AGS_EngineShutdown()
{
	//backmusic=null;
	//soundeffect=null;
	//Mix_FreeChunk(soundeffect[0]);
	//Mix_FreeMusic(backmusic);
	//GeneralAudio.Disabled=true;
	//AudioPause();
	/*
    int j=0;
	while (j < 300-1)
	{
		if (SFX[j].chunk!=NULL)
		{
			Mix_FreeChunk(SFX[j].chunk);
		}
		if (j<40 && musiceffect[j]!=NULL)
		{
			Mix_FreeMusic(musiceffect[j]);
		}
		j++;
	}*/


	Mix_Quit();
	SDL_Quit();
}

int AGS_EngineOnEvent(int event, int data)
{
  if (event == AGSE_PREGUIDRAW)
  {
	 Update();
  }
  else if (event == AGSE_RESTOREGAME)
  {
	int i=0;
	while (i < GeneralAudio.NumOfChannels)
	{
		Mix_HaltChannel(i);
		i++;
	}

	int j=0;
	while (j <500-1)
	{
		engine->FRead(&SFX[j].repeat,sizeof(int),data);
		engine->FRead(&SFX[j].volume,sizeof(int),data);
		engine->FRead(&SFX[j].playing,sizeof(int),data);
		j++;
	}

  }
  else if (event == AGSE_SAVEGAME)
  {
	  int j=0;
	  while (j <500-1)
	  {
		  engine->FWrite(&SFX[j].repeat,sizeof(int),data);
		  engine->FWrite(&SFX[j].volume,sizeof(int),data);
		  engine->FWrite(&SFX[j].playing,sizeof(int),data);
		  j++;
	  }
  }
  else if (event == AGSE_PRESCREENDRAW)
  {

    // Get screen size once here.
    engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);

	//engine->UnrequestEventHook(AGSE_SAVEGAME);
	//engine->UnrequestEventHook(AGSE_RESTOREGAME);

  }
  else if (event ==AGSE_ENTERROOM)
  {
	  
	int j=0;
	while (j < 500-1)
	{
		int i=0;
		while (i < GeneralAudio.NumOfChannels)
		{
			if (!Mix_Playing(i) && Mix_GetChunk(i)!=NULL && SFX[j].chunk!=NULL && Mix_GetChunk(i)==SFX[j].chunk && SFX[j].playing==0
				&& SFX[j].repeat==0)
			{
				UnloadSFX(j);
			}
			i++;
	    }
		j++;
	}



  }
  return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved)
{
  return 0;
}



#if defined(WINDOWS_VERSION) && !defined(BUILTIN_PLUGINS)

// ********************************************
// ***********  Editor Interface  *************
// ********************************************

  



const char* scriptHeader =
  "import void DrawScreenEffect(int sprite,int sprite_prev,int ide,int n);\r\n"
  "import void PlaySounds();\r\n"
  "import void SFX_Play(int SFX, int repeat);\r\n"
  "import void SFX_SetVolume(int SFX,int volume);\r\n"
  "import int SFX_GetVolume(int SFX);\r\n"
  "import void Music_Play(int MFX, int repeat,int fadeinMS,int fadeoutMS,int Position, bool fixclick=false);\r\n"
  "import void Music_SetVolume(int volume);\r\n"
  "import int Music_GetVolume();\r\n"
  "import void SFX_Stop(int SFX,int fademsOUT);\r\n"
  "import void SFX_SetPosition(int SFX,int x,int y,int intensity);\r\n"
  "import void SFX_SetGlobalVolume(int volume);\r\n"
  "import void Load_SFX(int SFX);\r\n"
  "import void Audio_Apply_Filter(int Frequency);\r\n"
  "import void Audio_Remove_Filter();\r\n"
  "import void SFX_AllowOverlap(int SFX,int allow);\r\n"
  "import void SFX_Filter(int SFX,int enable);\r\n"
  "import void Audio_Pause();\r\n"
  "import void Audio_Resume();\r\n"
  "import void DrawBlur(int spriteD,int radius);\r\n"
  "import void DrawTunnel(int spriteD, float scale, float speed);\r\n"
  "import void DrawCylinder(int spriteD, int ogsprite);\r\n"
  "import void DrawForceField(int spriteD, int scale, float speed,int id);\r\n"
  "import void Grayscale(int sprite);\r\n"
  "import void ReadWalkBehindIntoSprite(int sprite,int bgsprite,int walkbehindBaseline);\r\n"
  "import void AdjustSpriteFont(int sprite,int rate,int outlineRed,int outlineGreen,int outlineBlue);\r\n"
  "import void SpriteGradient(int sprite,int rate,int toy);\r\n"
  "import void Outline(int sprite,int red,int ged,int bed,int aed);\r\n"
  "import void OutlineOnly(int sprite,int refsprite,int red,int ged,int bed,int aed, int trans);\r\n"
  "import void SaveVariable(String value,int id);\r\n"
  "import String ReadVariable(int id);\r\n"
  "import int GameDoOnceOnly(String value);\r\n"
  "import void SetGDState(String value,bool setvalue);\r\n"
  "import bool GetGDState(String value);\r\n"
  "import void ResetAllGD();\r\n"
  "import void SpriteSkew(int sprite,float xskewmin, float yskewmin,float xskewmax, float yskewmax);\r\n"
  "import void FireUpdate(int getDynamicSprite, bool Fire2Visible);\r\n"
  "import void WindUpdate(int ForceX, int ForceY, int Transparency,int sprite);\r\n"
  "import void SetWindValues(int w,int h,int pr,int prev);\r\n"
  "import void RainUpdate(int rdensity, int FX, int FY,int RW,int RH,int graphic, float perc);\r\n"
  "import int ReturnWidth(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);\r\n"
  "import int ReturnHeight(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);\r\n"
  "import int ReturnNewHeight();\r\n"
  "import int ReturnNewWidth();\r\n"
  "import void SetWarper(int y2x,int x3x,int y3x,int x4x,int y4x);\r\n"
  "import void Warper(int swarp,int sadjust,int x1, int y1, int x2);\r\n"
  "import void BlendTwoSprites(int graphic, int refgraphic);\r\n"
  "import void Blend(int graphic, int refgraphic, bool screen,int perc);\r\n"
  "import void Dissolve(int graphic, int noisegraphic, int disvalue);\r\n"
  "import void ReverseTransparency(int graphic);\r\n"  
  "import void NoiseCreator(int graphic, int setA);\r\n"
  "import void TintProper(int sprite,int lightx,int lighty, int radi,int rex,int grx,int blx);\r\n"
  "import int GetWalkbehindBaserine(int id);\r\n"
  "import void SetWalkbehindBaserine(int id,int base);\r\n"
  ;

IAGSEditor* editor;


LPCSTR AGS_GetPluginName(void)
{
  // Return the plugin description
  return "AGSWave";
}

int  AGS_EditorStartup(IAGSEditor* lpEditor)
{
  // User has checked the plugin to use it in their game

  // If it's an earlier version than what we need, abort.
  if (lpEditor->version < 1)
    return -1;

  editor = lpEditor;
  editor->RegisterScriptHeader(scriptHeader);

  // Return 0 to indicate success
  return 0;
}

void AGS_EditorShutdown()
{
  // User has un-checked the plugin from their game
  editor->UnregisterScriptHeader(scriptHeader);
}

void AGS_EditorProperties(HWND parent)
{
  // User has chosen to view the Properties of the plugin
  // We could load up an options dialog or something here instead
  MessageBoxA(parent, "AGSWave", "About", MB_OK | MB_ICONINFORMATION);
}

int AGS_EditorSaveGame(char* buffer, int bufsize)
{
  // We don't want to save any persistent data
  return 0;
}

void AGS_EditorLoadGame(char* buffer, int bufsize)
{
  // Nothing to load for this plugin
}

#endif


#if defined(BUILTIN_PLUGINS)
} // namespace agswave
#endif
