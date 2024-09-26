#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "range.hpp"

const char* USAGE =
	"Usage: ./sieve [-h] <max>\n"
	"  -h			: Print this help message and exit\n"
	"  max			: Max number \n";

#define error(...) \
	std::cerr << "[error] " << __VA_ARGS__ << std::endl; \
	std::cerr << USAGE << std::endl; \
	exit(1); \


auto sieve_seq(int max) -> std::vector<uint64_t>
{
	auto composites = std::vector<bool>(max+1, false);

	for (auto k = 2; k*k < max; ++k)
	{
		if (!composites[k])
		{
			for (auto a = k*k; a <= max; a += k)
			{
				composites[a] = true;
			}
		}
	}

	std::vector<uint64_t> out{};
	for (auto ix : irange(2, max))
	{
		if (!composites[ix])
		{
			out.push_back(ix);
		}
	}

	return out;
}


auto sieve_seq_mark(std::vector<bool>& composites, int max) -> void
{
	for (auto k = 2; k*k < max; ++k)
	{
		if (!composites[k])
		{
			for (auto a = k*k; a <= max; a += k)
			{
				composites[a] = true;
			}
		}
	}
}

auto sieve_par_seeds(const std::vector<bool>& composites, int sqrt_max) -> std::vector<int>
{
	auto out = std::vector<int>();
	for (auto i : irange(2, sqrt_max))
	{
		if (!composites[i])
		{
			out.push_back(i);
		}
	}

	return out;
}


struct ThreadArgs {
    int start;
    int end;
    std::vector<bool>* composites;
    const std::vector<int>* seeds;
};

void* sieve_par_mark_pthread(void* arg)
{
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (auto prime : *(args->seeds))
    {
        auto first = std::max(prime*prime, (args->start+prime-1)/prime*prime);
        for (auto a = first; a <= args->end; a += prime)
        {
            (*(args->composites))[a] = true;
        }
    }
    return nullptr;
}

auto sieve_par(int max, int n_threads) -> std::vector<uint64_t>
{
    auto composites = std::vector<bool>(max + 1, false);
    auto sqrt_max   = static_cast<int>(std::sqrt(max));
    
    sieve_seq_mark(composites, max);
    
    auto seeds      = sieve_par_seeds(composites, sqrt_max);
    auto chunk_size = (max - sqrt_max) / n_threads;
    auto workers    = std::vector<pthread_t>(n_threads);
    auto args       = std::vector<ThreadArgs>(n_threads);

    for (auto tid : erange(0, n_threads))
    {
        int start	= sqrt_max+1+tid*chunk_size;
        int nums	= tid == n_threads-1 ? max-start+1 : chunk_size;
        auto end    = start+nums;
        args[tid] = {start, end, &composites, &seeds};
        pthread_create(&workers[tid], nullptr, sieve_par_mark_pthread, &args[tid]);
    }

    for (auto& worker : workers)
    {
        pthread_join(worker, nullptr);
    }

    auto out    = std::vector<uint64_t>();
    for (auto ix : irange(2, max))
    {
        if (!composites[ix])
        {
            out.push_back(ix);
        }
    }

    return out;
}


auto main(int argc, char* argv[]) -> int
{
	int		max;
	char	dummy;

	std::cout << "[info] running ./sieve" << std::endl;

	if (argc != 2 || sscanf(argv[1], "%d%c", &max, &dummy) != 1)
	{
		error("invalid argument, expected max (integer).");
	}

	auto p_res	= sieve_par(max, 10);
	auto s_res	= sieve_seq(max);

	assert(p_res.size() == s_res.size());
	for (auto ix : erange(0, p_res.size()))
	{
		std::cout << "[par]: " << p_res[ix] << "\t[seq]: " << s_res[ix] << std::endl;
		assert(p_res[ix] == s_res[ix]);
	}
}
