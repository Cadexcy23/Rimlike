#pragma once
#ifndef string
#include <string>
#endif
#ifndef SDL
#include <SDL.h>
#endif // !SDL
#ifndef vector
#include <vector>
#endif
#ifndef Artist
#include "Artist.h"
#endif
#ifndef Pathfinder
#include "Pathfinder.h"
#endif
#ifndef Time
#include <time.h>
#endif







class Entity {

	static std::vector<SDL_Texture*> gSelected;

public:

	struct entity {
		std::string name;
		int size;
		float x;
		float y;
		SDL_Texture* tex;
		float angle;
		std::vector<void (*)(entity* e)> updateList; //functions that the entity will run every tick of the game, need to populate this list after the ent is created

		entity(float inX, float inY, int inSize, float inAngle, std::string inName, SDL_Texture* inTex, std::vector<void (*)(entity* e)> upList)
		{
			name = inName;
			size = inSize;
			x = inX;
			y = inY;
			tex = inTex;
			angle = inAngle;
			updateList = upList;
		}

		entity() = default;

		bool radiusCollisionDetect(SDL_Point p, float r)
		{
			r += size / 2;
			if (size > 0 && r < 1)
			{
				r = 1;
			}
			return ((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y) < r * r);
		}

		void draw(SDL_Point off)
		{
			//have a func here so when drawing becomes more complex we dont have to manage it elsewhere like multipart ents or something

			Artist artist;

			//move the ent up left by half its width and height to center it
			SDL_Point centering;
			SDL_QueryTexture(tex, NULL, NULL, &centering.x, &centering.y);
			centering.x /= 2;
			centering.y /= 2;
			artist.drawImage(tex, x - off.x - centering.x, y - off.y - centering.y, 0, 0, angle);
		}

		//std::vector<SDL_Point> getPerimeter()
		//{
		//	//use the center of the ent to rotate a point its radius higher than it around itself at spaced degrees to get a perimete aroind the ent
		//	//maybe draw lines from point to point anc look for intersecting collision
		//
		//	//convert rot to radians
		//	//float rads = activeMap.camRot * (M_PI / 180);
		//	//
		//	////create rotation matrix based off camRot
		//	//float R[4];
		//	//R[0] = R[3] = cos(rads), R[1] = -(R[2] = -sin(rads));
		//	//
		//	////use matrix to rotate 
		//	//float p[2] = { newGoal.x, newGoal.y };
		//	//float o[2] = { activeMap.camPosX, activeMap.camPosY };
		//	//float t0 = p[0] - o[0], t1 = p[1] - o[1];
		//	//p[0] = R[0] * t0 + R[1] * t1 + o[0], p[1] = R[2] * t0 + R[3] * t1 + o[1];
		//	//
		//	////set goal to the rotated points
		//	//returnPoint.x = p[0];
		//	//returnPoint.y = p[1];
		//}
	};

	struct projectile : public entity {
		float velocity;
		int lifespan; //decrement each update til 0
		std::vector<void (*)(projectile* e)> updateList;
		std::vector<void (*)(projectile* e, int hitType, int hitID)> collideList; //funcs that run when we collide with something
		std::vector<void (*)(projectile* e)> expireList; //funcs that run when the projectile expires
		int ownerType; // 0 ent 1 pawn 2 pro 3 tile
		int ownerID;

		projectile(float inX, float inY, int inSize, float inAngle, std::string inName, SDL_Texture* inTex, float vel, int life, int inOwnerType, int inOwnerID, std::vector<void (*)(projectile* e)> upList, std::vector<void (*)(projectile* e, int hitType, int hitID)> coList, std::vector<void (*)(projectile* e)> expList)
		{
			Artist artist;

			//projectile specific
			velocity = vel;
			lifespan = life;
			ownerType = inOwnerType;//May lead to problems later if we are deleting owners maybe make some sort of identifying key, maybe just a arbitrary number
			ownerID = inOwnerID;
			updateList = upList;
			collideList = coList;
			expireList = expList;

			//basic
			name = inName;
			size = inSize;
			x = inX;
			y = inY;
			tex = inTex;
			angle = inAngle;
		}

