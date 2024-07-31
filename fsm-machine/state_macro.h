#pragma once
#include <array>
#include <tuple>

#define COUNT(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

#define CREATE_FSM_STATES(ENUM_NAME, ...) \
enum ENUM_NAME \
{ \
	__VA_ARGS__\
}; \
static constexpr std::array<ENUM_NAME, COUNT(__VA_ARGS__)> ENUM_NAME##_array_{__VA_ARGS__}
