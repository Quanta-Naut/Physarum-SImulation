#include <SFML/Graphics.hpp>
#include <vector>
#include <windows.h>
#include <random>

#ifndef PI
    #define PI 3.14159265358979323846
#endif

using namespace sf;
using namespace std;

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int antsNum = 200000;
const int sensorOffset = 80;
const int sensorAngle = 28;
const Color antColor(221, 236, 238);
const Color targetColor(163, 237, 247);
const int fadeRate = 3;
const float speed = 0.8f;
const int circleRadius = 500;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(1, 100);

struct Ant {
    Vector2f position;
    float angle;
};

class Ants {
public:
    int count = fadeRate;
    Ants() {
        ants.clear();
        Vector2f center = { WIDTH / 2.0f, HEIGHT / 2.0f };

        // Position ants in a circle and set their angles to face the center
        for (int i = 0; i < antsNum; ++i) {
            float angle = 2 * PI * i / antsNum; // Angle in radians
            Vector2f position = {
                center.x + circleRadius * std::cos(angle),
                center.y + circleRadius * std::sin(angle)
            };

            // Calculate the angle to face the center
            float angleToCenter = std::atan2(center.y - position.y, center.x - position.x);
            float angleInDegrees = angleToCenter * 180 / PI;
            float randomAngle = static_cast<float>(rand() % 360);

            // Add ant to the container
            ants.push_back({ position, randomAngle });
        }
    }
    void updateAngle(Image& image) {
        for (auto& a : ants) {
            float right = smell(a, sensorAngle, image);
            float center = smell(a, 0, image);
            float left = smell(a, -sensorAngle, image);

            if (center > left && center > right) {
                /*int randomNumber = distrib(gen);
                if (randomNumber == 50) {
                    a.angle += sensorAngle;
                }
                else if (randomNumber == 90) {
                    a.angle -= sensorAngle;
                }
                else {

                }*/
            }
            else if (left < right) a.angle += sensorAngle;
            else if (left > right) a.angle -= sensorAngle;
        }
    }

    void updatePosition(Image& image) {
        for (auto& a : ants) {
            a.position.x += cos(a.angle * PI / 180) * speed;
            a.position.y += sin(a.angle * PI / 180) * speed;

            if (a.position.x < 0) {
                a.position.x = 0; 
                a.angle = 180 - a.angle; 
            }
            else if (a.position.x >= WIDTH) {
                a.position.x = WIDTH - 1;
                a.angle = 180 - a.angle;
            }

            if (a.position.y < 0) {
                a.position.y = 0;
                a.angle = -a.angle; 
            }
            else if (a.position.y >= HEIGHT) {
                a.position.y = HEIGHT - 1; 
                a.angle = -a.angle; 
            }
            //a.position.x = fmod(a.position.x + WIDTH, WIDTH);
            //a.position.y = fmod(a.position.y + HEIGHT, HEIGHT);

            Uint8* pixels = const_cast<Uint8*>(image.getPixelsPtr());
            int index = (static_cast<int>(a.position.x) + static_cast<int>(a.position.y) * WIDTH) * 4;
            pixels[index] = antColor.r;
            pixels[index + 1] = antColor.g;
            pixels[index + 2] = antColor.b;
            pixels[index + 3] = 255;
        }
    }

    void fadeTrail1(Image& image) {
        Uint8* pixels = const_cast<Uint8*>(image.getPixelsPtr());
        int numPixels = WIDTH * HEIGHT * 4;
        if (count == fadeRate) {
            for (int i = 0; i < numPixels; i += 4) {
                if (pixels[i + 3] > 0) {
                    pixels[i + 3] = max(0, static_cast<int>(pixels[i + 3] - 0.1f));
                }
            }
            count = 0;
        }
        else {
            count++;
        }
    }
    void fadeTrail(Image& image) {
        
        Uint8* pixels = const_cast<Uint8*>(image.getPixelsPtr());
        int numPixels = WIDTH * HEIGHT * 4;

        if (count == fadeRate) {
            for (int i = 0; i < numPixels; i += 4) {
                // Only process pixels that are not fully transparent
                if (pixels[i + 3] > 0 && pixels[i] > 0 && pixels[i + 1] > 0 && pixels[i + 2] > 0) {
                    // Gradually move the RGB channels towards the target color
                    pixels[i] = static_cast<Uint8>(pixels[i] - 0.1f * (pixels[i] - targetColor.r)); // Red
                    pixels[i + 1] = static_cast<Uint8>(pixels[i + 1] - 0.1f * (pixels[i + 1] - targetColor.g)); // Green
                    pixels[i + 2] = static_cast<Uint8>(pixels[i + 2] - 0.1f * (pixels[i + 2] - targetColor.b)); // Blue

                    // Fade the alpha value
                    pixels[i + 3] = std::max(0, static_cast<int>(pixels[i + 3] - 3)); // Adjust fade speed as needed
                }
            }
            count = 0;
        }
        else {
            count++;
        }
    }
private:
    vector<Ant> ants;

    float smell(Ant& a, float d, Image& image) {
        float aim = a.angle + d;
        int x = static_cast<int>(a.position.x + sensorOffset * cos(aim * PI / 180));
        int y = static_cast<int>(a.position.y + sensorOffset * sin(aim * PI / 180));

        x = (x + WIDTH) % WIDTH;
        y = (y + HEIGHT) % HEIGHT;

        Uint8* pixels = const_cast<Uint8*>(image.getPixelsPtr());
        int index = (x + y * WIDTH) * 4;
        return pixels[index + 3];
    }
};

void drawCircle(sf::Image& image, int centerX, int centerY, int radius, sf::Color color) {
    int width = image.getSize().x;
    int height = image.getSize().y;

    // Iterate over the pixels of the image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Calculate distance from the current pixel to the circle center
            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy <= radius * radius) {
                // Set pixel color if it's within the circle's radius
                image.setPixel(x, y, color);
            }
        }
    }
}

int main()
{   
    srand(static_cast<unsigned>(time(nullptr)));
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "SFML works!");
    ShowWindow(window.getSystemHandle(), SW_MAXIMIZE);

    Texture texture;
    texture.create(WIDTH, HEIGHT);
    Image image;

    image.create(WIDTH, HEIGHT, Color::Black);
    //drawCircle(image, 100, 100, 10, Color(255, 0, 0));
    texture.update(image);

    Ants ants;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        window.clear();

        ants.updateAngle(image);
        ants.updatePosition(image);
        ants.fadeTrail(image);
        //drawCircle(image, 300, 300, 5, Color(0, 247, 239));
        //drawCircle(image, 500, 500, 5, Color(0, 247, 239));
        //drawCircle(image, 800, 800, 5, Color(0, 247, 239));
        texture.update(image);
        Sprite sprite(texture);
        window.draw(sprite);

        window.display();
    }

    return 0;
}