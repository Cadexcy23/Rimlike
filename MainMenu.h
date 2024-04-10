#pragma once

class MainMenu {
public:

	static enum menuStates
	{
		MAIN,
		IN_MAP
	};

	static int menuState;


	bool initMainMenu();

	void draw();

};
