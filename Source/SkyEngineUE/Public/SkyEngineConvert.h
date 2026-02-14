#pragma once

#include "core/math/Vector2.h"
#include "core/math/Vector3.h"
#include "core/math/Vector4.h"
#include "core/math/Color.h"
#include "core/math/Quaternion.h"

namespace sky{

	Quaternion FromUE(const FQuat& quat);

	Vector3 FromUE(const UE::Math::TVector<double>& vec);

	Vector4 FromUE(const FLinearColor& color);

	template <typename T>
	void Convert(uint8_t* ptr, uint32_t num, const T* src, bool inverse = false)
	{
		T* dst = reinterpret_cast<T*>(ptr);

		for (uint32_t i = 0; i < num; i += 3)
		{
			const T& i0 = src[i + 0];
			const T& i1 = src[i + 1];
			const T& i2 = src[i + 2];

			if (inverse)
			{
				dst[i + 0] = i0;
				dst[i + 1] = i2;
				dst[i + 2] = i1;
			}
			else
			{
				dst[i + 0] = i0;
				dst[i + 1] = i1;
				dst[i + 2] = i2;
			}
		}
	}

} // namespace sky