#pragma once
#include <SFML/Graphics.hpp>

class zone
{
public:
	sf::Vector2f coordinates;
	enum painterList
	{
		NONE = 0,
		CIRCLE = 1,
		CROSS = 2
	};
	int painter = painterList::NONE;
	zone(sf::Vector2f coordinates) : coordinates(coordinates) {}
	void Draw(sf::RenderWindow *window);

private:
	void DrawCircle(sf::RenderWindow *window);
	void DrawCross(sf::RenderWindow *window);
};