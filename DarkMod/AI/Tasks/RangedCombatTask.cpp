/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision$
 * $Date$
 * $Author$
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id$", init_version);

#include "RangedCombatTask.h"
#include "WaitTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& RangedCombatTask::GetName() const
{
	static idStr _name(TASK_RANGED_COMBAT);
	return _name;
}

void RangedCombatTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Init the base class
	CombatTask::Init(owner, subsystem);

	_enemy = owner->GetEnemy();
}

bool RangedCombatTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("RangedCombatTask performing.\r");

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	idActor* enemy = _enemy.GetEntity();
	if (enemy == NULL)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("No enemy, terminating task!\r");
		return false; // terminate me
	}

	// Can we damage the enemy already? (this flag is set by the combat state)
	if (owner->GetMemory().canHitEnemy)
	{
		idStr waitState(owner->WaitState());
		if (waitState != "ranged_attack")
		{
			// Waitstate is not matching, this means that the animation 
			// can be started.
			owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_RangedAttack", 5);
			owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_RangedAttack", 5);

			// greebo: Set the waitstate, this gets cleared by 
			// the script function when the animation is done.
			owner->SetWaitState("ranged_attack");

			if (_lastCombatBarkTime == -1)
			{
				EmitCombatBark(owner, "snd_combat_ranged");
			}
		}
		else
		{
			idAnimator* animator = owner->GetAnimatorForChannel(ANIMCHANNEL_LEGS);
			int animint = animator->CurrentAnim(ANIMCHANNEL_LEGS)->AnimNum();
			int length = animator->AnimLength(animint);

			int padding = gameLocal.random.RandomInt(4000) + 1000;

			owner->actionSubsystem->PushTask(TaskPtr(new WaitTask(length + padding)));
		}
	}

	return false; // not finished yet
}

void RangedCombatTask::OnFinish(idAI* owner)
{
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 5);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 5);
	owner->SetWaitState("");
}

void RangedCombatTask::Save(idSaveGame* savefile) const
{
	CombatTask::Save(savefile);
}

void RangedCombatTask::Restore(idRestoreGame* savefile)
{
	CombatTask::Restore(savefile);
}

RangedCombatTaskPtr RangedCombatTask::CreateInstance()
{
	return RangedCombatTaskPtr(new RangedCombatTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar rangedCombatTaskRegistrar(
	TASK_RANGED_COMBAT, // Task Name
	TaskLibrary::CreateInstanceFunc(&RangedCombatTask::CreateInstance) // Instance creation callback
);

} // namespace ai
