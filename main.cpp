#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace sf;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::set;

RenderWindow * window = NULL;

int VP_WIDTH = 800, VP_HEIGHT = 600;

void AutoTransform ( Sprite * sprite ) {
    double aspectA = (double)VP_WIDTH / (double)VP_HEIGHT;
    double aspect = 320. / 240.;
    double scale = aspectA < aspect ? ((double)VP_WIDTH / 320.) : ((double)VP_HEIGHT / 240.);
    sprite->setScale(scale, scale);
    Vector2f pos = sprite->getPosition();
    pos.x -= 320. * 0.5;
    pos.y -= 240. * 0.5;
    pos.x *= scale;
    pos.y *= scale;
    pos.x += (double)VP_WIDTH * 0.5;
    pos.y += (double)VP_HEIGHT * 0.5;
    sprite->setPosition(pos);
}

void AutoTransform ( Sprite & sprite ) {
    AutoTransform(&sprite);
}

class Terrain {
public:
    Texture tilesetTex;
    Sprite tileset;
    Image tilesetImg;
    Texture renderedTex;
    Sprite rendered;
    uint32_t * bfr = NULL;
    int width = 320, height = 240;
    int tileSize = 16, tsSize = 128 / 16;

    Terrain() {
        if (!tilesetTex.loadFromFile("sprites/tiles.png")) {
            cerr << "Tileset not found" << endl;
            exit(0);
        }

        tilesetImg = tilesetTex.copyToImage();

        tilesetTex.setSmooth(false);
        tileset.setTexture(tilesetTex);
        renderedTex.create(width, height);

        bfr = new uint32_t[width * height];

        resetLevel();

        renderedTex.setSmooth(false);
        rendered.setTexture(renderedTex);
    }

    void resetLevel() {
        memset(bfr, 0, sizeof(uint32_t)*width*height);

        const int TW = 20, TH = 15;

        int tileMap[TH][TW] = {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
            { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
        };

        for (int x=0; x<TW; x++) {
            for (int y=0; y<TH; y++) {
                int x0 = x > 0 ? tileMap[y][x-1] : 0,
                    y0 = y > 0 ? tileMap[y-1][x] : 0,
                    x1 = x < (TW-1) ? tileMap[y][x+1] : 0,
                    y1 = y < (TH-1) ? tileMap[y+1][x] : 0;
                if (tileMap[y][x]) {
                    int xx = 0, yy = 0;
                    if (!x0) {
                        xx -= 1;
                    }
                    if (!x1) {
                        xx += 1;
                    }
                    if (!y0) {
                        yy -= 1;
                    }
                    if (!y1) {
                        yy += 1;
                    }
                    drawTile(1 + xx, 1 + yy, x, y);
                }
            }
        }
        renderedTex.update((Uint8*)bfr);
    }

    void drawTile(int idX, int idY, int sx, int sy) {
        int rx = idX * tileSize;
        int ry = idY * tileSize;
        uint32_t * ptr = (uint32_t*)tilesetImg.getPixelsPtr();
        sx *= tileSize; sy *= tileSize;
        int twpx = tileSize * tsSize;
        for (int x=0; x<tileSize; x++) {
            for (int y=0; y<tileSize; y++) {
                int sx2 = x + sx, sy2 = y + sy;
                if (sx2 >= 0 && sy2 >= 0 && sx2 < width && sy2 < height) {;
                    bfr[sx2 + sy2 * width] = ptr[rx + x + (ry + y) * twpx];
                }
            }
        }
    }

    ~Terrain() {
        if (bfr != NULL) {
            delete bfr;
            bfr = NULL;
        }
    }

    void render (double dt) {
        rendered.setPosition(Vector2f(0., 0.));
        AutoTransform(rendered);
        window->draw(rendered);
    }
};

Terrain * terrain = NULL;

int main() {

    window = new RenderWindow(VideoMode(800, 600), "Day Of The Living Snowballs");

    terrain = new Terrain();

    Texture bgTex, castleTex;
    if (!bgTex.loadFromFile("sprites/bg.png")) {
        delete window; delete terrain;
        cerr << "Error loading bg.png" << endl;
        exit(0);
    }
    if (!castleTex.loadFromFile("sprites/castle.png")) {
        delete window; delete terrain;
        cerr << "Error loading castle.png" << endl;
        exit(0);
    }

    Sprite bg(bgTex);
    bg.setColor(Color(255, 255, 255, 255));

    Sprite castle(castleTex);

    window->setFramerateLimit(60);
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed) {
                window->close();
            }
            else if (event.type == Event::Resized) {
                VP_WIDTH = window->getSize().x;
                VP_HEIGHT = window->getSize().y;
	            window->setView(View(FloatRect(0.f, 0.f, (float)VP_WIDTH, (float)VP_HEIGHT)));
            }
        }

        window->clear(Color::Black);

        bg.setPosition(Vector2f(0., 0.));
        AutoTransform(bg);
        window->draw(bg);

        castle.setPosition(Vector2f(16. * 1., 16. * 1.));
        AutoTransform(castle);
        window->draw(castle);

        terrain->render(1. / 60.);

        window->display();
    }

    delete terrain;
    delete window;

    return 0;
}