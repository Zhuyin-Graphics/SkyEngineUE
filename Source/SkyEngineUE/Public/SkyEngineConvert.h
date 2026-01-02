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

} // namespace sky