/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

/**
\ingroup libutils
\brief Definition of a constexpr function that returns the underlying type of a enum class.
It also defines bitwise operators for enum classes to allow their use as flag containers.
*/

#ifndef ENUM_TYPE_H
#define ENUM_TYPE_H

#include <type_traits>

//! \brief This function causes the provided enum to be converted to its underlying datatype
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr std::underlying_type_t<Enum> enum_t (Enum enum_val) noexcept
{
	return static_cast<typename std::underlying_type_t<Enum>>(enum_val);
}

//! \brief This function peforms OR bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum operator | (Enum left_enum, Enum right_enum) noexcept
{
	return static_cast<Enum>(enum_t(left_enum) | enum_t(right_enum));
}

//! \brief This function peforms cummulative OR bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum &operator |= (Enum &left_enum, Enum right_enum) noexcept
{
	return left_enum = (left_enum | right_enum);
}

//! \brief This function peforms XOR bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum operator ^ (Enum left_enum, Enum right_enum) noexcept
{
	return static_cast<Enum>(enum_t(left_enum) ^ enum_t(right_enum));
}

//! \brief This function peforms cummulative XOR bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum &operator ^= (Enum &left_enum, Enum right_enum) noexcept
{
	return left_enum = (left_enum ^ right_enum);
}

//! \brief This function peforms AND bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum operator & (Enum left_enum, Enum right_enum) noexcept
{
	return static_cast<Enum>(enum_t(left_enum) & enum_t(right_enum));
}

//! \brief This function peforms cummulative AND bitwise operation on enums
template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr Enum &operator &= (Enum &left_enum, Enum right_enum) noexcept
{
	return left_enum = (left_enum & right_enum);
}

#endif
