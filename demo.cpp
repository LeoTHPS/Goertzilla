#include <iostream>

#include "Goertzilla.hpp"

#define SAMPLE_RATE   11025
#define SAMPLE_COUNT  512
#define CHANNEL_COUNT 1
#define CHANNEL_INDEX 0

int16_t samples[SAMPLE_COUNT] = {};

void samples_init()
{
	double f[] = { 697,  1209, 100  };
	double a[] = { 0.25, 0.25, 0.25 };

	Goertzilla::GenerateSineWave(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, f, a);
	Goertzilla::Window(samples, SAMPLE_COUNT, CHANNEL_INDEX, CHANNEL_COUNT, GOERTZILLA_WINDOW_HAMMING | GOERTZILLA_WINDOW_SYMMETRIC);
}

int main(int argc, char* argv[])
{
	samples_init();

	char   dtmf;  double dtmf_magnitude;
	double ctcss; double ctcss_magnitude;

	dtmf  = Goertzilla::DTMF(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, dtmf_magnitude);
	ctcss = Goertzilla::CTCSS(samples, SAMPLE_COUNT, SAMPLE_RATE, CHANNEL_INDEX, CHANNEL_COUNT, ctcss_magnitude);

	std::cout << "DTMF: "  << dtmf  << " - " << dtmf_magnitude  << std::endl;
	std::cout << "CTCSS: " << ctcss << " - " << ctcss_magnitude << std::endl;

	return 0;
}
