/// @ref gtc_integer
/// @file glm/gtc/integer.hpp
///
/// @see core (dependence)
/// @see gtc_integer (dependence)
///
/// @defgroup gtc_integer GLM_GTC_integer
/// @ingroup gtc
///
/// Include <glm/gtc/integer.hpp> to use the features of this extension.
///
/// @brief Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include <limits>

#include "../common.hpp"
#include "../detail/qualifier.hpp"
#include "../detail/setup.hpp"
#include "../exponential.hpp"
#include "../ext/scalar_common.hpp"
#include "../ext/vector_common.hpp"
#include "../integer.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_GTC_integer extension included")
#endif

namespace glm
{
	/// @addtogroup gtc_integer
	/// @{

	/// Returns the log2 of x for integer values. Useful to compute mipmap count from the texture size.
	/// @see gtc_integer
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_DECL vec<L, T, Q> log2(vec<L, T, Q> const& v);

	/// @}
} //namespace glm

#include "integer.inl"
