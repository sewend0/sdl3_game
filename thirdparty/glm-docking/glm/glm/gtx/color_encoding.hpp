/// @ref gtx_color_encoding
/// @file glm/gtx/color_encoding.hpp
///
/// @see core (dependence)
/// @see gtx_color_encoding (dependence)
///
/// @defgroup gtx_color_encoding GLM_GTX_color_encoding
/// @ingroup gtx
///
/// Include <glm/gtx/color_encoding.hpp> to use the features of this extension.
///
/// @brief Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include <limits>

#include "../detail/qualifier.hpp"
#include "../detail/setup.hpp"
#include "../vec3.hpp"

#ifndef GLM_ENABLE_EXPERIMENTAL
#	error "GLM: GLM_GTC_color_encoding is an experimental extension and may change in the future. Use #define GLM_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_GTC_color_encoding extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_color_encoding
	/// @{

	/// Convert a linear sRGB color to D65 YUV.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<3, T, Q> convertLinearSRGBToD65XYZ(vec<3, T, Q> const& ColorLinearSRGB);

	/// Convert a linear sRGB color to D50 YUV.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<3, T, Q> convertLinearSRGBToD50XYZ(vec<3, T, Q> const& ColorLinearSRGB);

	/// Convert a D65 YUV color to linear sRGB.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<3, T, Q> convertD65XYZToLinearSRGB(vec<3, T, Q> const& ColorD65XYZ);

	/// Convert a D65 YUV color to D50 YUV.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<3, T, Q> convertD65XYZToD50XYZ(vec<3, T, Q> const& ColorD65XYZ);

	/// @}
} //namespace glm

#include "color_encoding.inl"
