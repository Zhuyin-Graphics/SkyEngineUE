#pragma once

#include "core/math/Vector2.h"
#include "core/math/Vector3.h"
#include "core/math/Vector4.h"
#include "core/math/Quaternion.h"

namespace sky{

	Vector2 FromUE(const FVector2f& vec);

	Vector3 FromUE(const FVector3f& vec);

	Vector4 FromUE(const FVector4f& vec);

	Vector3 FromUE(const UE::Math::TVector<double>& vec);

	Quaternion FromUE(const FQuat& quat);

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

} // namespace sky