#include "Map.h"
#include "Artist.h"
#include "MainMenu.h"
#include "Controller.h"
#include "Entity.h"
#include "Mixer.h"
#include <fstream>
#include <regex>
#include <map>




Map::map Map::activeMap; //make reachable by entity.cpp so we can manipulate the map stuff then we can delete the lists for adding ents

enum tileVars { COMMENT, EMPTY, NAME, COLLISION }; //Add new vars here


std::vector<std::vector<bool>> getMapChunkCollision(SDL_Point topLeft, SDL_Point size)
{
	//make a 2d vector to hold the data
	std::vector<std::vector<bool>> returnChunk;
	//resize it to what we ask
	returnChunk.resize(size.x);
	for (int x = 0; x < returnChunk.size(); x++)
	{
		returnChunk[x].resize(size.y);
	}

	//read map data and pass it to the vector
	for (int x = 0; x < returnChunk.size(); x++)
	{
		for (int y = 0; y < returnChunk[0].size(); y++)
		{
			//make sure we are adding points that are in the map only
			if (x + topLeft.x < 0 || x + topLeft.y < 0 || x + topLeft.x >= Map::activeMap.tileGrid.size() || y + topLeft.y >= Map::activeMap.tileGrid[0].size())
			{
				returnChunk[x][y] = false;
			}
			else
			{
				returnChunk[x][y] = !Map::activeMap.tileGrid[x + topLeft.x][y + topLeft.y].tileType->collision;
			}
		}
	}

	return returnChunk;
}

SDL_Point getMapMousePos()
{
	SDL_Point returnPoint;

	//get mouse pos relative to zoom and panning
	SDL_Point newGoal = { (Controller::mouseX - 1920 / 2) / Map::activeMap.camZoom + Map::activeMap.camPosX, (Controller::mouseY - 1080 / 2) / Map::activeMap.camZoom + Map::activeMap.camPosY };

	//convert rot to radians
	float rads = Map::activeMap.camRot * (M_PI / 180);

	//create rotation matrix based off camRot
	float R[4];
	R[0] = R[3] = cos(rads), R[1] = -(R[2] = -sin(rads));

	//use matrix to rotate 
	float p[2] = { newGoal.x, newGoal.y };
	float o[2] = { Map::activeMap.camPosX, Map::activeMap.camPosY };
	float t0 = p[0] - o[0], t1 = p[1] - o[1];
	p[0] = R[0] * t0 + R[1] * t1 + o[0], p[1] = R[2] * t0 + R[3] * t1 + o[1];

	//set goal to the rotated points
	returnPoint.x = p[0];
	returnPoint.y = p[1];

	return returnPoint;
}

void renderEntities()
{
	Artist artist;

	//go thru each projectile
	for (int i = 0; i < Map::activeMap.entityList.projectiles.size(); i++)
	{
		Map::activeMap.entityList.projectiles[i].draw(Map::activeMap.camOffset);
	}
	//go thru each basic ent
	for (int i = 0; i < Map::activeMap.entityList.entities.size(); i++)
	{
		Map::activeMap.entityList.entities[i].draw(Map::activeMap.camOffset);
	}
	//go thru each pawn
	for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
	{
		Map::activeMap.entityList.pawns[i].drawGoal(Map::activeMap.camOffset);

		Map::activeMap.entityList.pawns[i].draw(Map::activeMap.camOffset);

		if (Map::activeMap.entityList.pawns[i].selected)
		{
			Map::activeMap.entityList.pawns[i].drawSelected(Map::activeMap.camOffset);
		}
	}
	
}

