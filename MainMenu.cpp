#include "MainMenu.h"
#include "Button.h"
#include "Map.h"


std::vector<Button::button> buttonList;




int MainMenu::menuState = MAIN;

void dummyFunc()
{

}

void startMap()
{
	Map map;

	map.loadMap("temp");
}

bool MainMenu::initMainMenu()
{
	Button button;
	Artist artist;

	buttonList.clear();


	std::vector<SDL_Texture*> newButtonTexSet;
	newButtonTexSet.push_back(artist.loadTexture("Resource/mainMenu/title.png"));
	newButtonTexSet.push_back(artist.loadTexture("Resource/mainMenu/title.png"));
	buttonList.push_back(button.makeButton(newButtonTexSet, 1920 / 2 - 128, 0, 16.6, dummyFunc));

	newButtonTexSet.clear();
	newButtonTexSet.push_back(artist.loadTexture("Resource/mainMenu/start.png"));
	newButtonTexSet.push_back(artist.loadTexture("Resource/mainMenu/startHover.png"));
	buttonList.push_back(button.makeButton(newButtonTexSet, 1920 / 2 - 64, 700, 16.6, startMap));


	return true;
}

void MainMenu::draw()
{
	if (menuState == MAIN)
	{
		Button button;

		button.updateButtonVector(buttonList);
	}
}