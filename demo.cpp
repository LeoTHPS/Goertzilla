#include <cmath>
#include <limits>

#include "Goertzilla.hpp"

#define SAMPLE_RATE   48000
#define SAMPLE_COUNT  512
#define CHANNEL_COUNT 1
#define CHANNEL_INDEX 0

int16_t samples[SAMPLE_COUNT] = {};

int main(int argc, char* argv[])
{
	for (size_t i = CHANNEL_INDEX; i < SAMPLE_COUNT; i += CHANNEL_COUNT)
	{
		auto dtmf_1      = sin(2 * M_PI * 697 * ((double)i / SAMPLE_RATE)) + sin(2 * M_PI * 1209 * ((double)i / SAMPLE_RATE));
		auto ctcss_67    = sin(2 * M_PI * 67 * ((double)i / SAMPLE_RATE));
		auto ctcss_103_5 = sin(2 * M_PI * 103.5 * ((double)i / SAMPLE_RATE));

		samples[i] = 0.5 * (dtmf_1 + ctcss_67);
	}

	char   dtmf;  double dtmf_magnitude;
	double ctcss; double ctcss_magnitude;

	// Goertzilla::Window(samples, SAMPLE_COUNT, Goertzilla::WINDOW_HAMMING | Goertzilla::WINDOW_SYMMETRIC);

	dtmf  = Goertzilla::DTMF(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, dtmf_magnitude);
	ctcss = Goertzilla::CTCSS(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, ctcss_magnitude);

	return 0;
}
