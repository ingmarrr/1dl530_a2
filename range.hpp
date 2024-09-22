#include <cstdint>
#include <vector>

auto erange(uint64_t start, uint64_t end) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i < end; ++i)
    {
        out.push_back(i);
    }

    return out;
}

auto erange(uint64_t start, uint64_t end, uint64_t step) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i < end; i += step) 
    {
        out.push_back(i);
    }

    return out;
}

auto irange(uint64_t start, uint64_t end) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i <= end; ++i) 
    {
        out.push_back(i);
    }

    return out;
}

auto irange(uint64_t start, uint64_t end, uint64_t step) -> std::vector<uint64_t>
{
    auto out = std::vector<uint64_t>();
    for (auto i = start; i <= end; i += step) 
    {
        out.push_back(i);
    }

    return out;
}
