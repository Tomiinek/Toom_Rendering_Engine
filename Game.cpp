#include "Game.h"

//#define DEBUG

#include "Game.h"

//#define DEBUG

void Game::run() {

	sf::ContextSettings settings;
	sf::RenderWindow window(sf::VideoMode(1366, 768), "Toom 1", sf::Style::Default /*Fullscreen*/, settings);
	window.setMouseCursorVisible(false);
	window.setFramerateLimit(65);
	sf::Clock clock;

	Sounds& s = Sounds::global();
	s.load(Sounds::bullet_wall, "Sounds/BulletHitWall.wav", false, false);
	s.load(Sounds::death, "Sounds/Death.wav", true, false);
	s.load(Sounds::death_enemy, "Sounds/DeathEnemy.wav", false, false);
	s.load(Sounds::door_end, "Sounds/DoorEnd.wav", true, false);
	s.load(Sounds::door, "Sounds/DoorLoop.wav", true, true);
	s.load(Sounds::fall, "Sounds/Fall.wav", true, false);
	s.load(Sounds::fire_enemy, "Sounds/FireEnemy.wav", false, false);
	s.load(Sounds::fire_player, "Sounds/FirePlayer.wav", false, false);
	s.load(Sounds::jump, "Sounds/Jump.wav", true, false);
	s.load(Sounds::walk, "Sounds/Walk.wav", true, true);
	s.load(Sounds::intro, "Sounds/Intro.wav", true, false);
	s.load(Sounds::environment, "Sounds/Sewers.wav", true, true);

	ThingTextures::global().load("barrel", 365, 267, 1, "Textures/barrel.png");
	ThingTextures::global().load("enemy1", 512, 335, 9, "Textures/enemy.png");

	Player::instance().set_gun(window, "Textures/1.png");
	Player::instance().set_gun_animation({ "Textures/2.png", "Textures/3.png" ,"Textures/4.png" , "Textures/5.png" ,"Textures/6.png" ,"Textures/6.png" });

	controller_.set(window);

	sf::Font font;
	sf::Text text;

	font.loadFromFile("Fonts/roboto.ttf");
	text.setFont(font);
	text.setColor(sf::Color::Red);
	text.setStyle(sf::Text::Bold);

	load("Maps/e1m1.map");
	Sounds::global().play(Sounds::intro, 100);

	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
		}

		sf::Time elapsed = clock.restart();
		std::stringstream ss;

#ifdef DEBUG
		ss << "dir:" << Player::instance().get_direction() << std::endl
			<< "pos: " << Player::instance().get_position().X
			<< ", " << Player::instance().get_position().Y
			<< ", " << Player::instance().get_eye_level() << std::endl
			<< "Sec: " << Player::instance().get_sec() << std::endl;
#endif // DEBUG

		float frame_rate = 1.f / elapsed.asSeconds();
		ss << "FPS: " << frame_rate << std::endl
			<< "hp:" << Player::instance().get_health() << std::endl;

		text.setCharacterSize(20);
		text.setString(ss.str());

		window.clear();
		update(elapsed.asMilliseconds());
		draw(window, sf::RenderStates::Default);

		window.draw(text);

		if (!Player::instance().is_alive()) {

			sf::Text message = text;
			message.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 20.0f));
			message.setCharacterSize(50);
			message.setString("Game over :(");

			sf::FloatRect textRect;

			textRect = message.getLocalBounds();
			message.setOrigin(textRect.left + textRect.width / 2.0f,
				textRect.top + textRect.height / 2.0f);

			window.draw(message);

			sf::Text esc = text;
			esc.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f + 35.0f));
			esc.setCharacterSize(25);
			esc.setString("Press ESC to exit");

			textRect = esc.getLocalBounds();
			esc.setOrigin(textRect.left + textRect.width / 2.0f,
				textRect.top + textRect.height / 2.0f);

			window.draw(esc);

		}

		window.display();
	}

}

void Game::update(int millis_elapsed) {

	if (!Player::instance().is_alive()) return;

	// update game map (i.e. doors, enemies, shooting)
	map_.update(millis_elapsed / 1000.0f);

	// update player state and position
	Player::instance().update_shooting(millis_elapsed);
	Player::instance().set_sneaking(controller_.sneak());

	float new_angle = controller_.rotation() / std::tan((float)scene_.get_fov() / 2.0f);
	Player::instance().move_to(millis_elapsed / 1000.0f, controller_.direction(), new_angle);

	if (controller_.jump() && !controller_.sneak()) { Player::instance().jump(); }
	if (controller_.fire()) Player::instance().fire();

	Sounds::global().play(Sounds::environment, 3);

}

void Game::draw(sf::RenderTarget & target, sf::RenderStates states) const {

	target.draw(scene_);
	target.draw(Player::instance());

}