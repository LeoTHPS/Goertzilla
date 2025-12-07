#pragma once
#include <cmath>
#include <complex>
#include <cstdint>

class Goertzilla
{
	static constexpr double PI  = 3.14159265358979323846;
	static constexpr double PI2 = PI * 2;

	static constexpr char   DTMF_KEY[4][4]        = { { '1', '2', '3', 'A' }, { '4', '5', '6', 'B' }, { '7', '8', '9', 'C' }, { '*', '0', '#', 'D' } };
	static constexpr double DTMF_FREQUENCY[8]     = { 697, 770, 852, 941, 1209, 1336, 1477, 1633 };
	static constexpr size_t DTMF_FREQUENCY_COUNT  = sizeof(DTMF_FREQUENCY) / sizeof(double);

	static constexpr double CTCSS_FREQUENCY[]     = { 67.0, 71.9, 74.4, 77.0, 79.7, 82.5, 85.4, 88.5, 91.5, 94.8, 97.4, 100.0, 103.5, 107.2, 110.9, 114.8, 118.8, 123.0, 127.3, 131.8, 136.5, 141.3, 146.2, 151.4, 156.7, 162.2, 167.9, 173.8, 179.9, 186.2, 192.8, 203.5, 210.7, 218.1, 225.7, 233.6, 241.8, 250.3 };
	static constexpr size_t CTCSS_FREQUENCY_COUNT = sizeof(CTCSS_FREQUENCY) / sizeof(double);

	Goertzilla() = delete;

public:
	template<typename T>
	static auto DTMF(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, double& magnitude)
	{
		double frequency[DTMF_FREQUENCY_COUNT];
		size_t frequency_max_offset[2]    = { 0, 0 };
		double frequency_max_magnitude[2] = { __DBL_MIN__, __DBL_MIN__ };

		Goertzel(buffer, size, sample_rate, channel, channel_count, DTMF_FREQUENCY, frequency);

		for (size_t i = 0, j = 4; i < 4; ++i, ++j)
		{
			if (frequency[i] > frequency_max_magnitude[0])
			{
				frequency_max_offset[0]    = i;
				frequency_max_magnitude[0] = frequency[i];
			}

			if (frequency[j] > frequency_max_magnitude[1])
			{
				frequency_max_offset[1]    = j;
				frequency_max_magnitude[1] = frequency[j];
			}
		}

		magnitude = std::min(frequency_max_magnitude[0], frequency_max_magnitude[1]);

		return DTMF_KEY[frequency_max_offset[0]][frequency_max_offset[1] - 4];
	}

	template<typename T>
	static auto CTCSS(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, double& magnitude)
	{
		double frequency[CTCSS_FREQUENCY_COUNT];
		size_t frequency_max_offset    = 0;
		double frequency_max_magnitude = __DBL_MIN__;

		Goertzel(buffer, size, sample_rate, channel, channel_count, CTCSS_FREQUENCY, frequency);

		for (size_t i = 0; i < CTCSS_FREQUENCY_COUNT; ++i)
			if (frequency[i] > frequency_max_magnitude)
			{
				frequency_max_offset    = i;
				frequency_max_magnitude = frequency[i];
			}

		magnitude = frequency_max_magnitude;

		return CTCSS_FREQUENCY[frequency_max_offset];
	}

	template<typename T>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, double frequency, double& magnitude)
	{
		double f[1] = { frequency };
		double m[1] = {};

		Goertzel(buffer, size, sample_rate, channel, channel_count, f, m);

		magnitude = m[0];
	}
	template<typename T, size_t S>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, const double(&frequency)[S], double(&magnitude)[S])
	{
		double q[S][3]  = {};
		double coeff[S] = {};

		for (size_t i = 0; i < S; ++i)
			coeff[i] = 2 * std::cos((PI2 / size) * (0.5 + (size * frequency[i] / sample_rate)));

		for (size_t i = 0; i < size; ++i)
			for (size_t j = 0; j < S; ++j)
			{
				q[j][0] = coeff[j] * q[j][1] - q[j][2] + buffer[i];
				q[j][2] = q[j][1];
				q[j][1] = q[j][0];
			}

		for (size_t i = 0; i < S; ++i)
			magnitude[i] = q[i][1] * q[i][1] + q[i][2] * q[i][2] - q[i][1] * q[i][2] * coeff[i];
	}

	template<typename T>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, double frequency, std::complex<double>& value)
	{
		double               f[1] = { frequency };
		std::complex<double> m[1] = {};

		Goertzel(buffer, size, sample_rate, channel, channel_count, f, m);

		value = m[0];
	}
	template<typename T, size_t S>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, const double(&frequency)[S], std::complex<double>(&value)[S])
	{
		double q[S][3]  = {};
		double coeff[S] = {};
		double omega[S] = {};

		for (size_t i = 0; i < S; ++i)
		{
			omega[i] = (PI2 / size) * (0.5 + (size * frequency[i] / sample_rate));
			coeff[i] = std::cos(omega[i]) * 2;
		}

		for (size_t i = 0; i < size; ++i)
			for (size_t j = 0; j < S; ++j)
			{
				q[j][0] = coeff[j] * q[j][1] - q[j][2] + buffer[i];
				q[j][2] = q[j][1];
				q[j][1] = q[j][0];
			}

		for (size_t i = 0; i < S; ++i)
			value[i] = std::complex<double>(q[i][1] - q[i][2] * (coeff[i] / 2), q[i][2] * std::sin(omega[i]));
	}

	static auto GetPhase(std::complex<double> value)
	{
		return std::atan2(value.imag(), value.real());
	}
	static auto GetAmplitude(std::complex<double> value, size_t buffer_size)
	{
		return std::sqrt(std::pow(value.real(), 2) + std::pow(value.imag(), 2)) / buffer_size;
	}
};
