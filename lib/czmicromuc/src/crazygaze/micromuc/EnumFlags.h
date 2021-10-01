#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <type_traits>

//
// Helper to create enum based flags. How to use:
// enum class FooFlag : uint8_t
// {
//     None = 0,
//     Super = 1 << 0,
//     Duper = 1 << 1,
//     Trooper = 1 << 2
// };
// ENUM_CLASS_FLAGS(FooFlag)
//
// Or if the enum is private/protected in a class:
//
//
// class Bar
// {
//  protected:
// 
//     enum class FooFlag : uint8_t
//     {
//         None = 0,
//         Super = 1 << 0,
//         Duper = 1 << 1,
//         Trooper = 1 << 2
//     };
//     FRIEND_ENUM_CLASS_FLAGS(FooFlag);
// };
// ENUM_CLASS_FLAGS(Bar::FooFlag);
// 
//

#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator |=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(std::underlying_type_t<Enum>(lhs) | std::underlying_type_t<Enum>(rhs)); } \
	inline           Enum& operator &=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(std::underlying_type_t<Enum>(lhs) & std::underlying_type_t<Enum>(rhs)); } \
	inline           Enum& operator ^=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(std::underlying_type_t<Enum>(lhs) ^ std::underlying_type_t<Enum>(rhs)); } \
	inline constexpr Enum  operator | (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) | static_cast<std::underlying_type_t<Enum>>(rhs)); } \
	inline constexpr Enum  operator & (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) & static_cast<std::underlying_type_t<Enum>>(rhs)); } \
	inline constexpr Enum  operator ^ (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) ^ static_cast<std::underlying_type_t<Enum>>(rhs)); } \
	inline constexpr bool  operator ! (Enum  val)           { return !(std::underlying_type_t<Enum>(val)); } \
	inline constexpr Enum  operator ~ (Enum  val)           { return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(val)); }

#define FRIEND_ENUM_CLASS_FLAGS(Enum) \
	friend           Enum& operator|=(Enum& lhs, Enum rhs); \
	friend           Enum& operator&=(Enum& lhs, Enum rhs); \
	friend           Enum& operator^=(Enum& lhs, Enum rhs); \
	friend constexpr Enum  operator| (Enum  lhs, Enum rhs); \
	friend constexpr Enum  operator& (Enum  lhs, Enum rhs); \
	friend constexpr Enum  operator^ (Enum  lhs, Enum rhs); \
	friend constexpr bool  operator! (Enum  val); \
	friend constexpr Enum  operator~ (Enum  val);

template<typename Enum>
constexpr bool enumHasAllflags(Enum flags, Enum contains)
{
	static_assert(std::is_enum_v<Enum>, "Only works with enums");
	return  (std::underlying_type_t<Enum>(flags) & std::underlying_type_t<Enum>(contains)) == std::underlying_type_t<Enum>(contains);
}

template<typename Enum>
constexpr bool enumHasAnyFlags(Enum flags, Enum contains)
{
	static_assert(std::is_enum_v<Enum>, "Only works with enums");
	return (std::underlying_type_t<Enum>(flags) & std::underlying_type_t<Enum>(contains)) != 0;
}

template<typename Enum>
void enumAddFlags(Enum& flags, Enum flagsToAdd)
{
	static_assert(std::is_enum_v<Enum>, "Only works with enums");
	flags |= flagsToAdd;
}

template<typename Enum>
void enumRemoveFlags(Enum& flags, Enum flagsToRemove)
{
	static_assert(std::is_enum_v<Enum>, "Only works with enums");
	flags &= ~flagsToRemove;
}

