#pragma once
#include ".\Library\GameObject.h"
#include "global.h"

class Enemy;

class  EnemyStateBase {
public:
	EnemyStateBase(Enemy* parent){enemy = parent;}
	virtual~EnemyStateBase(){}
	virtual void Update(Enemy& enemy)
	{
		enemy.Patroll();
	}
protected:
	Enemy* enemy;
};

class Enemy :
    public GameObject
{
private:
	EnemyStateBase* state_ = nullptr;
	EnemyStateBase* nextState_ = nullptr;
	friend class PatrollState;
	friend class ChaseState;
	friend class Attack;
	friend class Search;
public:
	int hImage_;//‰æ‘œID
	Point pos_;//ˆÊ’u
	DIR dir_;//ˆÚ“®•ûŒü
	void ChangeState(EnemyStateBase* nextState) {
		delete nextState_;
		nextState_ = nextState;
	}
	void ApplyStateChange();

	bool isFindPlayer();
	bool isAttackRange();
	bool isSearchTimeOver();
	enum State {
		Patroll,
		Chase,
		Attack,
		Search
	};
public:
	Enemy();
	~Enemy();
	void Update() {
		if (state_ != nullptr)
		{
			state_->Update(*this);
		}

		ApplyStateChange();
	}
	void Draw() override;
};

class PatrollState : public EnemyStateBase
{
public:
	PatrollState(Enemy*parent);
	~PatrollState();
	void Update(Enemy& enemy)override
	{
		enemy.Patroll();
		if (enemy.isFindPlayer())
		{
			enemy.ChangeState(new ChaseState());
		}
	}
};

class ChaseState : public EnemyStateBase
{
public:
	ChaseState();
	~ChaseState();
	void Update(Enemy& enemy)override
	{
		enemy.Chase();
		if (enemy.isAttackRange())
		{
			enemy.ChangeState(new AttackState);
		}
		if (enemy.isSearchTimeOver())
		{
			enemy.ChangeState(new SearchState);
		}
	}

};

class AttackState : public EnemyStateBase
{
public:
	AttackState();
	~AttackState();
	void Update(Enemy& enemy)override
	{
		enemy.Attack();
		if (enemy.isFindPlayer())
		{
			enemy.ChangeState(new ChaseState);
		}
	}
};

class SearchState : public EnemyStateBase
{
public:
	SearchState();
	~SearchState();
	void Update(Enemy& enemy)override
	{
		enemy.Search();
		if (enemy.isFindPlayer())
		{
			enemy.ChangeState(new PatrollState(this));
		}
	}
};