#pragma once

#include "Object.h"
#include "GridObject.h"
#include <SFML/Graphics.hpp>

class Spirit : public Object {
public:
	void init() override;
	void update(sf::Time elapsed) override;

private:
	// Inherited via Drawable
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

	void updateVelocity(sf::Time elapsed);
	void updateAnimation(sf::Time elapsed);

	// Internal data
	std::shared_ptr<GridObject> target;
	sf::Vector2f velocity;
	int frame;
	float frameTime;

	// Stats
	float topSpeed = 15;

	// Resources
	sf::Sprite sprite;
};

