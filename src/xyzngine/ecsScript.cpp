#include "ecsScript.h"
#include "ecsSceneObject.h"
#include "ecsComponents.h"

void register_ecs_script_systems(flecs::world* ecs)
{
	static auto scriptSystemQuery = ecs->query<ScriptSystemPtr>();

	ecs->system<ScriptComponent, const Position>()
		.each([&](flecs::entity e, ScriptComponent& scriptNode, const Position& pos)
			{
				scriptNode.ptr->Update(e.delta_time());
			});
}
