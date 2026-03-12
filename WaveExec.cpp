// Used IDE: Visual Studio 2015

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr std::uint32_t kDurationMs = 5000;
constexpr std::uint32_t kSampleRate = 44100;
constexpr std::uint16_t kNumChannels = 1;
constexpr std::uint16_t kBitsPerSample = 16;
constexpr double kFrequencyHz = 440.0;
constexpr double kVolume = 1.0;
constexpr const char* kOutputFileName = "test.wav";

using WaveFunction = double (*)(double timing);

struct WaveHeader {
	char chunkId[4] = {'R', 'I', 'F', 'F'};
	std::uint32_t chunkSize = 0;
	char format[4] = {'W', 'A', 'V', 'E'};

	char subchunk1Id[4] = {'f', 'm', 't', ' '};
	std::uint32_t subchunk1Size = 16;
	std::uint16_t audioFormat = 1;
	std::uint16_t numChannels = 0;
	std::uint32_t sampleRate = 0;
	std::uint32_t byteRate = 0;
	std::uint16_t blockAlign = 0;
	std::uint16_t bitsPerSample = 0;

	char subchunk2Id[4] = {'d', 'a', 't', 'a'};
	std::uint32_t subchunk2Size = 0;
};

static_assert(sizeof(WaveHeader) == 44, "PCM wave header must be 44 bytes");

std::uint32_t CalculateSampleCount(std::uint32_t sampleRate, std::uint32_t durationMs)
{
	return static_cast<std::uint32_t>(static_cast<std::uint64_t>(durationMs) * sampleRate / 1000ULL);
}

double WaveGenerator(double radians)
{
	return std::sin(radians);
}

WaveHeader GenerateWaveHeader(
	std::uint16_t bitsPerSample,
	std::uint16_t numChannels,
	std::uint32_t sampleRate,
	std::uint32_t durationMs)
{
	WaveHeader header;
	header.bitsPerSample = bitsPerSample;
	header.numChannels = numChannels;
	header.sampleRate = sampleRate;
	header.blockAlign = static_cast<std::uint16_t>(numChannels * bitsPerSample / 8);
	header.byteRate = header.sampleRate * header.blockAlign;

	const std::uint32_t sampleCount = CalculateSampleCount(sampleRate, durationMs);
	header.subchunk2Size = sampleCount * header.blockAlign;
	header.chunkSize = 36 + header.subchunk2Size;

	return header;
}

void Generate16BitWave(
	std::vector<std::int16_t>& buffer,
	std::uint32_t sampleRate,
	double frequencyHz,
	double volume,
	WaveFunction waveFunction)
{
	double normalizedVolume = volume;
	if (normalizedVolume < 0.0) {
		normalizedVolume = 0.0;
	} else if (normalizedVolume > 1.0) {
		normalizedVolume = 1.0;
	}
	const double amplitude = 32767.0 * normalizedVolume;
	const double angularFrequency = 2.0 * kPi * frequencyHz;

	for (std::size_t i = 0; i < buffer.size(); ++i) {
		const double absoluteTime = static_cast<double>(i) / sampleRate;
		const double radians = angularFrequency * absoluteTime;
		const double sample = waveFunction(radians);
		buffer[i] = static_cast<std::int16_t>(sample * amplitude);
	}
}

bool WriteWaveFile(const char* fileName, const WaveHeader& header, const std::vector<std::int16_t>& samples)
{
	std::FILE* file = nullptr;
#ifdef _MSC_VER
	if (fopen_s(&file, fileName, "wb") != 0) {
		file = nullptr;
	}
#else
	file = std::fopen(fileName, "wb");
#endif

	if (file == nullptr) {
		std::perror("Failed to open output file");
		return false;
	}

	const bool wroteHeader = std::fwrite(&header, sizeof(header), 1, file) == 1;
	const bool wroteSamples =
		std::fwrite(samples.data(), sizeof(samples[0]), samples.size(), file) == samples.size();

	if (std::fclose(file) != 0) {
		std::perror("Failed to close output file");
		return false;
	}

	if (!wroteHeader || !wroteSamples) {
		std::fprintf(stderr, "Failed to write the complete wave file.\n");
		return false;
	}

	return true;
}

}  // namespace

int main()
{
	if (kBitsPerSample != 16) {
		std::fprintf(stderr, "This sample currently supports only 16-bit PCM output.\n");
		return EXIT_FAILURE;
	}

	const WaveHeader header =
		GenerateWaveHeader(kBitsPerSample, kNumChannels, kSampleRate, kDurationMs);
	std::vector<std::int16_t> samples(CalculateSampleCount(kSampleRate, kDurationMs), 0);
	Generate16BitWave(samples, kSampleRate, kFrequencyHz, kVolume, WaveGenerator);

	if (!WriteWaveFile(kOutputFileName, header, samples)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
