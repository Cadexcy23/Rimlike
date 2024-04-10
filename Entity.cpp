#include "Entity.h"
#include <vector>
#include "Artist.h"
#include "Map.h"
#include <SDL.h>
#include "Controller.h"

std::vector<SDL_Texture*> Entity::gSelected;

void Entity::initEntity()
{
	Artist artist;

	//load the selected graphic
	Entity::gSelected.push_back(artist.loadTexture("Resource/entities/selected0.png"));
	Entity::gSelected.push_back(artist.loadTexture("Resource/entities/selected1.png"));
	Entity::gSelected.push_back(artist.loadTexture("Resource/entities/selected2.png"));
	Entity::gSelected.push_back(artist.loadTexture("Resource/entities/selected3.png"));
}

void checkCollision(Entity::projectile* e) //can prolly do with some good optimizing, like cell based checks so we dont check everything on the map, also maybe make pros out of map bouns not crash game xd
{
	//check all entities
	for (int j = 0; j < Map::activeMap.entityList.entities.size(); j++)
	{
		if (e->radiusCollisionDetect({ int(Map::activeMap.entityList.entities[j].x), int(Map::activeMap.entityList.entities[j].y) }, Map::activeMap.entityList.entities[j].size / 2))
		{
			for (int i = 0; i < e->collideList.size(); i++)
			{
				e->collideList[i](e, 0, j);
			}
		}
	}
	//pawns
	for (int j = 0; j < Map::activeMap.entityList.pawns.size(); j++)
	{
		if (e->radiusCollisionDetect({ int(Map::activeMap.entityList.pawns[j].x), int(Map::activeMap.entityList.pawns[j].y) }, Map::activeMap.entityList.pawns[j].size / 2))
		{
			if (!(e->ownerType == 1 && e->ownerID == j))
			{
				for (int i = 0; i < e->collideList.size(); i++)
				{
					e->collideList[i](e, 1, j);
				}
			}
		}
	}
	//projectiles
	for (int j = 0; j < Map::activeMap.entityList.projectiles.size(); j++)
	{
		if (e->radiusCollisionDetect({ int(Map::activeMap.entityList.projectiles[j].x), int(Map::activeMap.entityList.projectiles[j].y) }, Map::activeMap.entityList.projectiles[j].size / 2))
		{
			if (!(e->angle == Map::activeMap.entityList.projectiles[j].angle && e->x == Map::activeMap.entityList.projectiles[j].x && e->y == Map::activeMap.entityList.projectiles[j].y))
			{
				for (int i = 0; i < e->collideList.size(); i++)
				{
					e->collideList[i](e, 2, j);
				}
			}
		}
	}
	//check for tile we are on
	if (Map::activeMap.tileGrid[e->x / 64][e->y / 64].tileType->collision)
	{
		for (int i = 0; i < e->collideList.size(); i++)
		{
			e->collideList[i](e, 3, -1);
		}
	}
}

//Functions for basic entities

//Functions for pawn entities
void Entity::moveToGoal(Entity::pawn *e)
{
	//check if we even need to try moving
	if (e->x != e->goal.x || e->y != e->goal.y)
	{
		//if path is clear dont try to move
		if (e->path.size() > 0)
		{
			//get size of path so we can look at last one 
			int size = e->path.size() - 1;
			//get direction to move
			SDL_Point dir = { 0, 0 };
			if (abs(e->path[size].x - e->x))
				dir.x = (e->path[size].x - e->x) / abs(e->path[size].x - e->x);
			if (abs(e->path[size].y - e->y))
				dir.y = (e->path[size].y - e->y) / abs(e->path[size].y - e->y);

			//if we are within our speed of the path pos set it to the path pos
			if (abs(e->x - e->path[size].x) <= e->speed)
			{
				e->x = e->path[size].x;
			}
			else //else add speed as normal
			{
				//add dir*speed to pos
				e->x += dir.x * e->speed;
			}
			if (abs(e->y - e->path[size].y) <= e->speed)
			{
				e->y = e->path[size].y;
			}
			else
			{
				e->y += dir.y * e->speed;
			}

			//when we reach each point on the path clear it
			if (e->x == e->path[size].x && e->y == e->path[size].y)
			{
				e->path.pop_back();
			}
		}
	}
}

