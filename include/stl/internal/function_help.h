/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_INTERNAL_FUNCTION_HELP_H
#define EASTL_INTERNAL_FUNCTION_HELP_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <stl/internal/config.h>
#include <stl/type_traits.h>

namespace stl
{
	namespace internal
	{

		//////////////////////////////////////////////////////////////////////
		// is_null
		//
		template <typename T>
		bool is_null(const T&)
		{
			return false;
		}

		template <typename Result, typename... Arguments>
		bool is_null(Result (*const& function_pointer)(Arguments...))
		{
			return function_pointer == nullptr;
		}

		template <typename Result, typename Class, typename... Arguments>
		bool is_null(Result (Class::*const& function_pointer)(Arguments...))
		{
			return function_pointer == nullptr;
		}

		template <typename Result, typename Class, typename... Arguments>
		bool is_null(Result (Class::*const& function_pointer)(Arguments...) const)
		{
			return function_pointer == nullptr;
		}

	} // namespace internal
} // namespace stl

#endif // Header include guard

