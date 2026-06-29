#include "Core/TSGameplayTags.h"

namespace TSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sprinting, "State.Movement.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sliding, "State.Movement.Sliding");
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Crouching, "State.Movement.Crouching");

	UE_DEFINE_GAMEPLAY_TAG(State_Combat_ADS, "State.Combat.ADS");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Firing, "State.Combat.Firing");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Reloading, "State.Combat.Reloading");

	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Switching, "State.Weapon.Switching");

	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Fire, "Ability.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ADS, "Ability.ADS");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reload, "Ability.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint, "Ability.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Slide, "Ability.Slide");
	UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponSwitch, "Ability.WeaponSwitch");

	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Hit, "Event.Combat.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Kill, "Event.Combat.Kill");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_TakeDamage, "Event.Combat.TakeDamage");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Death, "Event.Combat.Death");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_Slot_Primary, "Weapon.Slot.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Slot_Secondary, "Weapon.Slot.Secondary");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Slide, "Cooldown.Slide");

	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
}