void renderBackground()//SOMEDAY MAKE IT SO IT COPIES WHAT WE ALREADY HAVE OVER THE AMOUNT OF TILES WE SHIFT THEN ONLY ADD WHAT WE DONT HAVE     ALSO ADD DECALS BLOODSPLATS/BULLETHOLES//EXPLOSIONS PERMANANT STUFF
{
	Artist artist; 

	//update our offset
	Map::activeMap.camOffset = { int(Map::activeMap.camPosX - Map::activeMap.renderDist / 2) / 64 * 64, int(Map::activeMap.camPosY - Map::activeMap.renderDist / 2) / 64 * 64 };
	
	artist.setRenderTarget(Map::activeMap.backgroundTiles);
	for (int x = 0; x < Map::activeMap.renderDist / 64; x++)//will need changed to include a cap on how much we draw something like res/32 with extra room ALSO a starting point
	{
		for (int y = 0; y < Map::activeMap.renderDist / 64; y++)//will need changed to include a cap on how much we draw something like res/32 with extra room ALSO a starting point
		{
			SDL_Point finalGridPos = { x + Map::activeMap.camOffset.x / 64, y + Map::activeMap.camOffset.y / 64 };//TEMP

			if (finalGridPos.x >= 0 && finalGridPos.x < Map::activeMap.tileGrid.size() && finalGridPos.y >= 0 && finalGridPos.y < Map::activeMap.tileGrid[0].size())
			{
				artist.drawImage(Map::activeMap.tileGrid[finalGridPos.x][finalGridPos.y].tileType->tex, x * 64, y * 64);
			}
			else
			{
				artist.drawImage(Map::activeMap.masterTileList[0].tex, x * 64, y * 64);
			}
		}
	}

	artist.setRenderTarget(NULL);
}

std::pair<tileVars, std::smatch> parse_line(std::istream* dataFile) {
	/*
	read line from file and parse it
	IN:  pointer to iostream data file
	OUT: key(unique identifier int), match (desired string from line)
	*/

	// Key/Regex map
	std::map<tileVars, std::regex> rx_map;
	rx_map[COMMENT] = std::regex("^\\s*//.*"); //Matches Comments
	rx_map[EMPTY] = std::regex("^\\s*$");        //Matches empty strings (with white space)
	rx_map[NAME] = std::regex("^\\s*\\[Name\\]\\s*(.*)"); //Add new vars here
	rx_map[COLLISION] = std::regex("^\\s*\\[Collision\\]\\s*(.*)");
	//rx_map[CONNECT] = std::regex("^\\s*\\[Connect\\]\\s*(.*)");

	static std::string s; //This must be static or the smatch var will not work
	std::smatch match;    // Regex matches
	tileVars key;         // Match key

	std::getline(*dataFile, s); //Read in a line (string\n) from the file

	for (std::map<tileVars, std::regex>::iterator iter = rx_map.begin(); iter != rx_map.end(); ++iter) {
		if (std::regex_match(s, (*iter).second)) { //If there is a match
			std::regex_search(s, match, (*iter).second); //get matches
			key = (*iter).first; //get the key associated with the match
			break; //Stop searching and return key match pair
		}
	}
	return std::make_pair(key, match); //Return key match pair
}

Map::masterTile loadTileData(std::string tileName) {
	/*
	RimLike Tile Vars Loading
	IN: tile name name string
	OUT: tile struct with vars set based on data file
	*/

	Artist artist;

	// Tile defaults
	Map::masterTile tile = { NULL, tileName }; //Add new vars here
	std::pair<tileVars, std::smatch> key_match;
	std::string s;

	// Get the texture from png
	tile.tex = artist.loadTexture("Resource/tiles/" + tileName + ".png");
	if (tile.tex == NULL)
	{
		tile.tex = artist.loadTexture("Resource/tiles/error.png");
	}

	// Open tile data file
	std::ifstream tileDataFile;
	tileDataFile.open("Resource/tiles/tileData"); // Open file for read

	if (!tileDataFile.is_open()) return tile; // Return from funciton if file does not exist

	key_match = parse_line(&tileDataFile); //Parse first line

	while (tileDataFile.eof() != 1) { //Loop until end of file

		//if the current line is [Name] something and it matches the given name or default
		if (key_match.first == NAME && (key_match.second.str(1) == tileName || key_match.second.str(1) == "default")) {

			bool nameDefault = (key_match.second.str(1) == "default"); //Makes the following loop not break; on default case

			key_match = parse_line(&tileDataFile); //Parse next line

			// Keep loading parameters until next [Name] or end of file
			while (key_match.first != NAME) {
				//Set variables based on key
				switch (key_match.first) {
					//Add new vars here
				case COLLISION: tile.collision = std::stoi(key_match.second.str(1));
					break;
				//case CONNECT: tile.connect = std::stoi(key_match.second.str(1));
					//break;
				}

				if (tileDataFile.eof()) break; //Break out if the file ends

				key_match = parse_line(&tileDataFile); //Parse next line
			};
			if (!nameDefault) break; //Break out if Name is not default
		}
		else key_match = parse_line(&tileDataFile); //Parse next line
	};
	tileDataFile.close(); //Close the file $_$ 
	return tile;
}

