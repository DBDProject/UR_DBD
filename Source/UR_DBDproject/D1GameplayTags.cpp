#include "D1GameplayTags.h"

namespace D1GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Move, "Input.Action.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Look, "Input.Action.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Run, "Input.Action.Run");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Crouch, "Input.Action.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Attack1, "Input.Action.Attack1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_RightClick, "Input.Action.RightClick");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Skill1, "Input.Action.Skill1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Interact, "Input.Action.Interact");


	UE_DEFINE_GAMEPLAY_TAG(Event_Transform_Begin, "Event.Transform.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Transform_End, "Event.Transform.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Begin, "Event.Attack.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_End, "Event.Attack.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_DetactStart, "Event.Attack.DetactStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_DetactEnd, "Event.Attack.DetactEnd");
}