#pragma once

#include "core/math/Vector2.h"
#include "core/math/Vector3.h"
#include "core/math/Vector4.h"
#include "core/math/Color.h"
#include "core/math/Quaternion.h"

namespace sky{

	Vector2 FromUE(const FVector2f& vec);

	Vector3 FromUE(const FVector3f& vec);

	Vector4 FromUE(const FVector4f& vec);

	Vector3 FromUE(const UE::Math::TVector<double>& vec);

	Quaternion FromUE(const FQuat& quat);

	Vector4 FromUE(const FLinearColor& color);

	template <typename T>
	Vector3 UEToRHYUpPosition(const UE::Math::TVector<T>& vec)
	{
		return Vector3(
			static_cast<float>(vec.X),    // X -> X
			static_cast<float>(-vec.Z),    // Z -> Y
			static_cast<float>(vec.Y)    // Y -> -Z
		);
	}

	template <typename T>
	Quaternion UEToRHYUpRotation(const UE::Math::TQuat<T>& quat)
	{
		UE::Math::TRotator<T> UnrealRot = quat.Rotator();

		UE::Math::TVector<T> RH_Euler(
			UnrealRot.Roll,
			UnrealRot.Yaw,
			-UnrealRot.Pitch
		);

		auto Result = UE::Math::TQuat<T>::MakeFromEuler(RH_Euler);

		return Quaternion(Result.W, Result.X, Result.Y, Result.Z);
	}

	template <typename T>
	void Convert(uint8_t* ptr, uint32_t num, const T* src)
	{
		T* dst = reinterpret_cast<T*>(ptr);

		for (uint32_t i = 0; i < num; i += 3)
		{
			const T& i0 = src[i + 0];
			const T& i1 = src[i + 1];
			const T& i2 = src[i + 2];


			dst[i + 0] = i0;
			dst[i + 1] = i2;
			dst[i + 2] = i1;
		}
	}

} // namespace sky