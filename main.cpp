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
const double PI = atan(1.) + atan(2.) + atan(3.);

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
const int MAX_SNOWBALL = 256;
const int SB_COUNT = 16;
const double SB_RADIUS = 6.;

class Particle {
public:
    double life;
    double x, y, lx, ly;
    double xv, yv;
    int type;
    Particle * next = NULL;
    int sbidx = -1;

    Particle() {
    }
    ~Particle() {
    }
};

class Snowball {
public:
    Particle *prt[SB_COUNT];
    double cx, cy;
    double minx, miny, maxx, maxy;
    double life;
    int idx;
    Snowball * next = NULL;

    Snowball() {
        life = -1.;
    }
    ~Snowball() {

    }

    void updateMinMaxCenter() {
        cx = 0.; cy = 0.;
        minx = 10000.; maxx = -10000.;
        miny = 10000.; maxy = -10000.;
        for (int i=0; i<SB_COUNT; i++) {
            Particle * P = prt[i];
            cx += P->x; cy += P->y;
            if (P->x < minx) minx = P->x;
            if (P->y < miny) miny = P->y;
            if (P->x > maxx) maxx = P->x;
            if (P->y > maxy) maxy = P->y;
        }
        cx /= (double)SB_COUNT;
        cy /= (double)SB_COUNT;
        for (int i=0; i<SB_COUNT; i++) {
            for (int j=i+1; j<SB_COUNT; j++) {
                double angi = atan2(prt[i]->y - cy, prt[i]->x - cx);
                double angj = atan2(prt[j]->y - cy, prt[j]->x - cx);
                if (angj < angi) {
                    Particle * tmp = prt[i];
                    prt[i] = prt[j];
                    prt[j] = tmp;
                }
            }
        }
    }

    void update(double dt) {
        dt *= 60.;
        updateMinMaxCenter();
        double ang0 = atan2(prt[0]->y - cy, prt[0]->x - cx);
        for (int i=0; i<SB_COUNT; i++) {
            Particle * P = prt[i];
            double a = (double)i / (double)SB_COUNT * PI * 2. + ang0;
            double ex = cx + cos(a) * SB_RADIUS;
            double ey = cy + sin(a) * SB_RADIUS;
            double dx = P->x - ex, dy = P->y - ey;
            double len = sqrt(dx*dx+dy*dy);
            if (len > 0) {
                P->x -= dx * dt * 0.25;
                P->y -= dy * dt * 0.25;
            }
        }
        for (int i=0; i<SB_COUNT; i++) {
            double a = (double)i / (double)SB_COUNT * PI * 2.;
            double exi = cx + cos(a) * SB_RADIUS;
            double eyi = cy + sin(a) * SB_RADIUS;
            for (int j=0; j<SB_COUNT; j++) {
                if (i == j) {
                    continue;
                }
                double b = (double)j / (double)SB_COUNT * PI * 2.;
                double exj = cx + cos(b) * SB_RADIUS;
                double eyj = cy + sin(b) * SB_RADIUS;
                double exdx = exi - exj, exdy = eyi - eyj;
                double exdist = sqrt(exdx*exdx+exdy*exdy);
                
                Particle * A = prt[i], * B = prt[j];
                double dx = A->x - B->x, dy = A->y - B->y;
                double len = sqrt(dx*dx+dy*dy);
                if (len > exdist) {
                    double F = fmin(1., (len - exdist) / exdist);
                    A->x -= dx / len * dt * F;
                    A->y -= dy / len * dt * F;
                    B->x -= dx / len * dt * F;
                    B->y -= dy / len * dt * F;
                }
                else if (len < exdist) {
                    double F = fmin(1., (exdist - len) / exdist);
                    A->x += dx / len * dt * F;
                    A->y += dy / len * dt * F;
                    B->x -= dx / len * dt * F;
                    B->y -= dy / len * dt * F;
                }
            }
        }
    }

