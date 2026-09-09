#include "Enemy.h"
#include "global.h"
#include "time.h"
#include "Stage.h"
#include "Player.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	const int ENEMY_SIZE = 48;
	const int MY_ENEMY_DRAW_SIZE = CHA_SIZE;
	const Point ENEMY_START_POS = { 20 * MY_ENEMY_DRAW_SIZE, 10 * MY_ENEMY_DRAW_SIZE };
	const DIR INIT_ENEMY_DIR = { LEFT };
	const int animFrame[4]{ 0, 1, 2, 1 };
	const float ANIM_INTERVAL = 0.2f;

	const int VISION_RANGE_GRID = 5;
	const int ATTACK_RANGE_GRID = 2;
	const float SEARCH_LIMIT_TIME = 4.0f;
}

Enemy::Enemy()
	: GameObject()
{
	hImage_ = LoadGraph("Assets/panda_R.png");
	pos_ = ENEMY_START_POS;
	oldPos_ = ENEMY_START_POS;
	dir_ = INIT_ENEMY_DIR;
	searchTimer_ = 0.0f;

	state_ = new PatrollState(this);
}

Enemy::~Enemy()
{
	delete state_;
	delete nextState_;
}

void Enemy::Update()
{
	if (state_ != nullptr)
	{
		state_->Update(*this);
	}
	ApplyStateChange();
}

