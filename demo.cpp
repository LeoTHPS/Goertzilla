#include <iostream>

#include "Goertzilla.hpp"

#define SAMPLE_RATE  11025
#define SAMPLE_COUNT 512

float       samples[SAMPLE_COUNT] = {};
const float frequency[]           = {      200      };
const float frequency_test[]      = { 100, 200, 300 };

void samples_init()
{
	size_t freq_count     = sizeof(frequency) / sizeof(frequency[0]);
	double freq_amplitude = 1.0 / freq_count;

	for (size_t i = 0; i < SAMPLE_COUNT; ++i)
	{
		double coeff = GOERTZILLA_PI2 * frequency[i] / SAMPLE_RATE;

		for (size_t j = 0; j < freq_count; ++j)
			samples[i] += std::sin(coeff * i) * freq_amplitude;
	}
}

int main(int argc, char* argv[])
{
	samples_init();

	Goertzilla               g(SAMPLE_RATE, frequency_test);
	Goertzilla::ResultsPower g_results;

	g.Calculate(g_results, samples, SAMPLE_COUNT, 0, 1);

	for (size_t i = 0; i < g_results.size(); ++i)
		std::cout << frequency_test[i] << " -> " << g_results[i].Value << std::endl;

	return 0;
}
