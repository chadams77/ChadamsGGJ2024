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
    const int width = 320, height = 240;
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

const int MAX_PARTICLE = 16384;

class Particle {
public:
    double life;
    double x, y, lx, ly;
    double xv, yv;
    int type;
    Particle * next = NULL;

    Particle() {
    }
    ~Particle() {
    }
};

class Physics {
public:
    Particle * prt = NULL;
    Terrain * terrain = NULL;
    Particle ** hash = NULL;
    Texture renderedTex;
    Sprite rendered;
    uint32_t * bfr = NULL;
    const int width = 320, height = 240;
    int newPrtIdx = 0;
    Physics(Terrain * t = NULL) {
        terrain = t;
        prt = new Particle[MAX_PARTICLE];
        hash = new Particle*[width*height];
        renderedTex.create(width, height);
        bfr = new uint32_t[width * height];
        renderedTex.setSmooth(false);
        rendered.setTexture(renderedTex);
        clear();
    }
    ~Physics() {
        terrain = NULL;
        delete hash;
        delete prt;
    }
    void clear() {
        for (int i=0; i<MAX_PARTICLE; i++) {
            prt[i].life = -1.;
            prt[i].next = NULL;
        }
        for (int i=0; i<(width*height); i++) {
            hash[i] = NULL;
        }
    }

    void updateHash() {
        const int cnt = width*height;
        for (int i=0; i<cnt; i++) {
            hash[i] = NULL;
        }
        for (int i=0; i<MAX_PARTICLE; i++) {
            prt[i].next = NULL;
        }
        for (int i=0; i<MAX_PARTICLE; i++) {
            Particle * P = prt + i;
            if (P->life <= 0.) {
                continue;
            }
            int hx = (int)floor(P->x), hy = (int)floor(P->y);
            if (hx < 0) {
                hx = 0;
            }
            if (hy < 0) {
                hy = 0;
            }
            if (hx >= width) {
                hx = width-1;
            }
            if (hy >= height) {
                hy = height-1;
            }
            int hpos = hx + hy * width;
            P->next = hash[hpos];
            hash[hpos] = P;
        }
    }

    void addParticle(Particle * P) {
        P->lx = P->x; P->ly = P->y;
        prt[newPrtIdx] = *P;

        newPrtIdx = (newPrtIdx + 1) % MAX_PARTICLE;
    }

    void render(double dt) {
        memset(bfr, 0, width*height*sizeof(uint32_t));
        for (int i=0; i<MAX_PARTICLE; i++) {
            Particle * P = prt + i;
            if (P->life <= 0.) {
                continue;
            }
            int hx = (int)floor(P->x), hy = (int)floor(P->y);
            if (hx < 0 || hy < 0 || hx >= width || hy >= height) {
                continue;
            }
            int bpos = hx + hy * width;
            bfr[bpos] = 0xFF0000B0;
        }
        renderedTex.update((Uint8*)bfr);
        rendered.setPosition(Vector2f(0., 0.));
        AutoTransform(rendered);
        window->draw(rendered);
    }

    void update(double dt) {
        const int cnt = width*height;
        if ((rand()%100) < 100) {
            Particle P;
            P.life = 1.;
            P.type = 1;
            P.x = (double)(rand() % 3200) / 10.;
            P.y = -(double)(rand() % 200) / 10.;
            P.xv = (double(rand() % 20)) - 10.;
            P.yv = 0.;
            addParticle(&P);
        }

        updateHash();

        int substeps = 8;

        double subdt = 1. / (double)substeps;
        dt *= subdt;

        for (int K=0; K<substeps; K++) {

            double dampf = pow(0.5, dt);

            for (int i=0; i<MAX_PARTICLE; i++) {
                Particle * P = prt + i;
                if (P->life <= 0.) {
                    continue;
                }

                P->xv = (P->x - P->lx) * dampf;
                P->yv = (P->y - P->ly) * dampf;
                P->yv += 32. * dt;
                P->x += P->xv;
                P->y += P->yv;
                P->lx = P->x;
                P->ly = P->y;

                for (int ox=-1; ox<=1; ox++) {
                    for (int oy=-1; oy<=1; oy++) {
                        int hx = (int)floor(P->x) + ox, hy = (int)floor(P->y) + oy;
                        if (hx < 0 || hy < 0 || hx >= width || hy >= height) {
                            continue;
                        }
                        int bpos = hx + hy * width;

                        if ((terrain->bfr[bpos] >> 24u) & 0xFFu) {
                            double dx = P->x - (floor(P->x) + 0.5 + (double)ox),
                                   dy = P->y - (floor(P->y) + 0.5 + (double)oy);
                            double lenSq = dx*dx+dy*dy;
                            if (lenSq < 1.) {
                                double len = sqrt(lenSq);
                                if (len < 0.001) {
                                    len = 0.001;
                                }
                                P->x += dx / len * 0.5 * (1. - len);
                                P->y += dy / len * 0.5 * (1. - len);
                            }
                        }
                    }
                }

            }

            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    int hpos = x + y * width;
                    Particle * head = hash[hpos];
                    if (head != NULL) {
                        for (int ox=-1; ox<=1; ox++) {
                            if (!x && ox<0) {
                                continue;
                            }
                            if (x == (width-1) && ox>0) {
                                continue;
                            }
                            for (int oy=-1; oy<=1; oy++) {
                                if (!y && oy<0) {
                                    continue;
                                }
                                if (y == (height-1) && oy>0) {
                                    continue;
                                }
                                int hpoff = ox + oy * width;
                                Particle * headB = hash[hpos + hpoff];
                                Particle * itA = head, * itB = headB;
                                while (itA != NULL) {
                                    itB = headB;
                                    while (itB != NULL) {

                                        double dx = itB->x - itA->x,
                                               dy = itB->y - itA->y;
                                        double lenSq = dx*dx+dy*dy;
                                        if (lenSq < 1.) {
                                            double len = sqrt(lenSq);
                                            if (len < 0.001) {
                                                len = 0.001;
                                            }
                                            itA->x -= dx / len * 0.5 * 0.2 * (1. - len);
                                            itA->y -= dy / len * 0.5 * 0.2 * (1. - len);
                                            itB->x += dx / len * 0.5 * 0.2 * (1. - len);
                                            itB->y += dy / len * 0.5 * 0.2 * (1. - len);
                                        }

                                        itB = itB->next;
                                    }
                                    itA = itA->next;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

Terrain * terrain = NULL;

int main() {

    window = new RenderWindow(VideoMode(800, 600), "Day Of The Living Snowballs");

    terrain = new Terrain();
    Physics physics(terrain);

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
    bg.setColor(Color(255, 255, 255, 128));

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

        physics.update(1. / 60.);
        physics.render(1. / 60.);
        terrain->render(1. / 60.);

        window->display();
    }

    delete terrain;
    delete window;

    return 0;
}