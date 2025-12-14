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
	for (size_t i = CHANNEL_INDEX; i < SAMPLE_COUNT; i += CHANNEL_COUNT)
	{
		auto j        = (i / CHANNEL_COUNT) / (double)SAMPLE_RATE;
		auto dtmf_1   = sin(2 * M_PI * 697 * j) + sin(2 * M_PI * 1209 * j);
		auto ctcss_67 = sin(2 * M_PI * 67 * j);

		samples[i] = (0.5 * (dtmf_1 + ctcss_67)) * INT16_MAX;
	}

	char   dtmf;  double dtmf_magnitude;
	double ctcss; double ctcss_magnitude;

	Goertzilla::Window(samples, SAMPLE_COUNT, CHANNEL_INDEX, CHANNEL_COUNT, GOERTZILLA_WINDOW_HAMMING | GOERTZILLA_WINDOW_SYMMETRIC);

	dtmf  = Goertzilla::DTMF(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, dtmf_magnitude);
	ctcss = Goertzilla::CTCSS(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, ctcss_magnitude);

	return 0;
}
