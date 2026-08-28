#include "Goertzilla.hpp"

#include <cmath>

struct Goertzilla_IQ
{
	double I[3];
	double Q[3];
};

Goertzilla::Goertzilla(uint32_t sample_rate, const float* frequency, size_t count)
	: sine(count, 0),
	omega(count, 0),
	coeff(count, 0),
	cosine(count, 0),
	frequency(count, 0),
	sample_rate(sample_rate)
{
	for (size_t i = 0; i < count; ++i, ++frequency)
	{
		this->omega[i]     = GOERTZILLA_PI2 * *frequency / sample_rate;
		this->sine[i]      = std::sin(omega[i]);
		this->cosine[i]    = std::cos(omega[i]);
		this->coeff[i]     = cosine[i] * 2;
		this->frequency[i] = *frequency;
	}
}

bool Goertzilla::Calculate(Results& value, const std::complex<float>* buffer, size_t size) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, [](size_t i, double real, double imag, void* param) {
			(*((Results*)param))[i] = {
				.Phase     = std::atan2(imag, real),
				.Power     = (real * real) + (imag * imag),
				.Magnitude = std::sqrt((real * real) + (imag * imag))
			};
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsPhase& value, const std::complex<float>* buffer, size_t size) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, [](size_t i, double real, double imag, void* param) {
			(*((ResultsPhase*)param))[i] = { .Value = std::atan2(imag, real) };
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsPower& value, const std::complex<float>* buffer, size_t size) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, [](size_t i, double real, double imag, void* param) {
			(*((ResultsPower*)param))[i] = { .Value = (real * real) + (imag * imag) };
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsMagnitude& value, const std::complex<float>* buffer, size_t size) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, [](size_t i, double real, double imag, void* param) {
			(*((ResultsMagnitude*)param))[i] = { .Value = std::sqrt((real * real) + (imag * imag)) };
		}, &value);

		return true;
	}

	return false;
}

bool Goertzilla::Calculate(Results& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, channel, channel_count, [](size_t i, double real, double imag, void* param) {
			(*((Results*)param))[i] = {
				.Phase     = std::atan2(imag, real),
				.Power     = (real * real) + (imag * imag),
				.Magnitude = std::sqrt((real * real) + (imag * imag))
			};
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsPhase& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, channel, channel_count, [](size_t i, double real, double imag, void* param) {
			(*((ResultsPhase*)param))[i] = { .Value = std::atan2(imag, real) };
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsPower& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, channel, channel_count, [](size_t i, double real, double imag, void* param) {
			(*((ResultsPower*)param))[i] = { .Value = (real * real) + (imag * imag) };
		}, &value);

		return true;
	}

	return false;
}
bool Goertzilla::Calculate(ResultsMagnitude& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const
{
	if (auto frequency_count = GetFrequencyCount())
	{
		value.resize(frequency_count);

		Calculate(buffer, size, channel, channel_count, [](size_t i, double real, double imag, void* param) {
			(*((ResultsMagnitude*)param))[i] = { .Value = std::sqrt((real * real) + (imag * imag)) };
		}, &value);

		return true;
	}

	return false;
}

void Goertzilla::Calculate(const std::complex<float>* buffer, size_t size, void(*function)(size_t i, double real, double imag, void* param), void* param) const
{
	std::vector<Goertzilla_IQ> iqs(GetFrequencyCount(), { { 0, 0, 0 }, { 0, 0, 0 } });
	auto                       iq    = iqs.data();
	auto                       coeff = this->coeff.data();

	for (size_t i = 0; i < GetFrequencyCount(); ++i, ++iq, ++coeff)
		for (size_t j = 0; j < size; ++j)
		{
			auto new_i = buffer[j].real() + (*coeff * iq->I[1] - iq->I[2]);
			auto new_q = buffer[j].imag() + (*coeff * iq->Q[1] - iq->Q[2]);

			iq->I[2] = iq->I[1]; iq->I[1] = new_i; iq->I[0] = new_i;
			iq->Q[2] = iq->Q[1]; iq->Q[1] = new_q; iq->Q[0] = new_q;
		}

	     iq     = iqs.data();
	auto sine   = this->sine.data();
	auto cosine = this->cosine.data();

	for (size_t i = 0; i < GetFrequencyCount(); ++i, ++iq, ++sine, ++cosine)
	{
		double real = (iq->Q[1] * *cosine) + (iq->I[1] * *sine) - iq->Q[2];
		double imag = (iq->I[1] * *cosine) - (iq->Q[1] * *sine) - iq->I[2];

		function(i, real, imag, param);
	}
}
void Goertzilla::Calculate(const float* buffer, size_t size, uint32_t channel, uint32_t channel_count, void(*function)(size_t i, double real, double imag, void* param), void* param) const
{
	std::vector<Goertzilla_IQ> iqs(GetFrequencyCount(), { { 0, 0, 0 }, { 0, 0, 0 } });
	auto                       iq    = iqs.data();
	auto                       coeff = this->coeff.data();

	for (size_t i = 0; i < GetFrequencyCount(); ++i, ++iq, ++coeff)
	{
		for (size_t j = channel; j < size; j += channel_count)
		{
			auto new_i = buffer[j] + (*coeff * iq->I[1] - iq->I[2]);
			auto new_q =             (*coeff * iq->Q[1] - iq->Q[2]);

			iq->I[2] = iq->I[1]; iq->I[1] = new_i; iq->I[0] = new_i;
			iq->Q[2] = iq->Q[1]; iq->Q[1] = new_q; iq->Q[0] = new_q;
		}
	}

	     iq     = iqs.data();
	auto sine   = this->sine.data();
	auto cosine = this->cosine.data();

	for (size_t i = 0; i < GetFrequencyCount(); ++i, ++iq, ++sine, ++cosine)
	{
		double real = (iq->Q[1] * *cosine) + (iq->I[1] * *sine) - iq->Q[2];
		double imag = (iq->I[1] * *cosine) - (iq->Q[1] * *sine) - iq->I[2];

		function(i, real, imag, param);
	}
}
