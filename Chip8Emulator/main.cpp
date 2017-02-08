#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include "Chip8.h"

int main(int aC, char** aV)
{
	if (aC < 2)
		return 1;

	FILE* f;
	if (fopen_s(&f, aV[1], "rb") != 0)
		return 2;

	const int W = 640, H = 320;
	Chip8 c8(f);
	int longSound = 0;

	sf::RenderWindow window(sf::VideoMode(W, H), "Chip8 Emulator");

	sf::Texture texture;
	texture.create(64, 32);
	sf::Sprite sprite(texture);
	sprite.scale(W >> 6, H >> 5);
	
	sf::Keyboard::Key keys[] =
	{
		sf::Keyboard::X,
		sf::Keyboard::Num1,
		sf::Keyboard::Num2,
		sf::Keyboard::Num3,
		sf::Keyboard::Q,
		sf::Keyboard::W,
		sf::Keyboard::E,
		sf::Keyboard::A,
		sf::Keyboard::S,
		sf::Keyboard::D,
		sf::Keyboard::Z,
		sf::Keyboard::C,
		sf::Keyboard::Num4,
		sf::Keyboard::R,
		sf::Keyboard::F,
		sf::Keyboard::V
	};

	sf::Sound s;
	sf::SoundBuffer sBuffer;
	{
		const int samples = 256;
		sf::Int16 raw[samples];
		memset(raw, 0, sizeof(raw) >> 1);
		memset(raw + (samples >> 1), 2, sizeof(raw) >> 1);

		if (!sBuffer.loadFromSamples(raw, samples, 1, 44100))
			return 3;

		s.setBuffer(sBuffer);
		s.setLoop(true);
	}

	sf::Clock clock;
	sf::Time elapsedTime = sf::Time::Zero;
	sf::Time executionTime = sf::microseconds(16667);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();

		elapsedTime += clock.restart();
		if (elapsedTime > executionTime)
		{
			elapsedTime -= executionTime;
			for (int i = 0; i < 16; ++i)
				c8.setKey(i, sf::Keyboard::isKeyPressed(keys[i]));

			c8.cycle(10);

			if (c8.playSound())
				longSound = 4;

			if (longSound)
			{
				--longSound;
				if (s.getStatus() == sf::Sound::Status::Stopped)
					s.play();
			}
			else
				s.stop();

			if (c8.canDraw())
			{
				texture.update((unsigned char*)c8.getDisplay());
				window.clear();
				window.draw(sprite);
				window.display();
			}
		}
	}

	return 0;
}