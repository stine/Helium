#include "ExampleGamePch.h"

#include "Health.h"
#include "Framework/WorldManager.h"
#include "ExampleGame/Components/GameLogic/Dead.h"


using namespace Helium;
using namespace ExampleGame;

//////////////////////////////////////////////////////////////////////////
// HealthComponent

HELIUM_DEFINE_COMPONENT(ExampleGame::HealthComponent, 128);

void ExampleGame::HealthComponent::ApplyDamage( float m_DamageAmount )
{
	m_Health = Helium::Max(m_Health - m_DamageAmount, 0.0f);
	HELIUM_TRACE(
		TraceLevels::Debug, 
		"HealthComponent::ApplyDamage - Amount: %f New: %f\n", 
		m_DamageAmount, 
		m_Health);
}

void HealthComponent::PopulateMetaType( Reflect::MetaStruct& comp )
{

}

void HealthComponent::Initialize( const HealthComponentDefinition &definition )
{
	m_Health = ( definition.m_InitialHealth < 0.0f) ? definition.m_MaxHealth : definition.m_InitialHealth;
	m_MaxHealth = definition.m_MaxHealth;
	m_CreatedDeadComponent = false;
}

//////////////////////////////////////////////////////////////////////////
// HealthComponentDefinition

HELIUM_IMPLEMENT_ASSET(ExampleGame::HealthComponentDefinition, Components, 0);

void HealthComponentDefinition::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &HealthComponentDefinition::m_InitialHealth, "m_InitialHealth" );
	comp.AddField( &HealthComponentDefinition::m_MaxHealth, "m_MaxHealth" );
}

//////////////////////////////////////////////////////////////////////////

HELIUM_DEFINE_ABSTRACT_TASK(DoDamage);

void ExampleGame::DoDamage::DefineContract( TaskContract &rContract )
{
	rContract.ExecuteAfter<StandardDependencies::ProcessPhysics>();
	rContract.ExecuteBefore<StandardDependencies::Render>();
}

//////////////////////////////////////////////////////////////////////////

void DoKillAllWithZeroHealth( HealthComponent *pHealthComponent )
{
	if ( pHealthComponent->m_Health < HELIUM_EPSILON )
	{
		if (!pHealthComponent->m_CreatedDeadComponent && !pHealthComponent->GetComponentCollection()->GetFirst<DeadComponent>())
		{
			pHealthComponent->AllocateSiblingComponent<DeadComponent>();
		}

		pHealthComponent->m_CreatedDeadComponent = true;
	}
}

HELIUM_DEFINE_TASK( KillAllWithZeroHealth, ( ForEachWorld< QueryComponents< HealthComponent, DoKillAllWithZeroHealth > > ) )

void ExampleGame::KillAllWithZeroHealth::DefineContract( Helium::TaskContract &rContract )
{
	rContract.ExecuteAfter<ExampleGame::DoDamage>();
	rContract.ExecuteBefore<Helium::StandardDependencies::Render>();
}