    bool pointInside(double x, double y) {
        if (x < minx || y < miny || x > maxx || y > maxy) {
            return false;
        }
        int i, j, c = 0;
        for (i = 0, j = SB_COUNT-1; i < SB_COUNT; j = i++) {
            if (((prt[i]->y>=y) != (prt[j]->y>=y)) && (x <= (prt[j]->x-prt[i]->x) * (y-prt[i]->y) / (prt[j]->y-prt[i]->y) + prt[i]->x)) {
                c = 1 - c;
            }
        }
        return c == 1;
    }
};

class Physics {
public:
    Particle * prt = NULL;
    Terrain * terrain = NULL;
    Particle ** hash = NULL;
    Snowball ** shash = NULL;
    Texture renderedTex;
    Sprite rendered;
    uint32_t * bfr = NULL;
    Snowball * snowb = NULL;
    const int width = 320, height = 240;
    const int swidth = 32, sheight = 24;
    int newPrtIdx = 0;
    Physics(Terrain * t = NULL) {
        terrain = t;
        prt = new Particle[MAX_PARTICLE];
        hash = new Particle*[width*height];
        shash = new Snowball*[swidth*sheight];
        renderedTex.create(width, height);
        bfr = new uint32_t[width * height];
        snowb = new Snowball[MAX_SNOWBALL];
        renderedTex.setSmooth(false);
        rendered.setTexture(renderedTex);
        clear();
    }
    ~Physics() {
        terrain = NULL;
        delete hash;
        delete shash;
        delete snowb;
        delete prt;
    }
    void clear() {
        for (int i=0; i<MAX_PARTICLE; i++) {
            prt[i].life = -1.;
            prt[i].next = NULL;
        }
        for (int i=0; i<MAX_SNOWBALL; i++) {
            snowb[i].life = -1.;
        }
        for (int i=0; i<(width*height); i++) {
            hash[i] = NULL;
        }
        for (int i=0; i<(swidth*sheight); i++) {
            shash[i] = NULL;
        }
    }

