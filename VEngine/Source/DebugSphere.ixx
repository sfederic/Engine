module;
import Actor;
export module DebugSphere;

//Debug representation of bounding sphere for actors.
export class DebugSphere : public ActorSystem
{
public:
	DebugSphere();
	DebugSphere::DebugSphere()
	{
		shaderName = "debugDraw.hlsl";
		modelName = "ico_sphere.fbx";
	}

	virtual void DebugSphere::Tick(float deltaTime)
	{

	}

	virtual void DebugSphere::Start()
	{
		Init<Actor>(1);
	}
	virtual void SpawnActors(int numToSpawn) {}
	virtual Actor* SpawnActor(Transform transform) { return nullptr; }

	//These transforms are linked to DebugDraw::DrawSphere
	std::vector<Transform> debugSphereTransforms;
};

export DebugSphere debugSphere;