void Enemy::Draw()
{
	static float animTimer = ANIM_INTERVAL;
	static int frame = 0;
	int nowFrame = animFrame[frame];

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 96);

	float originX = oldPos_.x + MY_ENEMY_DRAW_SIZE / 2.0f;
	float originY = oldPos_.y + MY_ENEMY_DRAW_SIZE / 2.0f;

	float visionRangePx = (float)(VISION_RANGE_GRID * MY_ENEMY_DRAW_SIZE);

	float baseAngle = 0.0f;
	switch (dir_)
	{
	case RIGHT: baseAngle = 0.0f;                   break;
	case DOWN:  baseAngle = 3.14159265f / 2.0f;     break;
	case LEFT:  baseAngle = 3.14159265f;            break;
	case UP:    baseAngle = -3.14159265f / 2.0f;    break;
	}

	float halfArc = 3.14159265f / 4.0f;

	int startGridX = (oldPos_.x / MY_ENEMY_DRAW_SIZE) - VISION_RANGE_GRID;
	int endGridX = (oldPos_.x / MY_ENEMY_DRAW_SIZE) + VISION_RANGE_GRID;
	int startGridY = (oldPos_.y / MY_ENEMY_DRAW_SIZE) - VISION_RANGE_GRID;
	int endGridY = (oldPos_.y / MY_ENEMY_DRAW_SIZE) + VISION_RANGE_GRID;

	for (int gy = startGridY; gy <= endGridY; ++gy)
	{
		for (int gx = startGridX; gx <= endGridX; ++gx)
		{
			int boxX1 = gx * MY_ENEMY_DRAW_SIZE;
			int boxY1 = gy * MY_ENEMY_DRAW_SIZE;
			float cellCenterX = boxX1 + MY_ENEMY_DRAW_SIZE / 2.0f;
			float cellCenterY = boxY1 + MY_ENEMY_DRAW_SIZE / 2.0f;

			float dx = cellCenterX - originX;
			float dy = cellCenterY - originY;
			float distSq = dx * dx + dy * dy;

			if (distSq <= visionRangePx * visionRangePx)
			{
				float angle = std::atan2(dy, dx);
				float diffAngle = angle - baseAngle;

				while (diffAngle > 3.14159265f)  diffAngle -= 2.0f * 3.14159265f;
				while (diffAngle < -3.14159265f) diffAngle += 2.0f * 3.14159265f;

				if (std::abs(diffAngle) <= halfArc)
				{
					DrawBox(boxX1, boxY1, boxX1 + MY_ENEMY_DRAW_SIZE, boxY1 + MY_ENEMY_DRAW_SIZE,
						GetColor(255, 0, 0), TRUE);
				}
			}
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	Rect iRect[4] = {
		{ nowFrame * ENEMY_SIZE, 3 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE },
		{ nowFrame * ENEMY_SIZE, 2 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE },
		{ nowFrame * ENEMY_SIZE, 0 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE },
		{ nowFrame * ENEMY_SIZE, 1 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE },
	};

	DrawBox(pos_.x, pos_.y, pos_.x + MY_ENEMY_DRAW_SIZE, pos_.y + MY_ENEMY_DRAW_SIZE, GetColor(255, 255, 0), FALSE, 2);
	DrawRectExtendGraph(pos_.x, pos_.y, pos_.x + MY_ENEMY_DRAW_SIZE, pos_.y + MY_ENEMY_DRAW_SIZE,
		iRect[dir_].x, iRect[dir_].y, iRect[dir_].w, iRect[dir_].h, hImage_, TRUE);

	if (animTimer < 0) {
		frame = (++frame) % 4;
		animTimer = ANIM_INTERVAL + animTimer;
	}
	animTimer = animTimer - Time::DeltaTime();
}

void Enemy::ApplyStateChange()
{
	if (nextState_ == nullptr)
	{
		return;
	}
	delete state_;
	state_ = nextState_;
	nextState_ = nullptr;
}

void Enemy::Patroll()
{
	static float moveTimer = 0.5f;
	moveTimer -= Time::DeltaTime();

	if (moveTimer <= 0.0f)
	{
		oldPos_ = pos_;

		Point newPos = pos_;
		switch (dir_)
		{
		case UP:    newPos.y -= MY_ENEMY_DRAW_SIZE; break;
		case RIGHT: newPos.x += MY_ENEMY_DRAW_SIZE; break;
		case DOWN:  newPos.y += MY_ENEMY_DRAW_SIZE; break;
		case LEFT:  newPos.x -= MY_ENEMY_DRAW_SIZE; break;
		}

		bool hitWall = (newPos.x < 1 ||
			newPos.x >(STAGE_WIDTH - 2) * MY_ENEMY_DRAW_SIZE ||
			newPos.y < 1 ||
			newPos.y >(STAGE_HEIGHT - 2) * MY_ENEMY_DRAW_SIZE);

		if (hitWall)
		{
			dir_ = (DIR)((dir_ + 1) % 4);
		}
		else
		{
			pos_ = newPos;
		}

		moveTimer = 0.5f;
	}
}

void Enemy::Chase()
{
	static float moveTimer = 0.3f;
	moveTimer -= Time::DeltaTime();

	if (moveTimer <= 0.0f)
	{
		Player* player = FindGameObject<Player>();
		if (player != nullptr)
		{
			oldPos_ = pos_;

			Point playerPos = player->GetPlayerPos();
			int diffX = playerPos.x - pos_.x;
			int diffY = playerPos.y - pos_.y;

			if (std::abs(diffX) > std::abs(diffY))
			{
				dir_ = (diffX > 0) ? RIGHT : LEFT;
			}
			else
			{
				dir_ = (diffY > 0) ? DOWN : UP;
			}

			switch (dir_)
			{
			case UP:    pos_.y -= MY_ENEMY_DRAW_SIZE; break;
			case RIGHT: pos_.x += MY_ENEMY_DRAW_SIZE; break;
			case DOWN:  pos_.y += MY_ENEMY_DRAW_SIZE; break;
			case LEFT:  pos_.x -= MY_ENEMY_DRAW_SIZE; break;
			}
		}
		moveTimer = 0.3f;
	}
}

void Enemy::Attack()
{
	static float rotTimer = 0.1f;
	rotTimer -= Time::DeltaTime();

	if (rotTimer <= 0.0f)
	{
		dir_ = (DIR)((dir_ + 1) % 4);
		rotTimer = 0.1f;
	}
}

void Enemy::Search()
{
	searchTimer_ += Time::DeltaTime();

	static float moveTimer = 0.5f;
	moveTimer -= Time::DeltaTime();

	if (moveTimer <= 0.0f)
	{
		oldPos_ = pos_;

		if (rand() % 2 == 0)
		{
			dir_ = (DIR)(rand() % 4);
		}

		Point newPos = pos_;
		switch (dir_)
		{
		case UP:    newPos.y -= MY_ENEMY_DRAW_SIZE; break;
		case RIGHT: newPos.x += MY_ENEMY_DRAW_SIZE; break;
		case DOWN:  newPos.y += MY_ENEMY_DRAW_SIZE; break;
		case LEFT:  newPos.x -= MY_ENEMY_DRAW_SIZE; break;
		}

		bool hitWall = (newPos.x < 1 ||
			newPos.x >(STAGE_WIDTH - 2) * MY_ENEMY_DRAW_SIZE ||
			newPos.y < 1 ||
			newPos.y >(STAGE_HEIGHT - 2) * MY_ENEMY_DRAW_SIZE);

		if (!hitWall)
		{
			pos_ = newPos;
		}

		moveTimer = 0.5f;
	}
}

int Enemy::GetPlayerDistanceGrid()
{
	Player* player = FindGameObject<Player>();
	if (player == nullptr) return 999;

	Point playerPos = player->GetPlayerPos();
	int gridX = std::abs(playerPos.x - pos_.x) / MY_ENEMY_DRAW_SIZE;
	int gridY = std::abs(playerPos.y - pos_.y) / MY_ENEMY_DRAW_SIZE;

	return gridX + gridY;
}

bool Enemy::isFindPlayer()
{
	return GetPlayerDistanceGrid() <= VISION_RANGE_GRID;
}

bool Enemy::isAttackRange()
{
	return GetPlayerDistanceGrid() <= ATTACK_RANGE_GRID;
}

bool Enemy::isSearchTimeOver()
{
	return searchTimer_ >= SEARCH_LIMIT_TIME;
}

void PatrollState::Update(Enemy& enemy)
{
	enemy.Patroll();
	if (enemy.isFindPlayer())
	{
		enemy.ChangeState(new ChaseState(&enemy));
	}
}

void ChaseState::Update(Enemy& enemy)
{
	enemy.Chase();
	if (enemy.isAttackRange())
	{
		enemy.ChangeState(new AttackState(&enemy));
	}
	else if (!enemy.isFindPlayer())
	{
		enemy.ChangeState(new SearchState(&enemy));
	}
}

void AttackState::Update(Enemy& enemy)
{
	enemy.Attack();
	if (!enemy.isAttackRange())
	{
		enemy.ChangeState(new SearchState(&enemy));
	}
}

void SearchState::Update(Enemy& enemy)
{
	enemy.Search();
	if (enemy.isFindPlayer() && enemy.isAttackRange())
	{
		enemy.ChangeState(new AttackState(&enemy));
	}
	else if (enemy.isSearchTimeOver())
	{
		enemy.ChangeState(new PatrollState(&enemy));
	}
}