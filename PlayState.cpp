#include "PlayState.h"

#include "Root.h"

PlayState::PlayState() {

}

PlayState::~PlayState() {

}

void PlayState::init() {
	testText.setTexture(loadTexture("Resource/Image/Font.png"));
	testText.setPosition(2, 2);
	testText.setText("");
	testText.setColor(sf::Color::White);

	hud.setState(this);
	hud.init();

	cursor.setTexture(loadTexture("Resource/Image/Cursor.png"));
	cursor.setOrigin(6, 6);

	transitionOverlay.setSize(sf::Vector2f(240, 135));
	transitionOverlay.setFillColor(sf::Color::Transparent);
	nightOverlay.setSize(sf::Vector2f(240, 135));
	nightOverlay.setFillColor(sf::Color::Transparent);

	//dayMusic.openFromFile("Resource/Music/Day.ogg");
	//nightMusic.openFromFile("Resource/Music/Night.ogg");

	buildWorld(50, 50);

	player.setState(this);
	player.setPosition(worldSize.x / 2 * 10, worldSize.y / 2 * 10 + 10);
	player.init();

	cameraPosition = player.getPosition() - sf::Vector2f(120, 70);

	calculateMaxResources();
	water = maxWater;
	nutrients = maxNutrients / 2;

	dayMusic.play();
}

void PlayState::gotEvent(sf::Event event) {
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Left) {
			if (!getGridObject(getCursorGridLocation().x, getCursorGridLocation().y)) {
				if (spendNutrients(1, sf::Vector2f(getCursorGridLocation() * 10))) {
					setGridObject(getCursorGridLocation().x, getCursorGridLocation().y, std::make_shared<Root>());
				}
			}
		}
	}
}

void PlayState::update(sf::Time elapsed) {
	time += elapsed.asSeconds();
	// Debug
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
		time += elapsed.asSeconds() * 10;
	}
	if (time >= secondsPerDay) {
		time = 0;
		for (std::shared_ptr<GridObject> &object : objectGrid) {
			if (object) {
				object->onHour(hour);
			}
		}
		hour += 1;
		if (hour > 10) {
			hour = 0;
			for (std::shared_ptr<GridObject> &object : objectGrid) {
				if (object) {
					object->onDay();
				}
			}
			day += 1;
		}
		if (hour == 2) {
			dayMusic.play();
			nightMusic.stop();
		}
		else if (hour == 9) {
			dayMusic.stop();
			nightMusic.play();
		}
	}
	updateOverlays();

	cameraPosition += (player.getPosition() - sf::Vector2f(120, 70) - cameraPosition) * elapsed.asSeconds() * 4.0f;

	updateParticles(elapsed);

	for (std::shared_ptr<GridTile> &object : tileGrid) {
		if (object) {
			object->update(elapsed);
		}
	}
	for (std::shared_ptr<GridObject> &object : objectGrid) {
		if (object) {
			object->update(elapsed);
		}
	}

	calculateMaxResources();

	player.update(elapsed);
	hud.update(elapsed);

	cursor.setPosition(getCursorGridLocation().x * 10 - cameraPosition.x, getCursorGridLocation().y * 10 - cameraPosition.y);
}

std::string PlayState::getTimeOfDay(int hour) {
	if (hour == 0 || hour == 1 || hour == 9 || hour == 10) {
		return "Night";
	}
	else if (hour == 2 || hour == 8) {
		return "Transition";
	}
	else {
		return "Day";
	}
}

sf::Color PlayState::getResourceColor(std::string resource) {
	if (resource == "Light") {
		return sf::Color(255, 240, 132);
	}
	else if (resource == "Water") {
		return sf::Color(0, 148, 255);
	}
	else if (resource == "Nutrients") {
		return sf::Color(204, 106, 106);
	}
	else {
		return sf::Color::White;
	}
}

void PlayState::calculateMaxResources() {
	maxLight = 0;
	maxWater = 0;
	maxNutrients = 0;
	for (std::shared_ptr<GridObject> &object : objectGrid) {
		if (object && object->playerOwned) {
			maxLight += object->maxLight;
			maxWater += object->maxWater;
			maxNutrients += object->maxNutrients;
		}
	}
	if (light > maxLight) {
		light = maxLight;
	}
	if (water > maxWater) {
		water = maxWater;
	}
	if (nutrients > maxNutrients) {
		nutrients = maxNutrients;
	}
}

