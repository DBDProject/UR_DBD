#include "D1GameplayTags.h"

namespace D1GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Move, "Input.Action.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Look, "Input.Action.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Run, "Input.Action.Run");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Crouch, "Input.Action.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Attack1, "Input.Action.Attack1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_RightClick, "Input.Action.RightClick");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_SpaceBar, "Input.Action.SpaceBar");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Skill1, "Input.Action.Skill1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Interact, "Input.Action.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Parkour, "Input.Action.Parkour");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_TestInput, "Input.Action.TestInput");
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_ItemUsage, "Input.Action.ItemUsage");

	UE_DEFINE_GAMEPLAY_TAG(Event_Transform_Begin, "Event.Transform.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Transform_End, "Event.Transform.End");

	UE_DEFINE_GAMEPLAY_TAG(Killer_Attack_DetactStart, "Killer.Attack.DetactStart");
	UE_DEFINE_GAMEPLAY_TAG(Killer_Attack_DetactEnd, "Killer.Attack.DetactEnd");
	UE_DEFINE_GAMEPLAY_TAG(Killer_PalletEnd, "Killer.PalletEnd");
	UE_DEFINE_GAMEPLAY_TAG(Killer_PickUpEnd, "Killer.PickUpEnd");
	UE_DEFINE_GAMEPLAY_TAG(Killer_HookEnd, "Killer.HookEnd");
	UE_DEFINE_GAMEPLAY_TAG(Killer_VaultWindowEnd, "Killer.VaultWindowEnd");

	UE_DEFINE_GAMEPLAY_TAG(Killer_Ability_Attack, "Killer.Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Killer_Ability_Transform, "Killer.Ability.Transform");
	UE_DEFINE_GAMEPLAY_TAG(Killer_Ability_DamageGenerator, "Killer.Ability.DamageGenerator");

}