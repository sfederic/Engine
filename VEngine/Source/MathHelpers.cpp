#include "Actor.h"
#include "MathHelpers.h"
#include <math.h>

void MatrixAddScale(float s, XMMATRIX& m)
{
    m.r[0].m128_f32[0] = s;
    m.r[1].m128_f32[1] = s;
    m.r[2].m128_f32[2] = s;
}

//GLOBAL VECTOR DIRECTIONS (X+ = Right, Y+ = Up, Z+ = forward)
XMVECTOR XMVectorRight()
{
    return XMVectorSet(1.f, 0.f, 0.f, 0.f);
}

XMVECTOR XMVectorUp()
{
    return XMVectorSet(0.f, 1.f, 0.f, 0.f);
}

XMVECTOR XMVectorForward()
{
    return XMVectorSet(0.f, 0.f, 1.f, 0.f);
}

/*XMVECTOR XMVectorConstantLerp(FXMVECTOR V0, FXMVECTOR V1, float deltaTime, float speed)
{
    XMVECTOR v = XMVectorLerp(V0, V1, deltaTime);
    return v * speed;
}*/

XMFLOAT3 PitchYawRollFromMatrix(XMMATRIX m)
{
    //REF: https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Conversion_formulae_between_formalisms
    float pitch = acosf(m.r[2].m128_f32[2]);
    float yaw = atan2f(m.r[0].m128_f32[2], m.r[1].m128_f32[2]);
    float roll = atan2f(m.r[2].m128_f32[0], m.r[2].m128_f32[1]);

    pitch = XMConvertToDegrees(pitch);
    yaw = XMConvertToDegrees(yaw);
    roll = XMConvertToDegrees(roll);

    return XMFLOAT3(pitch, yaw, roll);
}   

XMFLOAT3 PitchYawRollFromQuaternion(XMFLOAT4 q)
{
    XMMATRIX m = XMMatrixRotationQuaternion(XMLoadFloat4(&q));
    return XMFLOAT3(PitchYawRollFromMatrix(m));
}

void LookAtRotation(XMVECTOR lookAtPoint, XMMATRIX& m)
{
    //lookAtPoint.m128_f32[3] = 0.f;
    XMVECTOR forward = XMVectorAdd(lookAtPoint, m.r[3]);
    forward = XMVector3Normalize(forward);

    XMVECTOR right = XMVector3Cross(XMVectorUp(), forward);
    right = XMVector3Normalize(right);

    XMVECTOR up = XMVector3Cross(forward, right); //Already normalised.
    m.r[0] = right;
    m.r[1] = up;
    m.r[2] = forward;
}

XMFLOAT4X4 FbxMatrixToDirectXMathMatrix(fbxsdk::FbxMatrix fbxMatrix)
{
    //TODO: Fbx likes to work in units of 100. Either do the division or make like UE4.
    return XMFLOAT4X4(fbxMatrix.mData[0].mData[0] / 100.0, fbxMatrix.mData[0].mData[1] / 100.0, fbxMatrix.mData[0].mData[2] / 100.0, fbxMatrix.mData[0].mData[3] / 100.0,
        fbxMatrix.mData[1].mData[0] / 100.0, fbxMatrix.mData[1].mData[1] / 100.0, fbxMatrix.mData[1].mData[2] / 100.0, fbxMatrix.mData[1].mData[3] / 100.0,
        fbxMatrix.mData[2].mData[0] / 100.0, fbxMatrix.mData[2].mData[1] / 100.0, fbxMatrix.mData[2].mData[2] / 100.0, fbxMatrix.mData[2].mData[3] / 100.0,
        fbxMatrix.mData[3].mData[0] / 100.0, fbxMatrix.mData[3].mData[1] / 100.0, fbxMatrix.mData[3].mData[2] / 100.0, fbxMatrix.mData[3].mData[3] / 100.0);
}

