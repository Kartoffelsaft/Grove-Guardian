#include "GridTile.h"

#include "PlayState.h"

void GridTile::init() {
	sprite.setTexture(state->loadTexture("Resource/Image/Tiles.png"));
	setType(type);
	sprite.setOrigin(5, 5);
}

void GridTile::update(sf::Time elapsed) {
	if (getType() != "Grass" && quantity <= 0) {
		setType("Grass");
	}
	sprite.setPosition(getPosition() - state->cameraPosition);
}

void GridTile::setType(std::string type) {
	this->type = type;
	if (type == "Grass") {
		sprite.setTextureRect(sf::IntRect(std::rand() % 4 * 10, 0, 10, 10));
	}
	else if (type == "Water") {
		sprite.setTextureRect(sf::IntRect(40 + std::rand() % 4 * 10, 0, 10, 10));
		quantity = 10 + std::rand() % 6;
	}
	else if (type == "Nutrients") {
		sprite.setTextureRect(sf::IntRect(80 + std::rand() % 4 * 10, 0, 10, 10));
		quantity = 10 + std::rand() % 6;
	}
}

std::string GridTile::getType() {
	return type;
}

void GridTile::draw(sf::RenderTarget & target, sf::RenderStates states) const {
	target.draw(sprite);
}