		projectile() = default;
	};

	struct ammo {
		projectile proType; //make a pointer maybe when we have a master list of ammo?
		//tex
		//max stack
		//caliber

		ammo(projectile pro)
		{
			proType = pro;
		}

		ammo() = default;
	};

	struct gun {
		int loadedAmmoID;
		//tex
		//modifiers
		//velocity
		//damage
		//lifespan

		gun()
		{
			loadedAmmoID = 0;
		}

		//gun() = default;
	};

	struct pawn : public entity {
		float speed;
		Uint32 team;
		SDL_Point goal;
		SDL_Point lastTriedGoal;
		bool selected;
		std::vector<SDL_Point> path;
		std::vector<void (*)(pawn* e)> updateList;
		ammo ammoList; //MAKE LIST
		gun activeGun;

		pawn(float inX, float inY, int inSize, float inAngle, std::string inName, SDL_Texture* inTex, std::vector<void (*)(pawn* e)> upList, Uint32 inTeam, float inSpeed, ammo inAmmo, gun inGun)
		{
			//pawn specific
			goal = { int(inX), int(inY) };
			lastTriedGoal = { int(inX), int(inY) };
			selected = false;
			speed = inSpeed;
			path;
			team = inTeam;
			updateList = upList;
			ammoList = inAmmo;
			activeGun = inGun;

			//basic
			name = inName;
			size = inSize;
			x = inX;
			y = inY;
			tex = inTex;
			angle = inAngle;
		}

		bool findPath(std::vector<std::vector<bool>> nodeMap, SDL_Point topLeft)
		{
			Pathfinder pathfinder;

			//clear any old path
			path.clear();

			//make a place to get back the paths
			std::vector<std::vector<Artist::pos>> paths;
			std::vector<std::vector<bool>> possibles;

			//get paths and possibles
			possibles = pathfinder.findPaths(nodeMap, { int(x) / 64 - topLeft.x, int(y) / 64 - topLeft.y }, &paths);//CHECK TO SEE IF THIS STOPS WHEN WE FIND THE GOAL ALMOST DEF DONT SO MAYBE MAKE IT??????

			//check if we can get to goal
			if (!possibles[int(goal.x) / 64 - topLeft.x][int(goal.y) / 64 - topLeft.y])
			{
				bool foundNewGoal = false;
				//if not find the closest open spot wiht a cap on distance MUST BE LESS THAN THE EXTRA ROOM WE GIVE TO LOOK AROUND?
				for (int out = 0; out < 9; out++)
				{
					//make a vector to hold all the spots we need to check
					std::vector<SDL_Point> checkList;

					//resize the list to hold a ring around the goal/the ring we just checked
					checkList.resize((out * 2 + 1) * 4 + 4);

					//convert the goal to a tile and the possible map
					SDL_Point goalTile = { int(goal.x) / 64 - topLeft.x, int(goal.y) / 64 - topLeft.y };

					//get the starting point on the list
					checkList[0] = { goalTile.x , goalTile.y };
					if (x < goal.x)
					{
						checkList[0].x -= (out + 1);
					}
					else if (x > goal.x)
					{
						checkList[0].x += (out + 1);
					}
					if (y < goal.y)
					{
						checkList[0].y -= (out + 1);
					}
					else if (y > goal.y)
					{
						checkList[0].y += (out + 1);
					}

					//direction we are going to fill the ring
					int dir = 0;
					//set dir to left or right dependng on what side of the ring we are on
					if (checkList[0].x < goalTile.x && checkList[0].y > goalTile.y)
					{
						dir = 2;
					}

					//get rest of list based on start
					for (int i = 1; i < checkList.size(); i++)
					{
						bool looking = true;
						//loop until we add next point
						while (looking)
						{
							//get the x/y number to add based on dir
							SDL_Point addToPos = { 0,0 };
							switch (dir)
							{
							case 0:
								addToPos = { 1,0 };
								break;
							case 1:
								addToPos = { 0,1 };
								break;
							case 2:
								addToPos = { -1,0 };
								break;
							case 3:
								addToPos = { 0,-1 };
								break;
							}

							//get the point in the direction from last point
							SDL_Point newPos = { checkList[i - 1].x + addToPos.x, checkList[i - 1].y + addToPos.y };

							//check to see if its a valid point
							if (abs(newPos.x - goalTile.x) <= out + 1 && abs(newPos.y - goalTile.y) <= out + 1)//add check to see if its a new point
							{
								checkList[i] = newPos;
								looking = false;
							}
							else
							{
								dir++;
								if (dir > 3)
									dir = 0;
							}
						}
					}
					//loop thru each spot on the list
					for (int i = 0; i < checkList.size(); i++)
					{
						//if the spot is open set it as goal
						if (possibles[checkList[i].x][checkList[i].y])
						{
							goal = { (checkList[i].x + topLeft.x) * 64 + 32, (checkList[i].y + topLeft.y) * 64 + 32 };
							i = checkList.size() + 1;
							out = 11;
							foundNewGoal = true;
						}
					}
				}
				//if no new goal found
				if (!foundNewGoal)
				{
					return false;
				}
			}

			//construct path from goal back to start
			bool pathing = true;
			//get starting point relative to the node map
			SDL_Point currentPoint = { int(goal.x) / 64 - topLeft.x, int(goal.y) / 64 - topLeft.y };
			SDL_Point mapPoint = { (currentPoint.x + topLeft.x) * 64 + 32, (currentPoint.y + topLeft.y) * 64 + 32 };
			//add it to path
			path.push_back(mapPoint);
			while (pathing)
			{
				//get next point from the path
				currentPoint = { paths[currentPoint.x][currentPoint.y].x, paths[currentPoint.x][currentPoint.y].y };
				//convert node map point to real map point
				mapPoint = { (currentPoint.x + topLeft.x) * 64 + 32, (currentPoint.y + topLeft.y) * 64 + 32 };

				//if we have reached the goal tile stop looking
				if (mapPoint.x / 64 == int(x) / 64 && mapPoint.y / 64 == int(y) / 64)
				{
					pathing = false;
				}
				else //else we add the point
				{
					path.push_back(mapPoint);
				}
			}
			return true;
		}