bool VecEqual(XMVECTOR v1, XMVECTOR v2, float epsilon)
{
    if (v1.m128_f32[0] <= (v2.m128_f32[0] + epsilon) && v1.m128_f32[0] >= (v2.m128_f32[0] - epsilon) &&
        v1.m128_f32[1] <= (v2.m128_f32[1] + epsilon) && v1.m128_f32[1] >= (v2.m128_f32[1] - epsilon) && 
        v1.m128_f32[2] <= (v2.m128_f32[2] + epsilon) && v1.m128_f32[2] >= (v2.m128_f32[2] - epsilon) &&
        v1.m128_f32[3] <= (v2.m128_f32[3] + epsilon) && v1.m128_f32[3] >= (v2.m128_f32[3] - epsilon))
    {
        return true;
    }

    return false;
}

bool Float3Equal(XMFLOAT3& f1, XMFLOAT3& f2)
{
    if (f1.x == f2.x && f1.y == f2.y && f1.z == f2.z)
    {
        return true;
    }

    return false;
}

float FindMaxInVector(XMVECTOR v)
{
    float max = 0.f;
    for (int i = 0; i < 4; i++)
    {
        if (max < v.m128_f32[i])
        {
            max = v.m128_f32[i];
        }
    }

    return max;
}

XMVECTOR XMVectorConstantLerp(XMVECTOR current, XMVECTOR target, float dist)
{
    XMVECTOR toTarget = XMVectorSubtract(target, current);
    float magnitude = XMVector3Dot(toTarget, toTarget).m128_f32[0];
    if (magnitude > (dist * dist))
    {
        toTarget *= (dist / sqrtf(magnitude));
    }

    return current + toTarget;
}

XMMATRIX GetBoundingBoxMatrix(BoundingOrientedBox& boundingBox, Actor* actor)
{
    XMFLOAT3 actorPosFloat3 = actor->GetPositionFloat3();
    XMVECTOR actorPos = XMLoadFloat3(&actorPosFloat3);
    XMVECTOR boundingBoxCenter = XMLoadFloat3(&boundingBox.Center);
    XMVECTOR offset = actorPos + boundingBoxCenter;
    offset.m128_f32[3] = 1.0f;

    XMFLOAT3 actorScaleFloat3 = actor->GetScale();
    XMVECTOR actorScale = XMLoadFloat3(&actorScaleFloat3);
    XMVECTOR extents = XMLoadFloat3(&boundingBox.Extents);
    extents.m128_f32[3] = 1.0f;

    XMMATRIX boxBoundsMatrix = XMMatrixIdentity();
    boxBoundsMatrix = actor->GetTransformationMatrix(); //This is more just to get the rotation
    boxBoundsMatrix *= XMMatrixScalingFromVector(extents);
    boxBoundsMatrix.r[3] = offset;

    return boxBoundsMatrix;
}

void UpdateBoundingBox(BoundingOrientedBox& boundingBox, Actor* actor)
{
    XMFLOAT3 actorPosFloat3 = actor->GetPositionFloat3();
    XMVECTOR actorPos = XMLoadFloat3(&actorPosFloat3);
    XMVECTOR boundingBoxCenter = XMLoadFloat3(&boundingBox.Center);
    XMVECTOR offset = actorPos + boundingBoxCenter;
    offset.m128_f32[3] = 1.0f;

    XMFLOAT3 actorScaleFloat3 = actor->GetScale();
    XMVECTOR actorScale = XMLoadFloat3(&actorScaleFloat3);
    XMVECTOR extents = XMLoadFloat3(&boundingBox.Extents);
    XMVECTOR scale = extents * actorScale;
    scale.m128_f32[3] = 1.0f;

    XMFLOAT4 actorRotFloat4 = actor->GetRotationQuat();
    XMVECTOR orientation = XMLoadFloat4(&actorRotFloat4);

    XMStoreFloat3(&boundingBox.Center, offset);
    XMStoreFloat3(&boundingBox.Extents, scale);
    XMStoreFloat4(&boundingBox.Orientation, orientation);
}
