/// @ref ext_matrix_transform
/// @file glm/ext/matrix_transform.hpp
///
/// @defgroup ext_matrix_transform GLM_EXT_matrix_transform
/// @ingroup ext
///
/// Defines functions that generate common transformation matrices.
///
/// The matrices generated by this extension use standard OpenGL fixed-function
/// conventions. For example, the lookAt function generates a transform from world
/// space into the specific eye space that the projective matrix functions
/// (perspective, ortho, etc) are designed to expect. The OpenGL compatibility
/// specifications defines the particular layout of this eye space.
///
/// Include <glm/ext/matrix_transform.hpp> to use the features of this extension.
///
/// @see ext_matrix_projection
/// @see ext_matrix_clip_space

#pragma once

// Dependencies
#include "../geometric.hpp"
#include "../gtc/constants.hpp"
#include "../matrix.hpp"
#include "../trigonometric.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_matrix_transform extension included")
#endif

namespace glm
{
	/// @addtogroup ext_matrix_transform
	/// @{

	/// Builds an identity matrix.
	template<typename genType>
	GLM_FUNC_DECL GLM_CONSTEXPR genType identity();

	/// Builds a translation 4 * 4 matrix created from a vector of 3 components.
	///
	/// @param m Input matrix multiplied by this translation matrix.
	/// @param v Coordinates of a translation vector.
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @code
	/// #include <glm/glm.hpp>
	/// #include <glm/gtc/matrix_transform.hpp>
	/// ...
	/// glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f));
	/// // m[0][0] == 1.0f, m[0][1] == 0.0f, m[0][2] == 0.0f, m[0][3] == 0.0f
	/// // m[1][0] == 0.0f, m[1][1] == 1.0f, m[1][2] == 0.0f, m[1][3] == 0.0f
	/// // m[2][0] == 0.0f, m[2][1] == 0.0f, m[2][2] == 1.0f, m[2][3] == 0.0f
	/// // m[3][0] == 1.0f, m[3][1] == 1.0f, m[3][2] == 1.0f, m[3][3] == 1.0f
	/// @endcode
	///
	/// @see - translate(mat<4, 4, T, Q> const& m, T x, T y, T z)
	/// @see - translate(vec<3, T, Q> const& v)
	/// @see <a href="https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml">glTranslate man page</a>
	template<typename T, qualifier Q>
	GLM_FUNC_DECL GLM_CONSTEXPR mat<4, 4, T, Q> translate(
		mat<4, 4, T, Q> const& m, vec<3, T, Q> const& v);

	/// Builds a rotation 4 * 4 matrix created from an axis vector and an angle.
	///
	/// @param m Input matrix multiplied by this rotation matrix.
	/// @param angle Rotation angle expressed in radians.
	/// @param axis Rotation axis, recommended to be normalized.
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @see - rotate(mat<4, 4, T, Q> const& m, T angle, T x, T y, T z)
	/// @see - rotate(T angle, vec<3, T, Q> const& v)
	/// @see <a href="https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml">glRotate man page</a>
	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> rotate(
		mat<4, 4, T, Q> const& m, T angle, vec<3, T, Q> const& axis);

	/// Builds a scale 4 * 4 matrix created from 3 scalars.
	///
	/// @param m Input matrix multiplied by this scale matrix.
	/// @param v Ratio of scaling for each axis.
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @see - scale(mat<4, 4, T, Q> const& m, T x, T y, T z)
	/// @see - scale(vec<3, T, Q> const& v)
	/// @see <a href="https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml">glScale man page</a>
	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> scale(
		mat<4, 4, T, Q> const& m, vec<3, T, Q> const& v);

    /// Builds a scale 4 * 4 matrix created from point referent 3 shearers.
    ///
    /// @param m Input matrix multiplied by this shear matrix.
    /// @param p Point of shearing as reference.
    /// @param l_x Ratio of matrix.x projection in YZ plane relative to the y-axis/z-axis.
    /// @param l_y Ratio of matrix.y projection in XZ plane relative to the x-axis/z-axis.
    /// @param l_z Ratio of matrix.z projection in XY plane relative to the x-axis/y-axis.
    ///
    /// as example:
    ///                                         [1   , l_xy, l_xz, -(l_xy+l_xz) * p_x] [x]  T
    ///   [x`, y`, z`, w`] = [x`, y`, z`, w`] * [l_yx, 1   , l_yz, -(l_yx+l_yz) * p_y] [y]
    ///                                         [l_zx, l_zy, 1   , -(l_zx+l_zy) * p_z] [z]
    ///                                         [0   , 0   , 0   , 1                 ] [w]
    ///
    /// @tparam T A floating-point shear type
    /// @tparam Q A value from qualifier enum
    ///
    /// @see - shear(mat<4, 4, T, Q> const& m, T x, T y, T z)
    /// @see - shear(vec<3, T, Q> const& p)
    /// @see - shear(vec<2, T, Q> const& l_x)
    /// @see - shear(vec<2, T, Q> const& l_y)
    /// @see - shear(vec<2, T, Q> const& l_z)
    /// @see no resource...
    template <typename T, qualifier Q>
    GLM_FUNC_QUALIFIER mat<4, 4, T, Q> shear(
        mat<4, 4, T, Q> const &m, vec<3, T, Q> const& p, vec<2, T, Q> const &l_x, vec<2, T, Q> const &l_y, vec<2, T, Q> const &l_z);

    /// Build a right handed look at view matrix.
	///
	/// @param eye Position of the camera
	/// @param center Position where the camera is looking at
	/// @param up Normalized up vector, how the camera is oriented. Typically (0, 0, 1)
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @see - frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal) frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal)
	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> lookAtRH(
		vec<3, T, Q> const& eye, vec<3, T, Q> const& center, vec<3, T, Q> const& up);

	/// Build a left handed look at view matrix.
	///
	/// @param eye Position of the camera
	/// @param center Position where the camera is looking at
	/// @param up Normalized up vector, how the camera is oriented. Typically (0, 0, 1)
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @see - frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal) frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal)
	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> lookAtLH(
		vec<3, T, Q> const& eye, vec<3, T, Q> const& center, vec<3, T, Q> const& up);

	/// Build a look at view matrix based on the default handedness.
	///
	/// @param eye Position of the camera
	/// @param center Position where the camera is looking at
	/// @param up Normalized up vector, how the camera is oriented. Typically (0, 0, 1)
	///
	/// @tparam T A floating-point scalar type
	/// @tparam Q A value from qualifier enum
	///
	/// @see - frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal) frustum(T const& left, T const& right, T const& bottom, T const& top, T const& nearVal, T const& farVal)
	/// @see <a href="https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml">gluLookAt man page</a>
	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> lookAt(
		vec<3, T, Q> const& eye, vec<3, T, Q> const& center, vec<3, T, Q> const& up);

	/// @}
}//namespace glm

#include "matrix_transform.inl"