//Functions for projectile entities

//Updates
void Entity::moveForward(Entity::projectile *e)
{
	for (int i = 0; i < e->velocity; i++)
	{
		if (e->lifespan > 0)
		{
			checkCollision(e);

			//move forward based on angle
			//convert to radians
			double theta = e->angle * (M_PI / 180);

			//transform vel to be relative to the angle
			float addX = 0 * cos(theta) + 1 * sin(theta);
			float addY = 0 * sin(theta) - 1 * cos(theta);

			e->x += addX;
			e->y += addY;
		}
	}
}

//Collides
void Entity::makeBabys(Entity::projectile* e)
{
	//get inversed angle then move the spawn a little back from the point of expiration so they dont spawn in the wall
	float invAng = e->angle - 180;

	//convert to radians
	double theta = invAng * (M_PI / 180);

	//transform vel to be relative to the angle
	float addX = 0 * cos(theta) + 2 * sin(theta);
	float addY = 0 * sin(theta) - 2 * cos(theta);

	//how many to spawn
	float count = 160;
	//get the counth fraction of 360
	float addAng = 360 / count;

	//make a new projectile
	projectile newPro = projectile(e->x + addX, e->y + addY, 1, e->angle, "babyBullie", e->tex, 1, 60, -1, -1, { moveForward }, { stopAtWall, pushHit }, {});

	for (int i = 0; i < count; i++)
	{
		newPro.angle += addAng;
		Map::activeMap.entityList.projectiles.push_back(newPro);
	}
}

void Entity::destroyHit(Entity::projectile* e, int hitType, int hitID)
{
	//destroy the thing we hit
	switch (hitType)
	{
		case 0:
			Map::activeMap.entityList.entities.erase(Map::activeMap.entityList.entities.begin() + hitID);
			break;
		case 1:
			Map::activeMap.entityList.pawns.erase(Map::activeMap.entityList.pawns.begin() + hitID);
			break;
		case 2:
			Map::activeMap.entityList.projectiles.erase(Map::activeMap.entityList.projectiles.begin() + hitID);
			break;
	}
}

void Entity::spinHit(Entity::projectile* e, int hitType, int hitID)
{
	//spin the thing we hit
	switch (hitType)
	{
	case 0:
		Map::activeMap.entityList.entities[hitID].angle++;
		break;
	case 1:
		Map::activeMap.entityList.pawns[hitID].angle++;
		break;
	case 2:
		//Map::activeMap.entityList.projectiles[hitID].angle++;
		break;
	}
}

void Entity::pushHit(Entity::projectile* e, int hitType, int hitID)
{
	//convert to radians
	double theta = e->angle * (M_PI / 180);

	//transform vel to be relative to the angle
	float addX = 0 * cos(theta) + .5 * sin(theta);
	float addY = 0 * sin(theta) - .5 * cos(theta);

	//spin the thing we hit
	switch (hitType)
	{
	case 0:
		Map::activeMap.entityList.entities[hitID].x += addX;
		Map::activeMap.entityList.entities[hitID].y += addY;
		break;
	case 1:
		Map::activeMap.entityList.pawns[hitID].x += addX;
		Map::activeMap.entityList.pawns[hitID].y += addY;
		break;
	case 2:
		//Map::activeMap.entityList.projectiles[hitID].x++;
		break;
	}
}

void Entity::stopAtWall(Entity::projectile* e, int hitType, int hitID)
{
	switch (hitType)
	{
	case 0:
		
		break;
	case 1:
		
		break;
	case 2:
		
		break;
	case 3:
		e->lifespan = 0;

		break;
	}
}