Map::tile getTile(std::string name)
{
	Map::tile returnTile;

	//defaults
	returnTile.roofed = false;

	//look in master list to see if we already have it loaded
	for (int i = 0; i < Map::activeMap.masterTileList.size(); i++)
	{
		if (name == Map::activeMap.masterTileList[i].name)
		{
			returnTile.tileType = &Map::activeMap.masterTileList[i];
			return returnTile;
		}
	}

	//if we dont have it loaded, add it to the master list so we can return with it
	Map::activeMap.masterTileList.push_back(loadTileData(name));
	returnTile.tileType = &Map::activeMap.masterTileList[Map::activeMap.masterTileList.size()-1];
	return returnTile;
}

void Map::loadMap(std::string path)
{
	Artist artist;
	Entity entity;

	//set menu state to in map
	MainMenu::menuState = MainMenu::IN_MAP;

	//load map vars
	Map::activeMap.renderDist = 100 * 64;
	Map::activeMap.backgroundTiles = artist.loadTargetTexture(Map::activeMap.renderDist, Map::activeMap.renderDist);
	Map::activeMap.backgroundFinal = artist.loadTargetTexture(Map::activeMap.renderDist, Map::activeMap.renderDist);
	Map::activeMap.camRot = 0;
	Map::activeMap.camPosX = 1920/2;
	Map::activeMap.camPosY = 1080/2;
	Map::activeMap.camZoom = 1;
	Map::activeMap.camOffset = { int(Map::activeMap.camPosX - Map::activeMap.renderDist / 2), int(Map::activeMap.camPosY - Map::activeMap.renderDist / 2) };
	
	//set map size
	Map::activeMap.tileGrid.resize(100);
	for (int i = 0; i < Map::activeMap.tileGrid.size(); i++)
	{
		Map::activeMap.tileGrid[i].resize(100);
	}

	//load tiles
	getTile("outOfBounds");
	getTile("grass");
	getTile("dirt");
	getTile("wall");
	
	//set each tile
	srand(clock());
	for (int x = 0; x < Map::activeMap.tileGrid.size(); x++)
	{
		for (int y = 0; y < Map::activeMap.tileGrid[x].size(); y++)
		{
			int random = rand() % (Map::activeMap.masterTileList.size() - 1);
			Map::activeMap.tileGrid[x][y].tileType = &Map::activeMap.masterTileList[random + 1];
		}
	}

	//wall off the borders
	for (int x = 0; x < Map::activeMap.tileGrid.size(); x++)
	{
		Map::activeMap.tileGrid[x][0] = getTile("wall");
		Map::activeMap.tileGrid[x][Map::activeMap.tileGrid[0].size()-1] = getTile("wall");;
	}
	for (int y = 0; y < Map::activeMap.tileGrid[0].size(); y++)
	{
		Map::activeMap.tileGrid[0][y] = getTile("wall");;
		Map::activeMap.tileGrid[Map::activeMap.tileGrid.size() - 1][y] = getTile("wall");;
	}

	//create BG image from tiles
	renderBackground();
	
	//load entities
	//SDL_Texture* gEnt = artist.loadTexture("Resource/entities/ent.png");
	//int size;
	//SDL_QueryTexture(gEnt, NULL, NULL, &size, NULL);
	//for (int i = 0; i < 10; i++)
	//{
	//	SDL_Texture* colored = artist.loadTexture("Resource/entities/ent.png");
	//	SDL_SetTextureColorMod(colored, rand() % 256, rand() % 256, rand() % 256);
	//	Map::activeMap.entityList.entities.push_back(Entity::entity(rand() % 1920, rand() % 1080, size, 0, "Entity" + std::to_string(i), colored, {  }));
	//}

	//load pawns
	//SDL_Texture* gPawn = artist.loadTexture("Resource/entities/pawn.png");
	//int sizeP;
	//SDL_QueryTexture(gPawn, NULL, NULL, &sizeP, NULL);
	//for (int i = 0; i < 10; i++)
	//{
	//	SDL_Texture* colored = artist.loadTexture("Resource/entities/pawn.png");
	//	SDL_SetTextureColorMod(colored, rand() % 256, rand() % 256, rand() % 256);
	//	Map::activeMap.entityList.pawns.push_back(Entity::pawn(rand() % 1920 / 64 * 64 + 32, rand() % 1080 / 64 * 64 + 32, sizeP, 0, "Pawn" + std::to_string(i), colored, { Entity::moveToGoal }, 1, 1));
	//}
}