void PlayState::gainLight(float gained, sf::Vector2f position) {
	light += gained;
	if (light > maxLight) {
		light = maxLight;
	}
	for (int i = 0; i < std::floor(gained); i++) {
		createParticle(position + sf::Vector2f(-4, -4 * i), sf::Vector2f(0, -20), getResourceColor("Light"), Particle::plus);
	}
}

bool PlayState::spendLight(float spent, sf::Vector2f position) {
	if (light >= spent) {
		light -= spent;
		for (int i = 0; i < std::floor(spent); i++) {
			createParticle(position + sf::Vector2f(-4, -4 * i), sf::Vector2f(0, -20), getResourceColor("Light"), Particle::minus);
		}
		return true;
	}
	else {
		return false;
	}
}

void PlayState::gainWater(float gained, sf::Vector2f position) {
	water += gained;
	if (water > maxWater) {
		water = maxWater;
	}
	for (int i = 0; i < std::floor(gained); i++) {
		createParticle(position + sf::Vector2f(0, -4 * i), sf::Vector2f(0, -20), getResourceColor("Water"), Particle::plus);
	}
}

bool PlayState::spendWater(float spent, sf::Vector2f position) {
	if (water >= spent) {
		water -= spent;
		for (int i = 0; i < std::floor(spent); i++) {
			createParticle(position + sf::Vector2f(0, -4 * i), sf::Vector2f(0, -20), getResourceColor("Water"), Particle::minus);
		}
		return true;
	}
	else {
		return false;
	}
}

void PlayState::gainNutrients(float gained, sf::Vector2f position) {
	nutrients += gained;
	if (nutrients > maxNutrients) {
		nutrients = maxNutrients;
	}
	for (int i = 0; i < std::floor(gained); i++) {
		createParticle(position + sf::Vector2f(4, -4 * i), sf::Vector2f(0, -20), getResourceColor("Nutrients"), Particle::plus);
	}
}

bool PlayState::spendNutrients(float spent, sf::Vector2f position) {
	if (nutrients >= spent) {
		nutrients -= spent;
		for (int i = 0; i < std::floor(spent); i++) {
			createParticle(position + sf::Vector2f(4, -4 * i), sf::Vector2f(0, -20), getResourceColor("Nutrients"), Particle::minus);
		}
		return true;
	}
	else {
		return false;
	}
}

void PlayState::createParticle(sf::Vector2f position, sf::Vector2f velocity, sf::Color color, Particle::ParticleType type, bool onHud) {
	Particle particle;
	particle.type = type;
	particle.position = position;
	particle.velocity = velocity;
	particle.color = color;
	particle.onHud = onHud;
	particles.push_back(particle);
}

void PlayState::updateParticles(sf::Time elapsed) {
	auto particle = particles.begin();
	while (particle != particles.end()) {
		particle->lifetime -= elapsed.asSeconds();
		particle->velocity *= std::powf(0.2, elapsed.asSeconds());
		particle->position += particle->velocity * elapsed.asSeconds();
		if (particle->lifetime <= 0) {
			particle = particles.erase(particle);
		}
		else {
			particle++;
		}
	}
}

void PlayState::render(sf::RenderWindow &window) {
	for (std::shared_ptr<GridTile> &object : tileGrid) {
		if (object) {
			window.draw(*object);
		}
	}
	for (std::shared_ptr<GridObject> &object : objectGrid) {
		if (object) {
			window.draw(*object);
		}
	}

	sf::Sprite particleSprite(loadTexture("Resource/Image/Particles.png"));
	particleSprite.setOrigin(1, 1);
	for (Particle &particle : particles) {
		if (!particle.onHud) {
			particleSprite.setTextureRect(sf::IntRect((int)particle.type * 3, 0, 3, 3));
			particleSprite.setPosition(particle.position - cameraPosition);
			particleSprite.setColor(particle.color);
			window.draw(particleSprite);
		}
	}

	window.draw(nightOverlay);
	window.draw(transitionOverlay);

	window.draw(cursor);

	window.draw(player);

	window.draw(hud);
	window.draw(testText);
	for (Particle &particle : particles) {
		if (particle.onHud) {
			particleSprite.setTextureRect(sf::IntRect((int)particle.type * 3, 0, 3, 3));
			particleSprite.setPosition(particle.position);
			particleSprite.setColor(particle.color);
			window.draw(particleSprite);
		}
	}
}

