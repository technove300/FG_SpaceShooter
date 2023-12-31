#include "Graphics.h"
#include "Physics.h"
#include "Game.h"
#include <stdio.h>
#include <queue>

using hlslpp::float2;
using hlslpp::float1;

std::queue<Line> FrameLines;

const float spread = 0.01f;

float2 PlayerShip::Position;

PlayerShip::PlayerShip()
{
	collider = GetNewCollider();
	ResetPos();
	shooting = false;
	dead = false;
	score = 0;

	deadDelay = 500;
	deadTimer = deadDelay;
}

void PlayerShip::ResetPos()
{
	vm.StopMovement();
	position = float2(1.0f, 1.9f);
	angle = -0.225;
	sprite.Set(position, 0.0, 0);

	Collider* col = GetCollider(collider);
	if (col != NULL)
	{
		col->Set(position, float1(0.03f));
	}
}

void PlayerShip::Fire()
{
	if (dead) return;
	if (!shooting)
	{
		timer = 25.0f;
		shooting = true;
		FrameLines.emplace();
		Line* f = &FrameLines.back();

		float ang = angle;
		ang += ((((rand() % 200) - 100) / 100.0f) * spread );

		float2 fireVec = float2(cos(ang * 6.28), sin(ang * 6.28));
		fireVec = hlslpp::normalize(fireVec);

		Enemy* e = PhysicsQueryEnemy(position, fireVec);

		if (e != NULL)
		{
			e->Destroy();
			score += 200;
		}


		fireVec = fireVec * 3.0f;
		fireVec = position + fireVec;
		f->Set(position, fireVec);
	}
}

void PlayerShip::Thruster()
{
	if (dead) return;
	float2 fireVec = float2(cos(angle * 6.28), sin(angle * 6.28));

	fireVec = hlslpp::normalize(fireVec);

	fireVec /= 1000;

	fireVec *= (0.005f * deltaTime);

	vm.AddVector(fireVec);
	sprite.Set(1);
}

void PlayerShip::Update(float deltaTime)
{
	if (clearTimer < 0.0f)
	{
		if (!FrameLines.empty())
		{
			FrameLines.pop();
		}
		clearTimer = 50.0f;
	}
	else if (clearTimer >= 0.0f)
	{
		clearTimer = clearTimer - deltaTime;
	}

	if (timer < 0.0f && shooting)
	{
		if (FrameLines.size() < 3) shooting = false;
	}
	else if (timer >= 0.0f && shooting)
	{
		timer = timer - deltaTime;
	}

	if (scoreDisplay != score)
	{
		scoreDisplay = scoreDisplay + (((score - scoreDisplay) / 2) * (deltaTime / 100));
		if ((score - scoreDisplay) < 20) scoreDisplay = score;
	}

	if (dead)
	{
		deadTimer = deadTimer - deltaTime;

		float part = deadDelay / 3.0f;

		bool ret = true;

		if (deadTimer < 0.0f)
		{
			deadTimer = deadDelay;
			ret = false;
			dead = false;
			sprite.Hide = false;
			Collider* col = GetCollider(collider);
			if (col != NULL)
			{
				col->Skip = false;
			}
			
		}
		else if (deadTimer > (2 * part))
		{
			sprite.Set(4);
		}
		else if (deadTimer > part)
		{
			sprite.Set(5);
		}
		else if (deadTimer < part && !sprite.Hide)
		{
			sprite.Hide = true;
			sprite.Set(0);
			ResetPos();
			nuke = true;
		}


		if (ret) return;
	}


	sprite.Set(position, angle);

	Collider* col = GetCollider(collider);
	if (col != NULL)
	{
		if (col->CollisionFlag && col->CollisionType == COL_ENEMY)
		{
			dead = true;
			col->Skip = true;
			col->CollisionFlag = false;
		}

		position = vm.Update(position, deltaTime, col);
		col->Set(position, vm.velocity);
	}

	Position = position;
}