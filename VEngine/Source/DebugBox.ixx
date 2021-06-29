module;
import Actor;
export module DebugBox;

//Debug representation of bounding box for actors.
export class DebugBox : public ActorSystem
{
public:
	DebugBox()
	{
		modelName = "cube.fbx";
		shaderName = "debugDraw.hlsl";
	}

	void Tick(float deltaTime)
	{
	}

	void Start()
	{
		Init<Actor>(1);
	}

	virtual void SpawnActors(int numToSpawn) { }
	virtual Actor* SpawnActor(Transform transform) { return nullptr; }

	//These transforms are linked to DebugDraw::DrawBox
	std::vector<Transform> debugBoxTransforms;
};

export DebugBox debugBox;
