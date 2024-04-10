#pragma once
#ifndef SDL
#include <SDL.h>
#endif // !SDL
#ifndef vector
#include <vector>
#endif
#ifndef string
#include <string>
#endif
#ifndef Entity
#include "Entity.h"
#endif

class Map {
public:

	struct masterTile {
		SDL_Texture* tex;
		std::string name;
		bool collision;
		//whatever else
	};

	struct tile {
		masterTile* tileType;
		bool roofed;
		//other info?
	};

	struct entityList {
		std::vector<Entity::entity> entities;
		std::vector<Entity::pawn> pawns;
		std::vector<Entity::projectile> projectiles;
	};

	struct map {
		std::vector<std::vector<tile>> tileGrid;
		std::vector<masterTile> masterTileList;
		SDL_Texture* backgroundTiles;
		SDL_Texture* backgroundFinal;
		int renderDist;
		double camRot;
		float camPosX;
		float camPosY;
		float camZoom;
		SDL_Point camOffset;
		entityList entityList;
	};
	

	static Map::map activeMap;

	void loadMap(std::string path);

	void draw();

	void update();

	void controller();

};