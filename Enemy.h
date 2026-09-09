#pragma once
#include "Library/GameObject.h"
#include "global.h"

class EnemyStateBase;

class Enemy : public GameObject
{
private:
	EnemyStateBase* state_ = nullptr;
	EnemyStateBase* nextState_ = nullptr;

	int hImage_;
	Point pos_;    
	Point oldPos_; 
	DIR dir_;
	float searchTimer_;

public:
	Enemy();
	~Enemy();

	void Update() override;
	void Draw() override;

	void ChangeState(EnemyStateBase* nextState)
	{
		delete nextState_;
		nextState_ = nextState;
	}

	void ApplyStateChange(); 

	Point GetPos() const { return pos_; }
	Point GetOldPos() const { return oldPos_; }
	DIR GetDir() const { return dir_; }
	void SetDir(DIR dir) { dir_ = dir; }

	void SetPos(Point newPos)
	{
		oldPos_ = pos_;
		pos_ = newPos;
	}

	int GetPlayerDistanceGrid();
	bool isFindPlayer();
	bool isAttackRange();
	bool isSearchTimeOver();

	void Patroll();
	void Chase();
	void Attack();
	void Search();
};

class EnemyStateBase
{
public:
	virtual ~EnemyStateBase() {}
	virtual void Update(Enemy& enemy) = 0;
};

class PatrollState : public EnemyStateBase
{
public:
	PatrollState(Enemy* enemy) {}
	void Update(Enemy& enemy) override;
};

class ChaseState : public EnemyStateBase
{
public:
	ChaseState(Enemy* enemy) {}
	void Update(Enemy& enemy) override;
};

class AttackState : public EnemyStateBase
{
public:
	AttackState(Enemy* enemy) {}
	void Update(Enemy& enemy) override;
};

class SearchState : public EnemyStateBase
{
public:
	SearchState(Enemy* enemy) {}
	void Update(Enemy& enemy) override;
};