void Map::draw()
{
	if (MainMenu::menuState == MainMenu::IN_MAP)
	{
		Artist artist;

		//determine if we need to regenerate the background image
		SDL_Point testCamOffset = { (Map::activeMap.camPosX - Map::activeMap.renderDist / 2), (Map::activeMap.camPosY - Map::activeMap.renderDist / 2) };
		SDL_Point offsetDif = { abs(testCamOffset.x - Map::activeMap.camOffset.x), abs(testCamOffset.y - Map::activeMap.camOffset.y) };
		if (offsetDif.x > 64*10 || offsetDif.y > 64*10)
		{
			renderBackground();
		}
		
		//get the point that the cam is on relative to the image
		SDL_Point point = { int(Map::activeMap.camPosX * Map::activeMap.camZoom - Map::activeMap.camOffset.x * Map::activeMap.camZoom), int(Map::activeMap.camPosY * Map::activeMap.camZoom - Map::activeMap.camOffset.y * Map::activeMap.camZoom) };

		//copy the tile bg to the final bg
		artist.setRenderTarget(Map::activeMap.backgroundFinal);
		artist.drawImage(Map::activeMap.backgroundTiles, 0, 0);

		//render the entities ont the tile bg
		renderEntities();
		artist.setRenderTarget(NULL);

		//draw the final background to he screen with the given rotation and point
		artist.drawImage(Map::activeMap.backgroundFinal, 1920 / 2 - Map::activeMap.camPosX * Map::activeMap.camZoom + Map::activeMap.camOffset.x * Map::activeMap.camZoom, 1080 / 2 - Map::activeMap.camPosY * Map::activeMap.camZoom + Map::activeMap.camOffset.y * Map::activeMap.camZoom, Map::activeMap.renderDist * Map::activeMap.camZoom, Map::activeMap.renderDist * Map::activeMap.camZoom, Map::activeMap.camRot, &point);


		//TEMP for drawing crosshair
		//artist.changeRenderColor(255, 0, 0, 255);
		//artist.drawLineFromPoints(0, 1080 / 2, 1920, 1080 / 2);
		//artist.changeRenderColor(0, 255, 0, 255);
		//artist.drawLineFromPoints(1920 / 2, 0, 1920 / 2, 1080);
		//artist.changeRenderColor(255, 0, 255, 255);
		
		////TEMP for drawing some info
		//artist.drawLetters("X:" + std::to_string(Map::activeMap.camPosX) + " Y:" + std::to_string(Map::activeMap.camPosY), 0, 128, Artist::smallFont);
		//artist.drawLetters("Rot:" + std::to_string(Map::activeMap.camRot), 0, 128*2, Artist::smallFont);
		//artist.drawLetters("Zoom:" + std::to_string(Map::activeMap.camZoom), 0, 128*3, Artist::smallFont);
		//artist.drawLetters("Pro Count:" + std::to_string(Map::activeMap.entityList.projectiles.size()), 0, 128*4, Artist::smallFont);
	}
}

void Map::update()
{
	if (Controller::FPSLock)
	{
		//run update funcs for ents
		//all basic ents
		for (int i = 0; i < Map::activeMap.entityList.entities.size(); i++)
		{
			//go thur each update func the ent has
			for (int j = 0; j < Map::activeMap.entityList.entities[i].updateList.size(); j++)
			{
				Map::activeMap.entityList.entities[i].updateList[j](&Map::activeMap.entityList.entities[i]);
			}
		}
		//all pawns
		for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
		{
			for (int j = 0; j < Map::activeMap.entityList.pawns[i].updateList.size(); j++)
			{
				Map::activeMap.entityList.pawns[i].updateList[j](&Map::activeMap.entityList.pawns[i]);
			}
		}
		//all projectiles
		for (int i = 0; i < Map::activeMap.entityList.projectiles.size(); i++)
		{
			for (int j = 0; j < Map::activeMap.entityList.projectiles[i].updateList.size(); j++)
			{
				if (Map::activeMap.entityList.projectiles[i].lifespan > 0)
				{
					Map::activeMap.entityList.projectiles[i].updateList[j](&Map::activeMap.entityList.projectiles[i]);
				}
			}

			//keep updating lifespan so the projectile expires on time
			Map::activeMap.entityList.projectiles[i].lifespan--;
		}

		//clean up backwards so we dont skip any from erasing them
		for (int i = Map::activeMap.entityList.projectiles.size()-1; i >= 0; i--)
		{
			//check to see if any have expired yet
			if (Map::activeMap.entityList.projectiles[i].lifespan <= 0)
			{
				//run the expire funcs
				for (int j = 0; j < Map::activeMap.entityList.projectiles[i].expireList.size(); j++)
				{
					Map::activeMap.entityList.projectiles[i].expireList[j](&Map::activeMap.entityList.projectiles[i]);
				}

				//remove projectile
				Map::activeMap.entityList.projectiles.erase(Map::activeMap.entityList.projectiles.begin() + i);
			}
		}

		
	}
}

