// WaveExec.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#define _USE_MATH_DEFINES

#include <fcntl.h>
#include <math.h>
#include <fcntl.h>

typedef struct waveHeader_tag
{
	char ChunkID[4] = { 'R', 'I', 'F', 'F' };
	unsigned int ChunkSize = 0;
	char Format[4] = {'W', 'A', 'V', 'E'};

	char Subchunk1ID[4] = { 'f', 'm', 't', ' ' };
	unsigned int Subchunk1Size = 16;
	unsigned short AudioFormat = 1;
	unsigned short NumChannels = 0;
	unsigned int SampleRate = 0;
	unsigned int ByteRate = 0;
	unsigned short BlockAlign = 0;
	unsigned short BitsPerSample = 0;

	char Subchunk2ID[4] = { 'd', 'a', 't', 'a' };
	unsigned int Subchunk2Size = 0;
}waveHeader;

typedef double(*func)(double timing);

void GenerateWaveHeader(waveHeader *p, unsigned short BitsPerSample, unsigned short NumChannels, unsigned int SampleRate, unsigned int DurationMSec);
void Generate16bitWave(short *buffer, unsigned int SampleRate, unsigned int DurationMSec, double freq, double volume, func waveFunc);

//d: phase degree
inline double waveGenerator(double d)
{
	return sin(d);
	//return (sin(d) + sin(d*3) + cos(d*2))/3;	//sine wave
}

int main()
{
	//Specification
	unsigned int DurationMSec = 5000;	//5 Seconds
	unsigned int SampleRate = 44100;	//44.10Khz Sampling
	unsigned short NumChannels = 1;		//Mono
	unsigned short BitsPerSample = 16;	//16 bits per sample

	//Header generation
	waveHeader hdr;
	GenerateWaveHeader(&hdr, BitsPerSample, NumChannels, SampleRate, DurationMSec);
	
	//Waveform Generation
	unsigned int nSamples = DurationMSec * hdr.SampleRate / 1000;
	short *channel = new short[nSamples];
	memset(channel, 0, nSamples * sizeof(short));
	Generate16bitWave(channel, hdr.SampleRate, DurationMSec, 440, 1, waveGenerator);

	//File Write
	FILE *fp = nullptr;
	fopen_s(&fp, "test.wav", "w+b");
	fwrite(&hdr, sizeof(waveHeader), 1, fp);
	fwrite(channel, sizeof(short), hdr.Subchunk2Size / sizeof(short), fp);
	fclose(fp);
	
#if 0
	fopen_s(&fp, "test.csv", "w+b");
	for (unsigned int i = 0; i < nSamples; i++)
		fprintf(fp, "%d,\n", channel[i]);
	fclose(fp);
#endif

	delete[] channel;
	return 0;
}

void GenerateWaveHeader(waveHeader *p, unsigned short BitsPerSample, unsigned short NumChannels, unsigned int SampleRate, unsigned int DurationMSec)
{
	p->BitsPerSample = BitsPerSample;
	p->NumChannels = NumChannels;
	p->SampleRate = SampleRate;
	p->ByteRate = p->SampleRate * p->NumChannels * p->BitsPerSample / 8;
	p->BlockAlign = p->Subchunk1Size * p->NumChannels / 8;
	p->Subchunk2Size = DurationMSec * p->ByteRate / 1000;
	p->ChunkSize = p->Subchunk2Size + sizeof(waveHeader) - 8;
}

void Generate16bitWave(short *buffer, unsigned int SampleRate, unsigned int DurationMSec, double freq, double volume, func waveFunc)
{
	const double radian_convert_coef = M_PI / 180.0;
	unsigned int nSamples = DurationMSec * SampleRate / 1000;
	double scale = 32767.0 * volume;
	double freqTime = 1.0 / freq;

	//Degre conversion
	//freqTime : x = absolutTime * 360
	//x = absoluteTime * 360 / freqTime
	for (unsigned int i = 0; i < nSamples; i++)
	{
		double absoluteTime = double(i) / SampleRate;
		double degree = absoluteTime * 360 / freqTime;
		double radian = degree * radian_convert_coef;
		double sample = waveFunc(radian);
		buffer[i] = (short)(sample * scale);
	}
}
