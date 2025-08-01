/// @ref ext_vector_reciprocal

#include <limits>

#include "../trigonometric.hpp"

namespace glm
{
	// sec
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> sec(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'sec' only accept floating-point inputs");
		return static_cast<T>(1) / detail::functor1<vec, L, T, T, Q>::call(cos, x);
	}

	// csc
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> csc(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'csc' only accept floating-point inputs");
		return static_cast<T>(1) / detail::functor1<vec, L, T, T, Q>::call(sin, x);
	}

	// cot
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> cot(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'cot' only accept floating-point inputs");
		T const pi_over_2 = static_cast<T>(3.1415926535897932384626433832795 / 2.0);
		return detail::functor1<vec, L, T, T, Q>::call(tan, pi_over_2 - x);
	}

	// asec
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> asec(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'asec' only accept floating-point inputs");
		return detail::functor1<vec, L, T, T, Q>::call(acos, static_cast<T>(1) / x);
	}

	// acsc
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> acsc(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'acsc' only accept floating-point inputs");
		return detail::functor1<vec, L, T, T, Q>::call(asin, static_cast<T>(1) / x);
	}

	// acot
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> acot(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'acot' only accept floating-point inputs");
		T const pi_over_2 = static_cast<T>(3.1415926535897932384626433832795 / 2.0);
		return pi_over_2 - detail::functor1<vec, L, T, T, Q>::call(atan, x);
	}

	// sech
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> sech(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'sech' only accept floating-point inputs");
		return static_cast<T>(1) / detail::functor1<vec, L, T, T, Q>::call(cosh, x);
	}

	// csch
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> csch(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'csch' only accept floating-point inputs");
		return static_cast<T>(1) / detail::functor1<vec, L, T, T, Q>::call(sinh, x);
	}

	// coth
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> coth(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'coth' only accept floating-point inputs");
		return glm::cosh(x) / glm::sinh(x);
	}

	// asech
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> asech(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'asech' only accept floating-point inputs");
		return detail::functor1<vec, L, T, T, Q>::call(acosh, static_cast<T>(1) / x);
	}

	// acsch
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> acsch(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'acsch' only accept floating-point inputs");
		return detail::functor1<vec, L, T, T, Q>::call(asinh, static_cast<T>(1) / x);
	}

	// acoth
	template<length_t L, typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<L, T, Q> acoth(vec<L, T, Q> const& x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || GLM_CONFIG_UNRESTRICTED_FLOAT, "'acoth' only accept floating-point inputs");
		return detail::functor1<vec, L, T, T, Q>::call(atanh, static_cast<T>(1) / x);
	}
}//namespace glm
