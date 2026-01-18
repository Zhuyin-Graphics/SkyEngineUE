
#include "SkyEngineConvert.h"

namespace sky {

	Vector2 FromUE(const FVector2f& vec)
	{
		return Vector2(vec.X, vec.Y);
	}

	Vector3 FromUE(const FVector3f& vec)
	{
		return Vector3(vec.X, vec.Y, vec.Z);
	}

	Vector4 FromUE(const FVector4f& vec)
	{
		return Vector4(vec.X, vec.Y, vec.Z, vec.W);
	}

	Vector3 FromUE(const UE::Math::TVector<double>& vec)
	{
		return Vector3(static_cast<float>(vec.X), static_cast<float>(vec.Y), static_cast<float>(vec.Z));
	}

	Quaternion FromUE(const FQuat& quat)
	{
		return Quaternion(quat.W, quat.X, quat.Y, quat.Z);
	}

	Vector4 FromUE(const FLinearColor& color)
	{
		return Vector4(color.R, color.G, color.B, color.A);
	}

} // namespace