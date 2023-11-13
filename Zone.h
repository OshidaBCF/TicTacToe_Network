#pragma once
#include <SFML/Graphics.hpp>

class zone
{
public:
	sf::Vector2f coordinates;
	enum painterList
	{
		CIRCLE = -1,
		NONE = 0,
		CROSS = 1
	};
	int painter = painterList::NONE;

	void Draw(sf::RenderWindow *window);

private:
	void DrawCircle(sf::RenderWindow *window);
	void DrawCross(sf::RenderWindow *window);
};