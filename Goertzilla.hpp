#pragma once
#include <cmath>
#include <cfloat>
#include <complex>
#include <cstdint>

enum GOERTZILLA_WINDOW
{
	GOERTZILLA_WINDOW_PERIODIC         = 0b001,
	GOERTZILLA_WINDOW_SYMMETRIC        = 0b000,

	GOERTZILLA_WINDOW_FLATTOP          = 0b010,
	GOERTZILLA_WINDOW_HAMMING          = 0b100,
	GOERTZILLA_WINDOW_BLACKMAN_NUTTALL = 0b110
};

template<size_t S>
struct GoertzillaState
{
	double   CFF[S];
	double   COS[S];
	double   SIN[S];
	double   Frequency[S];
	uint32_t Channel;
	uint32_t ChannelCount;
};
struct GoertzillaResult
{
	double               Phase;
	double               Power;
	std::complex<double> Complex;
	double               Magnitude;
};

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
		double frequency_max_magnitude[2] = { DBL_MIN, DBL_MIN };

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
				frequency_max_magnitude[1]  = frequency[j];
				frequency_max_offset[1] = j;
			}
		}

		magnitude = (frequency_max_magnitude[0] <= frequency_max_magnitude[1]) ? frequency_max_magnitude[0] : frequency_max_magnitude[1];

		return DTMF_KEY[frequency_max_offset[0]][frequency_max_offset[1] - 4];
	}

	template<typename T>
	static auto CTCSS(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, double& magnitude)
	{
		double frequency[CTCSS_FREQUENCY_COUNT];
		size_t frequency_max_offset    = 0;
		double frequency_max_magnitude = DBL_MIN;

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
	static void Window(T* buffer, size_t size, uint32_t channel, uint32_t channel_count, int flags)
	{
		size_t n = size - (flags & 1);

		switch (flags & 0b110)
		{
			case GOERTZILLA_WINDOW_FLATTOP:
			{
				static constexpr double A[5] = { 0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368 };

				for (size_t i = channel; i < size; i += channel_count)
				{
					double w = PI2 * i / n;

					buffer[i] *= A[0] - A[1] * std::cos(w) + A[2] * std::cos(2 * w) - A[3] * std::cos(3 * w) + A[4] * std::cos(4 * w);
				}
			}
			break;

			case GOERTZILLA_WINDOW_HAMMING:
				for (size_t i = channel; i < size; i += channel_count)
					buffer[i] *= 0.54 - 0.46 * std::cos(PI2 * i / n);
				break;

			case GOERTZILLA_WINDOW_BLACKMAN_NUTTALL:
			{
				static constexpr double A[4] = { 0.3635819, 0.4891775, 0.1365995, 0.0106411 };

				for (size_t i = channel; i < size; i += channel_count)
				{
					double w = PI2 * i / n;

					buffer[i] *= A[0] - A[1] * std::cos(w) + A[2] * std::cos(2 * w) - A[3] * std::cos(3 * w);
				}
			}
			break;
		}
	}

	template<typename T>
	static void LowPass(T* buffer, size_t size, uint32_t channel, uint32_t channel_count, double coeff)
	{
		double state = 0;

		for (size_t i = channel; i < size; i += channel_count)
			buffer[i] = (state += coeff * ((double)buffer[i] - state));
	}

	template<typename T>
	static void HighPass(T* buffer, size_t size, uint32_t channel, uint32_t channel_count, double coeff)
	{
		double state[2][2] = {};

		for (size_t i = channel; i < size; i += channel_count)
		{
			state[0][0] = (double)buffer[i];
			state[0][1] = (1 - coeff) * (state[1][1] + state[0][0] - state[1][0]);
			buffer[i]   = state[0][1];
			state[1][0] = state[0][0];
			state[1][1] = state[0][1];
		}
	}

	template<typename T, size_t S>
	static void Goertzel(GoertzillaState<S>& state, const T* buffer, size_t size, GoertzillaResult(&result)[S])
	{
		auto   n       = size / (double)state.ChannelCount;
		double q[S][3] = {};

		for (size_t i = state.Channel; i < size; i += state.ChannelCount)
			for (size_t j = 0; j < S; ++j)
			{
				q[j][0] = (double)buffer[i] + state.CFF[j] * q[j][1] - q[j][2];
				q[j][2] = q[j][1];
				q[j][1] = q[j][0];
			}

		for (size_t j = 0; j < S; ++j)
		{
			auto i = q[j][1] * state.SIN[j];
			auto r = q[j][1] * state.COS[j] - q[j][2];
			auto c = std::complex<double>(r, i) / (double)n;

			result[j] =
			{
				.Phase     = std::atan2(c.imag(), c.real()),
				.Power     = (q[j][1] * q[j][1] + q[j][2] * q[j][2] - state.CFF[j] * q[j][1] * q[j][2]) / n,
				.Complex   = c,
				.Magnitude = std::sqrt(q[j][1] * q[j][1] + q[j][2] * q[j][2] - state.CFF[j] * q[j][1] * q[j][2]) / n
			};
		}
	}
	template<typename T, size_t S>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, const double(&frequency)[S], GoertzillaResult(&result)[S])
	{
		auto   n        = size / (double)channel_count;
		double q[S][3]  = {};
		double cos[S]   = {};
		double sin[S]   = {};
		double coeff[S] = {};

		for (size_t i = 0; i < S; ++i)
		{
			auto w = PI2 * frequency[i] / sample_rate;

			cos[i]   = std::cos(w);
			sin[i]   = std::sin(w);
			coeff[i] = std::cos(w) * 2;
		}

		for (size_t i = channel; i < size; i += channel_count)
			for (size_t j = 0; j < S; ++j)
			{
				q[j][0] = (double)buffer[i] + coeff[j] * q[j][1] - q[j][2];
				q[j][2] = q[j][1];
				q[j][1] = q[j][0];
			}

		for (size_t j = 0; j < S; ++j)
		{
			auto i = q[j][1] * sin[j];
			auto r = q[j][1] * cos[j] - q[j][2];
			auto c = std::complex<double>(r, i) / (double)n;

			result[j] =
			{
				.Phase     = std::atan2(c.imag(), c.real()),
				.Power     = (q[j][1] * q[j][1] + q[j][2] * q[j][2] - coeff[j] * q[j][1] * q[j][2]) / n,
				.Complex   = c,
				.Magnitude = std::sqrt(q[j][1] * q[j][1] + q[j][2] * q[j][2] - coeff[j] * q[j][1] * q[j][2]) / n
			};
		}
	}
	template<typename T, size_t S>
	static void Goertzel(const T* buffer, size_t size, uint32_t sample_rate, uint32_t channel, uint32_t channel_count, const double(&frequency)[S], double(&magnitude)[S])
	{
		auto   n        = size / (double)channel_count;
		double q[S][3]  = {};
		double coeff[S] = {};

		for (size_t i = 0; i < S; ++i)
			coeff[i] = 2 * std::cos(PI2 * frequency[i] / sample_rate);

		for (size_t i = channel; i < size; i += channel_count)
			for (size_t j = 0; j < S; ++j)
			{
				q[j][0] = (double)buffer[i] + coeff[j] * q[j][1] - q[j][2];
				q[j][2] = q[j][1];
				q[j][1] = q[j][0];
			}

		for (size_t j = 0; j < S; ++j)
			magnitude[j] = std::sqrt(q[j][1] * q[j][1] + q[j][2] * q[j][2] - coeff[j] * q[j][1] * q[j][2]) / n;
	}

	template<size_t S>
	static auto GoertzelBegin(uint32_t sample_rate, uint32_t channel, uint32_t channel_count, const double(&frequency)[S])
	{
		GoertzillaState<S> state =
		{
			.Channel      = channel,
			.ChannelCount = channel_count
		};

		memcpy(state.Frequency, frequency, S * sizeof(double));

		for (size_t i = 0; i < S; ++i)
		{
			auto w = PI2 * frequency[i] / sample_rate;

			state.COS[i] = std::cos(w);
			state.SIN[i] = std::sin(w);
			state.CFF[i] = std::cos(w) * 2;
		}

		return state;
	}
};
