#pragma once
#include <vector>
#include <complex>
#include <cstdint>

class Goertzilla
{
	std::vector<double> sine;
	std::vector<double> omega;
	std::vector<double> coeff;
	std::vector<double> cosine;
	std::vector<float>  frequency;
	uint32_t            sample_rate;

public:
	typedef double Phase;
	typedef double Power;
	typedef double Magnitude;

	struct Result
	{
		Goertzilla::Phase     Phase;
		Goertzilla::Power     Power;
		Goertzilla::Magnitude Magnitude;
	};
	struct ResultPhase
	{
		Phase Value;
	};
	struct ResultPower
	{
		Power Value;
	};
	struct ResultMagnitude
	{
		Magnitude Value;
	};

	typedef std::vector<Result>          Results;
	typedef std::vector<ResultPhase>     ResultsPhase;
	typedef std::vector<ResultPower>     ResultsPower;
	typedef std::vector<ResultMagnitude> ResultsMagnitude;

	Goertzilla()
		: Goertzilla(0, nullptr, 0)
	{
	}
	template<size_t S>
	Goertzilla(uint32_t sample_rate, const float(&frequency)[S])
		: Goertzilla(sample_rate, frequency, S)
	{
	}
	Goertzilla(uint32_t sample_rate, const float* frequency, size_t count);

	constexpr auto GetSampleRate() const
	{
		return sample_rate;
	}

	constexpr auto GetFrequencyCount() const
	{
		return frequency.size();
	}

	bool Calculate(Results& value, const std::complex<float>* buffer, size_t size) const;
	bool Calculate(ResultsPhase& value, const std::complex<float>* buffer, size_t size) const;
	bool Calculate(ResultsPower& value, const std::complex<float>* buffer, size_t size) const;
	bool Calculate(ResultsMagnitude& value, const std::complex<float>* buffer, size_t size) const;

	bool Calculate(Results& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const;
	bool Calculate(ResultsPhase& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const;
	bool Calculate(ResultsPower& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const;
	bool Calculate(ResultsMagnitude& value, const float* buffer, size_t size, uint32_t channel, uint32_t channel_count) const;

private:
	void Calculate(const std::complex<float>* buffer, size_t size, void(*function)(size_t i, double real, double imag, void* param), void* param) const;
	void Calculate(const float* buffer, size_t size, uint32_t channel, uint32_t channel_count, void(*function)(size_t i, double real, double imag, void* param), void* param) const;
};