    void updateHash() {
        const int cnt = width*height;
        for (int i=0; i<cnt; i++) {
            hash[i] = NULL;
        }
        const int scnt = swidth*sheight;
        for (int i=0; i<scnt; i++) {
            shash[i] = NULL;
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
        for (int i=0; i<MAX_SNOWBALL; i++) {
            snowb[i].next = NULL;
        }
        for (int i=0; i<MAX_SNOWBALL; i++) {
            Snowball * S = snowb + i;
            if (S->life <= 0.) {
                continue;
            }
            int hx = (int)floor(S->cx / 10.), hy = (int)floor(S->cy / 10.);
            if (hx < 0) {
                hx = 0;
            }
            if (hy < 0) {
                hy = 0;
            }
            if (hx >= swidth) {
                hx = swidth-1;
            }
            if (hy >= sheight) {
                hy = sheight-1;
            }
            int hpos = hx + hy * swidth;
            S->next = shash[hpos];
            shash[hpos] = S;
        }
    }

    int addParticle(Particle * P) {
        int k = MAX_PARTICLE;
        while (prt[newPrtIdx].life > 0. && P->type == 2 && k-- > 0) {
            newPrtIdx = (newPrtIdx + 1) % MAX_PARTICLE;
        }
        P->lx = P->x; P->ly = P->y;
        prt[newPrtIdx] = *P;
        int idx = newPrtIdx;
        newPrtIdx = (newPrtIdx + 1) % MAX_PARTICLE;
        return idx;
    }

    int addSnowball(double cx, double cy) {
        int id = -1;
        for (int i=0; i<MAX_SNOWBALL; i++) {
            Snowball * S = snowb + i;
            if (S->life <= 0.) {
                id = i;
                break;
            }
        }
        if (id < 0) {
            return id;
        }
        Snowball * S = snowb + id;
        S->idx = id;
        for (int i=0; i<SB_COUNT; i++) {
            Particle P;
            P.life = 1.;
            P.type = 2;
            P.xv = P.yv = 0.;
            double a = (double)i / (double)SB_COUNT * PI * 2.;
            P.x = cx + cos(a) * SB_RADIUS;
            P.y = cy + sin(a) * SB_RADIUS;
            P.sbidx = id;
            int idx = addParticle(&P);
            S->prt[i] = prt + idx;
        }
        S->life = 1.;
        S->update(1. / 60.);
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
            if (P->type == 1) {
                bfr[bpos] = 0xFF0000B0;
            }
            else if (P->type == 2) {
                //bfr[bpos] = 0xDFDFFFFF;
            }
        }
        for (int i=0; i<MAX_SNOWBALL; i++) {
            Snowball * S = snowb + i;
            if (S->life > 0.) {
                S->updateMinMaxCenter();
            }
        }
        for (int x=0; x<width; x++) {
            for (int y=0; y<height; y++) {
                bool pfound = false;
                for (int ox=-1; ox<=1 && !pfound; ox++) {
                    for (int oy=-1; oy<=1 && !pfound; oy++) {
                        int hx = x/10 + ox, hy = y/10 + oy;
                        if (hx < 0 || hy < 0 || hx >= swidth || hy >= sheight) {
                            continue;
                        }
                        int hpos = hx + hy * swidth;
                        Snowball * head = shash[hpos];
                        while (head != NULL) {
                            if (head->pointInside((double)x, (double)y)) {
                                int bpos = x + y * width;
                                bfr[bpos] = 0xDFDFFFFF;
                                pfound = true;
                                break;
                            }
                            head = head->next;
                        }
                    }
                }
            }
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
            P.sbidx = -1;
            addParticle(&P);
        }

        updateHash();

        int substeps = 8;

        double subdt = 1. / (double)substeps;
        dt *= subdt;

        for (int K=0; K<substeps; K++) {

            double dampf = pow(0.5, dt);

            for (int i=0; i<MAX_SNOWBALL; i++) {
                Snowball * S = snowb + i;
                if (S->life <= 0.) {
                    continue;
                }
                S->update(dt);
            }

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
                                double F = 1.;
                                if (P->sbidx >= 0) {
                                    F = 2.;
                                }
                                P->x += dx / len * 0.5 * (1. - len) * F;
                                P->y += dy / len * 0.5 * (1. - len) * F;
                            }
                        }
                    }
                }

                for (int ox=-1; ox<=1; ox++) {
                    for (int oy=-1; oy<=1; oy++) {
                        int hx = (int)floor(P->x / 10.) + ox, hy = (int)floor(P->y / 10.) + oy;
                        if (hx < 0 || hy < 0 || hx >= swidth || hy >= sheight) {
                            continue;
                        }
                        int hpos = hx + hy * swidth;
                        Snowball * head = shash[hpos];
                        while (head != NULL) {

                            if (head->idx != P->sbidx && head->pointInside(P->x, P->y)) {
                                double dx = P->x - head->cx,
                                       dy = P->y - head->cy;
                                double len = sqrt(dx*dx+dy*dy);
                                if (len < 0.001) {
                                    len = 0.001;
                                }
                                dx /= len; dy /= len;
                                P->x += dx * (SB_RADIUS * 1.5 - len) * 0.1;
                                P->y += dy * (SB_RADIUS * 1.5 - len) * 0.1;
                            }

                            head = head->next;
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

                                        if (itA != itB && (itA->sbidx == -1 || itA->sbidx != itB->sbidx)) {

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

    window = new RenderWindow(VideoMode(800, 600), "Night Of The Living Snowballs");

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
    bg.setColor(Color(192, 128, 255, 128));

    Sprite castle(castleTex);

    window->setFramerateLimit(60);

    physics.addSnowball(145., 50.);
    physics.addSnowball(155., 60.);
    physics.addSnowball(170., 70.);
    physics.addSnowball(185., 80.);
    physics.addSnowball(200., 90.);
    
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