void PlayState::buildWorld(int worldWidth, int worldHeight) {
	worldSize.x = worldWidth;
	worldSize.y = worldHeight;
	tileGrid.resize(worldSize.x * worldSize.y);
	objectGrid.resize(worldSize.x * worldSize.y);
	for (int y = 0; y < worldSize.y; y++) {
		for (int x = 0; x < worldSize.x; x++) {
			std::shared_ptr<GridTile> newTile = std::make_shared<GridTile>();
			newTile->setState(this);
			newTile->setPosition(x * 10, y * 10);
			newTile->init();
			tileGrid[y * worldSize.x + x] = newTile;
			if (std::rand() % 20 == 0) {
				setGridTile(x, y, "Water");
			}
			else if (std::rand() % 20 == 0) {
				setGridTile(x, y, "Nutrients");
			}

			if (x == worldSize.x / 2 && y == worldSize.y / 2) {
				setGridObject(x, y, std::make_shared<Tree>());
			}
		}
	}
}

std::shared_ptr<GridTile> PlayState::getGridTile(int x, int y) {
	if (x < 0 || x > worldSize.x - 1) {
		return std::shared_ptr<GridTile>();
	}
	else if (y < 0 || y > worldSize.y - 1) {
		return std::shared_ptr<GridTile>();
	}
	else {
		return tileGrid[y * worldSize.x + x];
	}
}

void PlayState::setGridTile(int x, int y, std::string newType) {
	if (x >= 0 && x <= worldSize.x - 1 && y >= 0 || y <= worldSize.y - 1) {
		tileGrid[y * worldSize.x + x]->setType(newType);
	}
}

std::shared_ptr<GridObject> PlayState::getGridObject(int x, int y) {
	if (x < 0 || x > worldSize.x - 1) {
		return std::shared_ptr<GridObject>();
	}
	else if (y < 0 || y > worldSize.y - 1) {
		return std::shared_ptr<GridObject>();
	}
	else {
		return objectGrid[y * worldSize.x + x];
	}
}

void PlayState::setGridObject(int x, int y, std::shared_ptr<GridObject> newObject) {
	if (x >= 0 && x <= worldSize.x - 1 && y >= 0 || y <= worldSize.y - 1) {
		newObject->setState(this);
		newObject->setPosition(x * 10, y * 10);
		newObject->init();
		objectGrid[y * worldSize.x + x] = newObject;
	}
}

sf::Vector2i PlayState::worldLocationToGrid(sf::Vector2f location) {
	return sf::Vector2i((location.x + 5) / 10, (location.y + 5) / 10);
}

sf::Vector2f PlayState::screenLocationToWorld(sf::Vector2f location) {
	return sf::Vector2f(location + cameraPosition);
}

sf::Vector2f PlayState::getCursorLocation() {
	return (sf::Vector2f(sf::Mouse::getPosition(*game->getWindow())) - game->screenOffset) / float(game->scale);
}

sf::Vector2i PlayState::getCursorGridLocation() {
	return worldLocationToGrid(screenLocationToWorld(getCursorLocation()));
}

void PlayState::updateOverlays() {
	if (hour == 2) {
		nightOverlay.setFillColor(sf::Color(0, 0, 0, 140 * (1 - time / secondsPerDay)));
		transitionOverlay.setFillColor(sf::Color(255, 119, 56, 60 * (time / secondsPerDay)));
	}
	else if (hour == 3) {
		transitionOverlay.setFillColor(sf::Color(255, 119, 56, 60 * (1 - time / secondsPerDay)));
	}
	else if (hour == 7) {
		transitionOverlay.setFillColor(sf::Color(255, 119, 56, 60 * (time / secondsPerDay)));
	}
	else if (hour == 8) {
		transitionOverlay.setFillColor(sf::Color(255, 119, 56, 60 * (1 - time / secondsPerDay)));
		nightOverlay.setFillColor(sf::Color(0, 0, 0, 140 * (time / secondsPerDay)));
	}
}
