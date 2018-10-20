/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_INTERNAL_PIECEWISE_CONSTRUCT_T_H
#define EASTL_INTERNAL_PIECEWISE_CONSTRUCT_T_H


#include <EABase/eabase.h>
#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

namespace stl
{
	///////////////////////////////////////////////////////////////////////////////
	/// piecewise_construct_t
	///
	/// http://en.cppreference.com/w/cpp/utility/piecewise_construct_t
	///
	struct piecewise_construct_t
	{
		explicit piecewise_construct_t() = default;
	};


	///////////////////////////////////////////////////////////////////////////////
	/// piecewise_construct
	/// 
	/// A tag type used to disambiguate between function overloads that take two tuple arguments.
	///
	/// http://en.cppreference.com/w/cpp/utility/piecewise_construct
	///
	EA_CONSTEXPR piecewise_construct_t piecewise_construct = stl::piecewise_construct_t();

} // namespace stl


#endif // Header include guard