		void drawGoal(SDL_Point off)
		{
			Artist artist;

			//get size of tex
			SDL_Point size;
			SDL_QueryTexture(tex, NULL, NULL, &size.x, &size.y);
			//half it to draw a smaller version for the goal
			size.x /= 2;
			size.y /= 2;
			artist.drawImage(tex, goal.x - off.x - size.x / 2, goal.y - off.y - size.y / 2, size.x, size.y);

			//draw line to first point on path
			if (path.size() > 0)
			{
				artist.drawLineFromPoints(path[path.size() - 1].x - off.x, path[path.size() - 1].y - off.y, x - off.x, y - off.y);
			}
			//draw lines from each point on the path
			for (int i = 1; i < path.size(); i++)
			{
				artist.drawLineFromPoints(path[i].x - off.x, path[i].y - off.y, path[i-1].x - off.x, path[i-1].y - off.y);//PROLLY REPLACE WITH A FASTER FUNC LATER and maybe thats relative to the map use ents? eh idk
			}
			
		}

		void drawSelected(SDL_Point off)
		{
			Artist artist;
			SDL_Point size;
			SDL_QueryTexture(gSelected[0], NULL, NULL, &size.x, &size.y);
			artist.drawAnimation(gSelected, x - size.x / 2 - off.x, y - size.y / 2 - off.y, 150);
		}
	};

	

	
	

	static void moveToGoal(Entity::pawn* e);

	static void moveForward(Entity::projectile* e);

	static void makeBabys(Entity::projectile* e);

	static void destroyHit(Entity::projectile* e, int hitType, int hitID);

	static void spinHit(Entity::projectile* e, int hitType, int hitID);

	static void pushHit(Entity::projectile* e, int hitType, int hitID);

	static void stopAtWall(Entity::projectile* e, int hitType, int hitID);

	static void initEntity();
};
