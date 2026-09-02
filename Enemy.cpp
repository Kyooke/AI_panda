#include "Enemy.h"
#include "time.h"
#include"Stage.h";

namespace
{
	const int ENEMY_SIZE = 48; //敵のサイズ 32*32
	const Point ENEMY_START_POS = { 20 * ENEMY_SIZE, 10 * ENEMY_SIZE }; //敵の初期位置
	const DIR INIT_ENEMY_DIR = { LEFT };
	const int ENEMY_DRAW_SIZE = 32; //敵の描画サイズ
	const int animFrame[4]{ 0, 1, 2, 1 };
	const float ANIM_INTERVAL = 0.2f;
}

Enemy::Enemy()
	: GameObject() 
{
	hImage_ = LoadGraph("Assets/panda_R.png");
	pos_ = ENEMY_START_POS; //32はブロックの位置pos_
	dir_ = INIT_ENEMY_DIR;
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
	static float prog_timer = 0.5f;

	float dt = Time::DeltaTime();
	prog_timer -= dt;

	if (prog_timer < 0.0f)
	{
		Point newPos = pos_;

		// 現在の向きに1マス進む
		switch (dir_)
		{
		case UP:
			newPos.y -= ENEMY_DRAW_SIZE;
			break;
		case RIGHT:
			newPos.x += ENEMY_DRAW_SIZE;
			break;
		case DOWN:
			newPos.y += ENEMY_DRAW_SIZE;
			break;

		case LEFT:
			newPos.x -= ENEMY_DRAW_SIZE;
			break;

		}

		// 壁判定
		bool hitWall =
			(newPos.x < 1 ||
				newPos.x >(STAGE_WIDTH - 2) * ENEMY_DRAW_SIZE ||
				newPos.y < 1 ||
				newPos.y >(STAGE_HEIGHT - 2) * ENEMY_DRAW_SIZE);

		if (hitWall)
		{
			dir_ = (DIR)((dir_ + 1) % 4);
		}
		else
		{
			// 移動
			pos_ = newPos;
		}
		int angle = 60;
		int r = ENEMY_DRAW_SIZE * 5;
		int cx = (int)(pos_.x + cosf(angle) * r);
		int cy = (int)(pos_.y + sinf(angle) * r);
		int mapValue = FindGameObject<Stage>()->GetMap(newPos.x / CHA_SIZE, ENEMY_DRAW_SIZE / CHA_SIZE);
		if (mapValue == 0)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
			DrawBox(pos_.x, pos_.y, pos_.x + CHA_SIZE, pos_.y + CHA_SIZE, GetColor(255, 0, 0), TRUE);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		prog_timer = 0.5f;
	}
	
}

void Enemy::Draw()
{
	static float animTimer = ANIM_INTERVAL;
	static int frame = 0;
	int nowFrame = animFrame[frame];

	Rect iRect[4] = {
		{  nowFrame * ENEMY_SIZE, 3 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 2 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 0 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 1 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
	};
	DrawBox(pos_.x, pos_.y, pos_.x + ENEMY_DRAW_SIZE, pos_.y + ENEMY_DRAW_SIZE,
		GetColor(255, 255, 0), FALSE,2);
	DrawRectExtendGraph(pos_.x, pos_.y,pos_.x + ENEMY_DRAW_SIZE, pos_.y + ENEMY_DRAW_SIZE,
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
bool Enemy::isFindPlayer()
{
	return false;
}

bool Enemy::isAttackRange()
{
	return false;
}

bool Enemy::isSearchTimeOver()
{
	return false;
}