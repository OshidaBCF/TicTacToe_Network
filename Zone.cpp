#include "Zone.h"

void zone::Draw(sf::RenderWindow *window)
{
	switch (painter)
	{
	case NONE:
		break;
	case CIRCLE:
		DrawCircle(window);
		break;
	case CROSS:
		DrawCross(window);
		break;
	}
}

void zone::DrawCircle(sf::RenderWindow *window)
{
	sf::CircleShape shape(100);
	shape.setFillColor(sf::Color(0, 0, 0));
	shape.setOutlineThickness(-10);
	shape.setOutlineColor(sf::Color(52, 152, 219));
	shape.setPosition(coordinates + sf::Vector2f(50, 50));
	window->draw(shape);
}

void zone::DrawCross(sf::RenderWindow *window)
{
	sf::RectangleShape line(sf::Vector2f(300, 10));
	line.setFillColor(sf::Color(241, 196, 15));
	line.setPosition(coordinates + sf::Vector2f(125.0 / 2.84 + 3.5, 125.0 / 2.84 - 3.5));
	line.rotate(45);
	window->draw(line);
	line.setPosition(coordinates + sf::Vector2f(300 - 125.0 / 2.84 + 3.5, 125.0 / 2.84 + 3.5));
	line.rotate(90);
	window->draw(line);
}
