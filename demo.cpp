#include <cmath>
#include <cstdint>

#include "Goertzilla.hpp"

#define SAMPLE_RATE   48000
#define SAMPLE_COUNT  512
#define CHANNEL_COUNT 1
#define CHANNEL_INDEX 0

int16_t samples[SAMPLE_COUNT] = {};

int main(int argc, char* argv[])
{
	double f[] = { 697,  1209, 67 };
	double a[] = { 0.25, 0.25, 0.25 };

	Goertzilla::GenerateSineWave(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, f, a);

	char   dtmf;  double dtmf_magnitude;
	double ctcss; double ctcss_magnitude;

	Goertzilla::Window(samples, SAMPLE_COUNT, CHANNEL_INDEX, CHANNEL_COUNT, GOERTZILLA_WINDOW_HAMMING | GOERTZILLA_WINDOW_SYMMETRIC);

	dtmf  = Goertzilla::DTMF(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, dtmf_magnitude);
	ctcss = Goertzilla::CTCSS(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, ctcss_magnitude);

	return 0;
}