void Map::controller()//MOVE SOME FUNCTIUONALITY TO THIER OWN FUNCTIONS/IN THIER STRUCT
{
	if (MainMenu::menuState == MainMenu::IN_MAP)
	{
		if (Controller::FPSLock)
		{
			//ROT CAM WITH KEYS
			if (Controller::keyboardStates[SDL_SCANCODE_E])
			{
				Map::activeMap.camRot -= 1;
				if (Map::activeMap.camRot < 0)
				{
					Map::activeMap.camRot = 359;
				}
			}
			if (Controller::keyboardStates[SDL_SCANCODE_Q])
			{
				Map::activeMap.camRot += 1;
				if (Map::activeMap.camRot > 359)
				{
					Map::activeMap.camRot = 0;
				}
			}

			//MOVE CAM WITH KEYS
			//get directions we are moving
			bool directions[4] = { 0, 0, 0, 0 };
			if (Controller::keyboardStates[SDL_SCANCODE_UP])
			{
				directions[0] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_DOWN])
			{
				directions[1] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_LEFT])
			{
				directions[2] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_RIGHT])
			{
				directions[3] = true;
			}

			//get the velocity split
			int dirCount = 0;
			for (int i = 0; i < 4; i++)
			{
				if (directions[i])
				{
					dirCount++;
				}
			}
			float splitVel = 0;
			if (dirCount > 0)
				splitVel = 5 / dirCount;

			//apply vel to vector
			SDL_Point v = { 0, 0 };
			if (directions[0])
			{
				v.y -= splitVel;
			}
			if (directions[1])
			{
				v.y += splitVel;
			}
			if (directions[2])
			{
				v.x -= splitVel;
			}
			if (directions[3])
			{
				v.x += splitVel;
			}

			//convert to radians
			double theta = Map::activeMap.camRot * (M_PI / 180);

			//transform vel to be relative to the cam rot
			float addX = v.x * cos(theta) + v.y * sin(theta);
			float addY = v.x * sin(theta) - v.y * cos(theta);

			//add it to the cam pos
			Map::activeMap.camPosX += addX;
			Map::activeMap.camPosY -= addY;
		
		}
		//MOVE CAM WITH MOUSE DRAG
		if (Controller::mouseStates[1] == 2)
		{
			//calculate dif in mouse pos
			SDL_Point mouseDif = { Controller::lastMousePos.x - Controller::mouseX, Controller::lastMousePos.y - Controller::mouseY };

			if (abs(mouseDif.x) > 0 || abs(mouseDif.y) > 0)
			{
				//apply dif to vector
				SDL_Point v = { mouseDif.x, mouseDif.y };

				//convert to radians
				double theta = Map::activeMap.camRot * (M_PI / 180);

				//transform vel to be relative to the cam rot
				float addX = v.x * cos(theta) + v.y * sin(theta);
				float addY = v.x * sin(theta) - v.y * cos(theta);

				Map::activeMap.camPosX += addX;
				Map::activeMap.camPosY -= addY;
			}
		}

		//ZOOM IN OR OUT WITH MOUSE WHEEL
		Map::activeMap.camZoom += Controller::mouseWheelMovment.y * .05;
		if (Map::activeMap.camZoom > 2)
		{
			//Map::activeMap.camZoom = 2;
		}
		else if(Map::activeMap.camZoom < .5)
		{
			//Map::activeMap.camZoom = .5;
		}

		//RESET ZOOM
		if (Controller::keyboardStates[SDL_SCANCODE_Z] == 1)
		{
			Map::activeMap.camZoom = 1;
		}

		//SET PAWN GOAL TO MOUSE POS
		if (Controller::mouseStates[2] == 1 || Controller::mouseStates[2] == 2)
		{
			//get mouse map pos
			SDL_Point mousePoint = getMapMousePos();

			//convert to the center of the tile they clicked
			mousePoint.x = mousePoint.x / 64 * 64 + 32;
			mousePoint.y = mousePoint.y / 64 * 64 + 32;

			//set goal
			for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
			{
				//check if pawn is selected and that we arnt trying the goal we last tried
				if (Map::activeMap.entityList.pawns[i].selected && !(mousePoint.x == Map::activeMap.entityList.pawns[i].lastTriedGoal.x && mousePoint.y == Map::activeMap.entityList.pawns[i].lastTriedGoal.y))
				{
				
				
					Map::activeMap.entityList.pawns[i].lastTriedGoal = mousePoint;
					//TEMP AS F
					if (Controller::mouseStates[2] == 1)
					{
						Mixer mixer;
						mixer.playSound(Mixer::sMove[rand() % Mixer::sMove.size()]);
					}

					//check if the goal will be new
					if (mousePoint.x != Map::activeMap.entityList.pawns[i].goal.x || mousePoint.y != Map::activeMap.entityList.pawns[i].goal.y)
					{
						Map::activeMap.entityList.pawns[i].goal.x = mousePoint.x;
						Map::activeMap.entityList.pawns[i].goal.y = mousePoint.y;

						//figure out which point is up left most between the goal and entity
						SDL_Point topLeft = Map::activeMap.entityList.pawns[i].goal; //default to goal
						if (Map::activeMap.entityList.pawns[i].x < topLeft.x)
						{
							topLeft.x = Map::activeMap.entityList.pawns[i].x;
						}
						if (Map::activeMap.entityList.pawns[i].y < topLeft.y)
						{
							topLeft.y = Map::activeMap.entityList.pawns[i].y;
						}
						//convert to tile pos
						topLeft.x /= 64;
						topLeft.y /= 64;
						//move it up left to acount for extra room givin
						topLeft.x -= 10;
						topLeft.y -= 10;

						//get distance of ent and goal in tiles
						SDL_Point dist = { abs(Map::activeMap.entityList.pawns[i].x - Map::activeMap.entityList.pawns[i].goal.x),  abs(Map::activeMap.entityList.pawns[i].y - Map::activeMap.entityList.pawns[i].goal.y) };
						dist.x /= 64;
						dist.y /= 64;
						//give room around to look for paths
						dist.x += 21;
						dist.y += 21;


						if (!Map::activeMap.entityList.pawns[i].findPath(getMapChunkCollision(topLeft, dist), topLeft))
						{
							//if we dont find a way to goal set a path point to the tile we are in
							Map::activeMap.entityList.pawns[i].path.push_back({ int(Map::activeMap.entityList.pawns[i].x) / 64 * 64 + 32, int(Map::activeMap.entityList.pawns[i].y) / 64 * 64 + 32 });
							Map::activeMap.entityList.pawns[i].goal = { int(Map::activeMap.entityList.pawns[i].x) / 64 * 64 + 32, int(Map::activeMap.entityList.pawns[i].y) / 64 * 64 + 32 };

						}
					}
				
				}
			}
		}

		//SELECT A PAWN WITH MOUSE
		if (Controller::mouseStates[0] == 1)
		{
			//if shift not held clear all selections
			if (!Controller::keyboardStates[SDL_SCANCODE_LSHIFT])
			{
				for (int j = 0; j < Map::activeMap.entityList.pawns.size(); j++)
				{
					Map::activeMap.entityList.pawns[j].selected = false;
				}
			}
			//detect if we clicked on any doots WILL GRAB MULTIPLE IF WE DONT RETURN
			for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
			{
				if (Map::activeMap.entityList.pawns[i].radiusCollisionDetect(getMapMousePos(), 1) && !Map::activeMap.entityList.pawns[i].selected)
				{
					Map::activeMap.entityList.pawns[i].selected = true;
					break;
				}
			}
		}

		//SELECT ALL PAWNS
		if (Controller::keyboardStates[SDL_SCANCODE_LSHIFT] && Controller::keyboardStates[SDL_SCANCODE_A])
		{
			for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
			{
				Map::activeMap.entityList.pawns[i].selected = true;
			}
		}
	
		//ATTACK COMMAND
		if (Controller::keyboardStates[SDL_SCANCODE_A] == 1)//MAYBE MAKE IT SO THE BULLET COMES OUT HALF THE ENTS SIZE AWAY SO IT CANT HIT ITSELF THEN WE DONT NEED SHITTY OWNER BULSHIT
		{
			for (int i = 0; i < Map::activeMap.entityList.pawns.size(); i++)
			{
				if (Map::activeMap.entityList.pawns[i].selected)
				{
					Artist artist; //UGLY NEEDS A BETTER OPTION LATER   MAYBE JUST HAVE A MASTER TEX LIST OR HAVE THE AMMO TYPE HOLD THE TEX
					//get angle from pawn to mouse
					SDL_Point mousePos = getMapMousePos();

					float angle = atan2(Map::activeMap.entityList.pawns[i].y - mousePos.y, Map::activeMap.entityList.pawns[i].x - mousePos.x);
					angle *= 180;
					angle /= M_PI;
					angle -= 90;

					//fire the bullet that is the currently loaded by loading its projectile to the projectile list after updating possition and angle
					Map::activeMap.entityList.pawns[i].ammoList.proType.x = Map::activeMap.entityList.pawns[i].x;
					Map::activeMap.entityList.pawns[i].ammoList.proType.y = Map::activeMap.entityList.pawns[i].y;
					Map::activeMap.entityList.pawns[i].ammoList.proType.angle = angle;
					Map::activeMap.entityList.pawns[i].ammoList.proType.ownerType = 1;
					Map::activeMap.entityList.pawns[i].ammoList.proType.ownerID = i;
					Map::activeMap.entityList.projectiles.push_back(Map::activeMap.entityList.pawns[i].ammoList.proType);
				}
			}
		}

		//SPAWN ENT
		if (Controller::keyboardStates[SDL_SCANCODE_1] == 1)
		{
			SDL_Point spawnPos = getMapMousePos();
			Artist artist;
			SDL_Texture* gEnt = artist.loadTexture("Resource/entities/ent.png");
			SDL_SetTextureColorMod(gEnt, rand() % 255, rand() % 255, rand() % 255);
			activeMap.entityList.entities.push_back(Entity::entity(spawnPos.x, spawnPos.y, 33, 0, "Spawned Ent", gEnt, {}));
		}

		//SPAWN PAWN
		if (Controller::keyboardStates[SDL_SCANCODE_2] == 1)
		{
			//get mouse pos in game
			SDL_Point spawnPos = getMapMousePos();
			//load pawn tex
			Artist artist;
			SDL_Texture* gPawn = artist.loadTexture("Resource/entities/pawn.png");
			SDL_SetTextureColorMod(gPawn, rand() % 255, rand() % 255, rand() % 255);
			//create a projectile
			Entity::projectile passPro = Entity::projectile(0, 0, 1, 0, "Bullet", artist.loadTexture("Resource/entities/bullet.png"), 64, 10, -1, -1, { Entity::moveForward }, { Entity::stopAtWall, Entity::spinHit }, { Entity::makeBabys });
			activeMap.entityList.pawns.push_back(Entity::pawn(spawnPos.x, spawnPos.y, 33, 0, "Spawned Pawn", gPawn, { Entity::moveToGoal }, 0, 1, Entity::ammo(passPro), Entity::gun()));
		}

		if (Controller::keyboardStates[SDL_SCANCODE_3] == 1)
		{
			//get mouse pos in game
			SDL_Point spawnPos = getMapMousePos();
			//load pawn tex
			Artist artist;
			SDL_Texture* gPawn = artist.loadTexture("Resource/entities/pawnEnemy.png");
			SDL_SetTextureColorMod(gPawn, rand() % 255, rand() % 255, rand() % 255);
			//create a projectile
			Entity::projectile passPro = Entity::projectile(0, 0, 1, 0, "Bullet", artist.loadTexture("Resource/entities/bullet.png"), 10, 64, -1, -1, { Entity::moveForward }, { Entity::stopAtWall, Entity::pushHit }, { Entity::makeBabys });
			activeMap.entityList.pawns.push_back(Entity::pawn(spawnPos.x, spawnPos.y, 33, 0, "Spawned Pawn", gPawn, { Entity::moveToGoal }, 0, 1, Entity::ammo(passPro), Entity::gun()));
		}

		//MUTE SOUND TEMP
		if (Controller::keyboardStates[SDL_SCANCODE_M] == 1)
		{
			Mixer mixer;

			if (mixer.changeVolume(0) == 0)
			{
				mixer.changeVolume(32);
			}
		}
	}
}