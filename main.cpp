#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

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

void InvTransform ( int sx, int sy, double & rx, double & ry ) {
    double aspectA = (double)VP_WIDTH / (double)VP_HEIGHT;
    double aspect = 320. / 240.;
    double scale = aspectA < aspect ? ((double)VP_WIDTH / 320.) : ((double)VP_HEIGHT / 240.);
        
    rx = (double)sx;
    ry = (double)sy;
    rx -= (double)VP_WIDTH * 0.5;
    ry -= (double)VP_HEIGHT * 0.5;
    rx /= scale;
    ry /= scale;
    rx += 320. * 0.5;
    ry += 240. * 0.5;
}

Texture bgTex, castleTex, eyesTex, eyes2Tex, cursorTex, arrowTex, bombTex, spawnTex, numbersTex, defeatTex, titleTex;
Music music;
SoundBuffer arrowSfx, expSfx, getBombSfx, sbHurtSfx, sbJumpSfx, sbVoice1, sbVoice2;
uint32_t GAME_SCORE = 0, NUM_BOMBS = 0, GAME_SCORE_TO;
const int MAX_SOUNDS = 64;
int soundIdx = 0;
Sound sounds[MAX_SOUNDS];

void playSound(SoundBuffer & bfr, double rate=1., double vol=1.) {
    sounds[soundIdx].setBuffer(bfr);
    sounds[soundIdx].setVolume(vol*100.);
    sounds[soundIdx].setPitch(rate);
    sounds[soundIdx].play();
    soundIdx = (soundIdx + 1) % MAX_SOUNDS;
}

class Numbers {
public:
    Sprite * sprites;
    int size;

    Numbers(int sz) {
        size = sz;
        sprites = new Sprite[size+1];
        for (int i=0; i<(size+1); i++) {
            sprites[i].setTexture(numbersTex);
        }
    }

    ~Numbers() {
        delete sprites;
    }

    void draw(double x, double y, uint32_t value, Color clr = Color::Black, int icon=0) {
        sprites[size].setTextureRect(IntRect((icon+10)*8, 0, 8, 8));
        sprites[size].setPosition(x, y);
        //sprites[size].setColor(clr);
        AutoTransform(sprites[size]);
        window->draw(sprites[size]);

        uint32_t p = 1;
        for (int i=0; i<size; i++) {
            p *= 10;
        }

        for (int i=size-1; i>=0; i--) {
            x += 8.;
            p /= 10;
            int digit = (value / p) % 10;
            sprites[i].setTextureRect(IntRect(digit*8, 0, 8, 8));
            sprites[i].setPosition(x, y);
            sprites[i].setColor(clr);
            AutoTransform(sprites[i]);
            window->draw(sprites[i]);
        }
    }
};

class Castle {
public:
    double damageT = 0.;
    double displayLife = 0.;
    double life = 100.;
    bool dead;
    double deathT;
    Sprite spr;
    Sprite hud1, hud2, hud3, def;
    
    Castle() {
        spr = Sprite(castleTex);
        hud1 = Sprite(numbersTex);
        hud2 = Sprite(numbersTex);
        hud3 = Sprite(numbersTex);
        def = Sprite(defeatTex);
        dead = false;
        deathT = 0.;
    }
    ~Castle() {

    }

    void damage(double amt) {
        if (life > 0.) {
            life -= amt * 1.0001;
            damageT += 1.0;
            if (life <= 0. && !dead) {
                dead = true;
                deathT = 0.;
                playSound(expSfx, 0.3, 1.);
            }
        }
    }

    void render(double dt) {
        double rx = 0., ry = 0.;
        Color clr(255, 255, 255, 255);
        if (dead) {
            damageT = 1.;
            deathT += dt;
            if (0==(rand()%12)) {
                playSound(expSfx, 0.5, 1.);
            }
        }
        if (damageT > 0.) {
            damageT -= dt;
            rx = (double)(rand() % 100) / 50. - 0.5;
            ry = (double)(rand() % 100) / 50. - 0.5;
            rx *= damageT * 3.;
            ry *= damageT * 3.;
            clr.g = (clr.g * (int)(255. - 255. * damageT)) / 256;
            clr.b = (clr.b * (int)(255. - 255. * damageT)) / 256;
        }
        else {
            damageT = 0.;
        }
        spr.setColor(clr);
        spr.setPosition(Vector2f(16. * 1. + rx, 16. * 1. + ry + deathT * 24.));
        AutoTransform(spr);
        window->draw(spr);
    }

    bool renderHud(double dt) {
        displayLife += (fmax(0., fmin(100., life)) - displayLife) * dt * 8.;

        hud1.setTextureRect(IntRect(12*8, 0, 8, 8));
        hud1.setPosition(4., 4.);
        AutoTransform(hud1);
        window->draw(hud1);

        hud2.setTextureRect(IntRect(13*8, 0, 80, 8));
        hud2.setPosition(4. + 8., 4.);
        AutoTransform(hud2);
        window->draw(hud2);

        hud3.setTextureRect(IntRect(13*8 + 80, 0, (int)(80. * displayLife / 100.), 8));
        hud3.setPosition(4. + 8., 4.);
        AutoTransform(hud3);
        window->draw(hud3);

        if (dead && deathT > 5.) {
            int alpha = (int)(16. * fmin(1., (deathT - 5.) / 1.5)) * 16;
            if (alpha > 255) {
                alpha = 255;
            }
            int shade = 255;
            if (deathT > 9.) {
                shade = 255 - (int)((deathT - 9.) * 256.);
                if (shade < 0) {
                    shade = 0;
                }
                shade /= 16;
                shade *= 16;
            }
            def.setPosition(0., 0.);
            def.setColor(Color((uint8_t)shade, (uint8_t)shade, (uint8_t)shade, (uint8_t)alpha));
            AutoTransform(def);
            window->draw(def);
            if (deathT > 10.) {
                return true;
            }
        }

        return false;
    }
};

Castle * castle = NULL;

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

        GAME_SCORE = GAME_SCORE_TO = 0;
        NUM_BOMBS = 3;

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
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
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

const int MAX_PARTICLE = 4096;
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
    uint32_t clr = 0;

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
    double groundAgo;
    double groundFor;
    bool jumped, dying, attacking;
    double dieT;
    Sprite * eyesSpr = NULL;
    double hpx, hpy;

    Snowball() {
        life = -1.;
        groundAgo = 100.;
        groundFor = 0.;
        jumped = false;
        eyesSpr = NULL;
        dying = false;
        attacking = false;
    }
    ~Snowball() {
        delete eyesSpr;
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
        if (0==(rand() % (60 * (attacking ? 5 : 5)))) {
            attacking = !attacking;
            if (0 == (rand()%10)) {
                playSound(rand()%2 ? sbVoice1 : sbVoice2, 1., 0.5);
            }
        }
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
                P->x -= dx * dt * len * len * 0.002;
                P->y -= dy * dt * len * len * 0.002;
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
                    double F = (len - exdist)*(len - exdist) * 0.002;
                    A->x -= dx / len * dt * F;
                    A->y -= dy / len * dt * F;
                    B->x -= dx / len * dt * F;
                    B->y -= dy / len * dt * F;
                }
                else if (len < exdist) {
                    double F = (exdist - len)*(exdist - len) * 0.002;
                    A->x += dx / len * dt * F;
                    A->y += dy / len * dt * F;
                    B->x -= dx / len * dt * F;
                    B->y -= dy / len * dt * F;
                }
            }
        }
        if (groundAgo <= 0.001 && attacking) {
            groundFor += dt;
            if (groundFor > 0.25 && !jumped) {
                for (int i=0; i<SB_COUNT; i++) {
                    Particle * P = prt[i];
                    if (P->y < cy) {
                        P->y -= 0.1 * dt;
                        if (!dying) {
                            P->x -= 0.075 * dt;
                        }
                        else {
                            P->y -= 0.1 * dt;
                        }
                    }
                }
                updateMinMaxCenter();
                jumped = true;
                playSound(sbJumpSfx, 0.9 + 0.1 * (double)(rand()%10), 0.125);
            }
            else if (groundFor > .275) {
                jumped = false;
                groundFor = 1.;
            }
            groundAgo += dt;
        }
        else {
            jumped = false;
            groundFor = 0.;
            groundAgo += dt;
        }
        updateMinMaxCenter();
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

class SnowballDeath {
public:
    double cx, cy;
    double life;
    SnowballDeath(Snowball * S) {
        life = 3. / 5.;
        cx = S->cx;
        cy = S->cy;
    }
};

class Projectile {
public:
    double x, y;
    double xv, yv;
    int type;
    double t=0.;
    Sprite spr;
    const int width = 320, height = 240;

    Projectile(int _type=0) {
        type = _type;
        if (type==0) {
            spr.setTexture(arrowTex);
        }
        else {
            spr.setTexture(bombTex);
        }
    }
    ~Projectile() {}

    bool updateRender(double dt, Terrain * terrain, Snowball * snowb) {
        t += dt;

        const double dampF = exp(-dt * 1./0.95);
        xv *= dampF; yv *= dampF;
        yv += 0.1 * dt * 16.;
        x += xv; y += yv;
        int ix = (int)floor(x), iy = (int)floor(y);
        if (ix >= 0 && iy >= 0 && ix < width && iy < height) {
            int bpos = ix + iy * width;
            if (((terrain->bfr[bpos] >> 24u) & 0xFFu) > 0) {
                return false;
            }
        }
        else if (ix < 0 || ix >= width || iy >= height) {
            return false;
        }
        for (int i=0; i<MAX_SNOWBALL; i++) {
            Snowball * S = snowb + i;
            if (S->life > 0.) {
                if (S->pointInside(x, y)) {
                    if (!S->dying) {
                        playSound(sbHurtSfx, 1., 0.75);
                        playSound(rand()%2 ? sbVoice1 : sbVoice2, 0.65, 0.75);
                        S->dying = true;
                        S->hpx = x - S->cx;
                        S->hpy = y - S->cy;
                        S->dieT = type == 1 ? 0.05 : 1.0;
                        GAME_SCORE_TO += (int)(t*10) * (int)(t*10);
                        if (type == 0 && 0==(rand()%8)) {
                            NUM_BOMBS += 1;
                            if (NUM_BOMBS > 99) {
                                NUM_BOMBS = 99;
                            }
                            else {
                                playSound(getBombSfx, 1.0, 0.5);
                            }
                        }
                    }
                    return false;
                }
                if (type == 1) {
                    double dx = x - S->cx, dy = y - S->cy;
                    double lenSq = dx*dx+dy*dy;
                    if (lenSq < 8.*8.) {
                        if (!S->dying) {
                            playSound(sbHurtSfx, 1., 0.75);
                            playSound(rand()%2 ? sbVoice1 : sbVoice2, 0.65, 0.75);
                            S->dying = true;
                            S->hpx = x - S->cx;
                            S->hpy = y - S->cy;
                            S->dieT = type == 1 ? 0.05 : 1.0;
                            GAME_SCORE_TO += (int)(t*10) * (int)(t*10);
                        }
                        return false;
                    }
                }
            }
        }
        spr.setOrigin(8., 8.);
        spr.setPosition(x, y);
        if (type == 0) {
            spr.setRotation(atan2(yv, xv)/PI*180.);
            spr.setColor(Color(0xFFFFFFFFu));
        }
        else if (type == 1) {
            double st = sin(t * PI * 8.) * 0.5 + 0.5;
            Color clr(255, 255, 255, 255);
            clr.g = (clr.g * (int)(255. * st)) / 256;
            clr.b = (clr.b * (int)(255. * st)) / 256;
            spr.setColor(clr);
        }
        AutoTransform(spr);
        window->draw(spr);
        return true;
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
    double PRT_MASS[3] = { 1., 1., 10 };
    double arrowSpeed = 1., bombSpeed = 1.;
    bool lastMouseLeft = false, lastMouseRight = false;
    vector<SnowballDeath> sda;
    vector<Projectile*> proj;
    double arrowCooldown = 0., bombCooldown = 0.;

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
        for (int i=0; i<proj.size(); i++) {
            delete proj[i];
        }
        proj.clear();
        sda.clear();
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
        P->lx = P->x - P->xv; P->ly = P->y - P->yv;
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
        if (rand()%2) {
            playSound(sbVoice1, 1., 0.75);
        }
        else {
            playSound(sbVoice2, 1., 0.75);
        }
        Snowball * S = snowb + id;
        S->dying = false;
        S->eyesSpr = new Sprite(eyesTex);
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

    void render(double dt, double mouseX, double mouseY, bool mouseLeft, bool mouseRight) {
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
                bfr[bpos] = 0xFF0000BB;
            }
            else if (P->type == 2) {
                //bfr[bpos] = 0xDFDFFFFF;
            }
            if (P->clr) {
                bfr[bpos] = P->clr;
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
                                bfr[bpos] = (head->dying || head->cx < 40.) ? 0xCD0000FFu : 0xFFFFDADAu;
                                pfound = true;
                                break;
                            }
                            head = head->next;
                        }
                    }
                }
            }
        }
        if (NUM_BOMBS == 0) {
            mouseRight = false;
        }
        arrowCooldown -= dt;
        bombCooldown -= dt;
        if (((mouseLeft || lastMouseLeft) || (mouseRight || lastMouseRight)) && !castle->dead) {
            double startX, startY;
            double speed = 1.;
            if (mouseLeft || lastMouseLeft) {
                startX = 15.;
                startY = 30.;
                if (mouseLeft) {
                    arrowSpeed += dt * 2.;
                    if (arrowSpeed > 8.) {
                        arrowSpeed = 1.;
                    }
                }
                speed = arrowSpeed;
            }
            else {
                startX = 15.;
                startY = 110.;
                if (mouseRight) {
                    bombSpeed += dt;
                    if (bombSpeed > 5.) {
                        bombSpeed = 1.;
                    }
                }
                speed = bombSpeed;
            }
            startX += 16.; startY += 16.;
            double x = startX, y = startY;
            double dx = mouseX - startX, dy = mouseY - startY;
            double len = sqrt(dx*dx+dy*dy);
            if (len < 1.) {
                len = 1.;
            }
            dx /= len; dy /= len;
            double vx = dx * speed, vy = dy * speed;
            if (!mouseLeft && lastMouseLeft && speed > 1.1 && arrowCooldown <= 0.) {
                Projectile * P = new Projectile(0);
                P->x = x; P->y = y;
                P->xv = vx; P->yv = vy;
                P->t = 0.;
                proj.push_back(P);
                playSound(arrowSfx, 1., 1.);
                arrowCooldown = 1.0;
            }
            else if (!mouseRight && lastMouseRight && speed > 1.1 && bombCooldown <= 0.) {
                Projectile * P = new Projectile(1);
                P->x = x; P->y = y;
                P->xv = vx; P->yv = vy;
                P->t = 0.;
                proj.push_back(P);
                NUM_BOMBS -= 1;
                playSound(arrowSfx, 0.5, 1.);
                bombCooldown = 1.0;
            }
            else {
                const double dampF = exp(-dt * 1./0.95);
                for (int i=0; i<60; i++) {
                    vx *= dampF; vy *= dampF;
                    vy += 0.1 * dt * 16.;
                    x += vx; y += vy;
                    int ix = (int)floor(x), iy = (int)floor(y);
                    if (ix >= 0 && iy >= 0 && ix < width && iy < height) {
                        int bpos = ix + iy * width;
                        bfr[bpos] = speed > 1.1 ? 0xA00000FF : 0xA000008F;
                        if (((terrain->bfr[bpos] >> 24u) & 0xFFu) > 0) {
                            break;
                        }
                    }
                }
            }
        }
        if (!mouseLeft) {
            arrowSpeed = 10.;
        }
        if (!mouseRight) {
            bombSpeed = 10.;
        }
        lastMouseLeft = mouseLeft;
        lastMouseRight = mouseRight;
        renderedTex.update((Uint8*)bfr);
        rendered.setPosition(Vector2f(0., 0.));
        AutoTransform(rendered);
        window->draw(rendered);
        for (int i=0; i<MAX_SNOWBALL; i++) {
            Snowball * S = snowb + i;
            if (S->life > 0.) {
                if (S->cx < 40 || S->dying) {
                    S->eyesSpr->setTexture(eyes2Tex);
                }
                else {
                    S->eyesSpr->setTexture(eyesTex);
                }
                S->eyesSpr->setPosition(Vector2f(S->cx - 8. - S->prt[0]->xv * 10., S->cy - 8. - S->prt[0]->yv * 10.));
                AutoTransform(S->eyesSpr);
                window->draw(*(S->eyesSpr));        
            }
        }
        for (int i=0; i<proj.size(); i++) {
            if (!proj[i]->updateRender(dt, terrain, snowb)) {
                int texr, texrt;
                uint32_t exClr1, exClr2;
                int exCnt;
                double exPwr;
                if (proj[i]->type == 0) {
                    texr = texrt = 1;
                    exClr2 = exClr1 = 0xFFFFDADAu;
                    exCnt = 8;
                    exPwr = 1.;
                    playSound(expSfx, 3.0, 0.45);
                }
                else if (proj[i]->type == 1)  {
                    texr = 18;
                    texrt = 5;
                    exClr1 = 0xFF00FFFFu;
                    exClr2 = 0xFF008FFFu;
                    exCnt = 128;
                    exPwr = 1.;
                    playSound(expSfx, 1.0, 0.75);
                }
                for (int xo=-texr; xo<=texr; xo++) {
                    for (int yo=-texr; yo<=texr; yo++) {
                        int lenSq = xo*xo+yo*yo;
                        if (lenSq <= texr*texr) {
                            int sx = (int)(proj[i]->x) + xo,
                                sy = (int)(proj[i]->y) + yo;
                            if (sx >= 0 && sy >= 0 && sx < width && sy < height) {
                                for (int j=0; j<MAX_SNOWBALL; j++) {
                                    Snowball * S = snowb + j;
                                    if (S->life > 0.) {
                                        if (S->pointInside((double)sx, (double)sy)) {
                                            if (!S->dying) {
                                                S->dying = true;
                                                S->dieT = proj[i]->type == 0 ? 0.5 : 0.05;
                                                GAME_SCORE_TO += (int)(proj[i]->t * 10) * ((int)proj[i]->t*10);
                                                playSound(sbHurtSfx, 1., 0.75);
                                                playSound(rand()%2 ? sbVoice1 : sbVoice2, 0.65, 0.75);
                                            }
                                        }
                                    }
                                }
                                if (lenSq <= texrt*texrt) {
                                    int boff = sx + sy * width;
                                    if (sy >= (height-24)) {
                                        uint32_t tmp = terrain->bfr[boff];
                                        uint8_t r = tmp & 0xFFu;
                                        uint8_t g = (tmp >> 8u) & 0xFFu;
                                        uint8_t b = (tmp >> 16u) & 0xFFu;
                                        r /= 2;
                                        g /= 2;
                                        b /= 2;
                                        terrain->bfr[boff] = (tmp & 0xFF000000u) | (b << 16u) | (g << 8u) | r;
                                    }
                                    else {
                                        terrain->bfr[boff] = 0x00000000u;
                                    }
                                }
                            }
                        }
                    }
                }
                terrain->renderedTex.update((Uint8*)(terrain->bfr));
                for (int j=0; j<exCnt; j++) {
                    Particle P;
                    P.type = 1;
                    P.life = 3. + (double)(rand() % 8);
                    P.life /= 3.;
                    double a = (double)(rand() % 360) / (PI * 2.);
                    double r = (double)(rand() % 290 + 10);
                    P.x = (proj[i]->x + cos(a) * r / 300. * 8.) * exPwr;
                    P.y = (proj[i]->y + sin(a) * r / 300. * 8.) * exPwr;
                    P.clr = rand()%2 ? exClr1 : exClr2;
                    P.xv = cos(a) * r * 0.0000005;
                    P.yv = sin(a) * r * 0.0000005;
                    P.sbidx = -1;
                    addParticle(&P);
                }
                delete proj[i];
                proj.erase(proj.begin() + i);
                i --;
                continue;
            }
        }
    }

    void update(double dt) {
        const int cnt = width*height;
        if ((rand()%100) < 40) {
            Particle P;
            P.life = 15.;
            P.type = 1;
            P.x = (double)(rand() % 3200) / 10.;
            P.y = -(double)(rand() % 200) / 10.;
            P.xv = ((double(rand() % 20)) - 10.) * dt * 0.1;
            P.yv = 0.;
            P.clr = 0xFFFFDADAu;
            P.sbidx = -1;
            addParticle(&P);
        }

        for (int k=0; k<sda.size(); k++) {
            SnowballDeath * S = &(sda[k]);
            S->life -= dt;
            if (S->life < 0.) {
                sda.erase(sda.begin() + k);
                k --;
                continue;
            }
            for (int j=0; j<8; j++) {
                Particle P;
                P.type = 1;
                P.life = 3. + (double)(rand() % 8);
                P.life /= 3.;
                double a = (double)(rand() % 360) / (PI * 2.);
                double r = (double)(rand() % 290 + 10);
                P.x = S->cx + cos(a) * r / 300. * 8.;
                P.y = S->cy + sin(a) * r / 300. * 8.;
                P.xv = cos(a) * r * 0.0000005;
                P.yv = sin(a) * r * 0.0000005;
                P.sbidx = -1;
                addParticle(&P);
            }
        }

        for (int k=0; k<MAX_SNOWBALL; k++) {
            Snowball * S = snowb + k;
            if (S->life > 0. && S->dying) {
                for (int j=0; j<1; j++) {
                    Particle P;
                    P.type = 1;
                    P.life = 3. + (double)(rand() % 8);
                    P.life /= 3.;
                    double a = (double)((rand() % 20) - 10) / (PI * 2.) + atan2(S->hpy, S->hpx);
                    double r = (double)(rand() % 150 + 150);
                    P.x = S->cx + S->hpx;
                    P.y = S->cy + S->hpy;
                    P.xv = cos(a) * r * 0.000005;
                    P.yv = sin(a) * r * 0.000005;
                    P.sbidx = -1;
                    addParticle(&P);
                }
            }
        }

        updateHash();

        int substeps = 8;

        double subdt = 1. / (double)substeps;
        dt *= subdt;

        for (int K=0; K<substeps; K++) {

            double dampf = exp(-dt / 0.5);
            double dampfBlood = exp(-dt / 0.9);

            for (int i=0; i<MAX_SNOWBALL; i++) {
                Snowball * S = snowb + i;
                if (S->life <= 0.) {
                    continue;
                }
                if (S->dying) {
                    S->dieT -= dt;
                }
                S->update(dt);
                if (S->cx < 32. || (S->dying && S->dieT <= 0.)) {
                    S->life = -1.;
                    for (int j=0; j<SB_COUNT; j++) {
                        S->prt[j]->life = -1.;
                    }
                    playSound(expSfx, 1.5, 0.75);
                    if (!S->dying) {
                        castle->damage(10.);
                        playSound(sbHurtSfx, 0.5, 0.75);
                        playSound(rand()%2 ? sbVoice1 : sbVoice2, 1.25, 1.);
                    }
                    SnowballDeath SD(S);
                    sda.push_back(SD);
                    for (int xo=-9; xo<=9; xo++) {
                        for (int yo=-9; yo<=9; yo++) {
                            int lenSq = xo*xo+yo*yo;
                            if (lenSq <= 9*9) {
                                int sx = (int)(S->cx) + xo,
                                    sy = (int)(S->cy + (S->dying ? 0. : 3.)) + yo;
                                if (sx >= 0 && sy >= 0 && sx < width && sy < height) {
                                    int boff = sx + sy * width;
                                    terrain->bfr[boff] = 0x00000000u;
                                }
                            }
                        }
                    }
                    terrain->renderedTex.update((Uint8*)(terrain->bfr));
                    updateHash();
                }
            }

            for (int i=0; i<MAX_PARTICLE; i++) {
                Particle * P = prt + i;
                if (P->life <= 0.) {
                    continue;
                }
                if (P->type == 1) {
                    P->life -= dt;
                }
                P->xv = (P->x - P->lx) * (P->type == 1 ? dampfBlood : dampf);
                P->yv = (P->y - P->ly) * (P->type == 1 ? dampfBlood : dampf);
                P->lx = P->x;
                P->ly = P->y;
                P->yv += 0.1 * dt;
                P->x += P->xv;
                P->y += P->yv;

                if (P->type == 1) { // backvel/vel field
                    double bx = P->x - P->xv,
                           by = P->y - P->yv;
                    double bvx = 0., bvy = 0., bvt = 0.;
                    int hx = (int)floor(bx), hy = (int)floor(by);
                    if (hx < 0 || hy < 0 || hx >= width || hy >= height) {
                        continue;
                    }
                    int hpos = hx + hy * width;
                    Particle * head = hash[hpos];
                    while (head != NULL) {
                        double dx = head->x - P->x,
                               dy = head->y - P->y;
                        double t = 1. - sqrt(dx*dx+dy*dy);
                        if (t > 0.) {
                            bvx += head->xv * t;
                            bvy += head->yv * t;
                            bvt += t;
                        }
                        head = head->next;
                    }
                    if (bvt > 0.) {
                        bvx /= bvt;
                        bvy /= bvt;
                        P->x += bvx * 0.0001;
                        P->y += bvy * 0.0001;
                    }
                }

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
                            if (P->sbidx >= 0) {
                                Snowball * S = snowb + P->sbidx;
                                if (S->cy < P->y) {
                                    S->groundAgo = 0.;
                                }
                            }
                            if (lenSq < 1.) {
                                double len = sqrt(lenSq);
                                if (len < 0.001) {
                                    len = 0.001;
                                }
                                double F = 1.;
                                if (P->sbidx >= 0) {
                                    F = 0.25;
                                }
                                if (P->type == 1) {
                                    F = 0.5;
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

                            if (head->idx != P->sbidx && head->pointInside(P->x, P->y) && head->life >= 0.) {
                                double dx = P->x - head->cx,
                                       dy = P->y - head->cy;
                                double len = sqrt(dx*dx+dy*dy);
                                if (len < 0.001) {
                                    len = 0.001;
                                }
                                dx /= len; dy /= len;
                                double F = 1.;
                                if (P->type == 1) {
                                    F = 0.2;
                                }
                                P->x += dx * (SB_RADIUS * 1.5 - len) * 0.02 * F;
                                P->y += dy * (SB_RADIUS * 1.5 - len) * 0.02 * F;
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
                                                double massA = PRT_MASS[itA->type], massB = PRT_MASS[itB->type];
                                                double tmass = massA + massB;
                                                double F = 1. / len * 0.5 * 0.2 * (1. - len);
                                                itA->x -= dx * F * (massB / tmass) * (itA->type == 1 ? 0.5 : 0.25);
                                                itA->y -= dy * F * (massB / tmass) * (itA->type == 1 ? 0.5 : 0.25);
                                                itB->x += dx * F * (massA / tmass) * (itB->type == 1 ? 0.5 : 0.25);
                                                itB->y += dy * F * (massA / tmass) * (itB->type == 1 ? 0.5 : 0.25);
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

    bool fullscreen = false;

    window = new RenderWindow(VideoMode(800, 600), "Night Of The Livid Snowballs");
    window->setMouseCursorVisible(false);

    terrain = new Terrain();
    Physics physics(terrain);

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
    if (!eyesTex.loadFromFile("sprites/eyes.png")) {
        delete window; delete terrain;
        cerr << "Error loading eyes.png" << endl;
        exit(0);
    }
    if (!eyes2Tex.loadFromFile("sprites/eyes2.png")) {
        delete window; delete terrain;
        cerr << "Error loading eyes2.png" << endl;
        exit(0);
    }
    if (!bombTex.loadFromFile("sprites/bomb.png")) {
        delete window; delete terrain;
        cerr << "Error loading bomb.png" << endl;
        exit(0);
    }
    if (!arrowTex.loadFromFile("sprites/arrow.png")) {
        delete window; delete terrain;
        cerr << "Error loading arrow.png" << endl;
        exit(0);
    }
    if (!cursorTex.loadFromFile("sprites/cursor.png")) {
        delete window; delete terrain;
        cerr << "Error loading cursor.png" << endl;
        exit(0);
    }
    if (!spawnTex.loadFromFile("sprites/spawn.png")) {
        delete window; delete terrain;
        cerr << "Error loading spawn.png" << endl;
        exit(0);
    }
    if (!numbersTex.loadFromFile("sprites/numbers.png")) {
        delete window; delete terrain;
        cerr << "Error loading numbers.png" << endl;
        exit(0);
    }
    if (!defeatTex.loadFromFile("sprites/defeat.png")) {
        delete window; delete terrain;
        cerr << "Error loading defeat.png" << endl;
        exit(0);
    }
    if (!titleTex.loadFromFile("sprites/title.png")) {
        delete window; delete terrain;
        cerr << "Error loading title.png" << endl;
        exit(0);
    }
    if (!music.openFromFile("sfx/music.wav")) {
        delete window; delete terrain;
        cerr << "Error loading music.wav" << endl;
        exit(0);    
    }
    if (!arrowSfx.loadFromFile("sfx/arrow-shoot.wav")) {
        delete window; delete terrain;
        cerr << "Error loading arrow-shoot.wav" << endl;
        exit(0);    
    }
    if (!expSfx.loadFromFile("sfx/exp.wav")) {
        delete window; delete terrain;
        cerr << "Error loading exp.wav" << endl;
        exit(0);    
    }
    if (!getBombSfx.loadFromFile("sfx/get-bomb.wav")) {
        delete window; delete terrain;
        cerr << "Error loading get-bomb.wav" << endl;
        exit(0);    
    }
    if (!sbHurtSfx.loadFromFile("sfx/sb-hurt.wav")) {
        delete window; delete terrain;
        cerr << "Error loading sb-hurt.wav" << endl;
        exit(0);    
    }
    if (!sbJumpSfx.loadFromFile("sfx/sb-jump.wav")) {
        delete window; delete terrain;
        cerr << "Error loading sb-jump.wav" << endl;
        exit(0);    
    }
    if (!sbHurtSfx.loadFromFile("sfx/sb-hurt.wav")) {
        delete window; delete terrain;
        cerr << "Error loading sb-hurt.wav" << endl;
        exit(0);    
    }
    if (!sbVoice1.loadFromFile("sfx/sb-voice.wav")) {
        delete window; delete terrain;
        cerr << "Error loading sb-voice.wav" << endl;
        exit(0);    
    }
    if (!sbVoice2.loadFromFile("sfx/sb-voice-2.wav")) {
        delete window; delete terrain;
        cerr << "Error loading sb-voice-2.wav" << endl;
        exit(0);    
    }

    music.setLoop(true);
    music.setVolume(75.);
    music.play();

    castle = new Castle();

    Sprite bg(bgTex);
    Sprite cursor(cursorTex);
    Sprite spawn(spawnTex);
    Sprite title(titleTex);
    Numbers scoreNumbers(7);
    Numbers bombNumbers(2);

    spawn.setOrigin(16., 16.);

    window->setFramerateLimit(60);    

    double time = 0.;
    double spawnTime = 0.;
    double startTime = 0.;
    bool started = false;
    bool lMouseLeft = false, lMouseRight = false;
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed) {
                music.stop();
                window->close();
            }
            else if (event.type == Event::Resized) {
                VP_WIDTH = window->getSize().x;
                VP_HEIGHT = window->getSize().y;
	            window->setView(View(FloatRect(0.f, 0.f, (float)VP_WIDTH, (float)VP_HEIGHT)));
            }
            else if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Key::F11) {
                    fullscreen = !fullscreen;
                    delete window;
                    window = new RenderWindow(fullscreen ? VideoMode::getDesktopMode() : VideoMode(800, 600), "Night Of The Livid Snowballs", fullscreen ? Style::Fullscreen : Style::Default);
                    window->setFramerateLimit(60);
                    VP_WIDTH = window->getSize().x;
                    VP_HEIGHT = window->getSize().y;
	                window->setView(View(FloatRect(0.f, 0.f, (float)VP_WIDTH, (float)VP_HEIGHT)));
                }
                else if (!started) {
                    started = true;
                    playSound(getBombSfx, 0.5, 0.5);
                }
            }
        }

        if ((!Mouse::isButtonPressed(Mouse::Left) && lMouseLeft) || (!Mouse::isButtonPressed(Mouse::Right) && lMouseRight)) {
            if (!started) {
                started = true;
                playSound(getBombSfx, 0.5, 0.5);
            }
        }
        lMouseLeft = Mouse::isButtonPressed(Mouse::Left);
        lMouseRight = Mouse::isButtonPressed(Mouse::Right);

        Vector2i mousePosI = Mouse::getPosition(*window);
        double mouseX, mouseY;
        InvTransform(mousePosI.x, mousePosI.y, mouseX, mouseY);

        window->clear(Color::Black);

        int shade = (int)(255. * startTime);
        shade /= 16;
        shade *= 16;

        int bgshade = shade;
        if (started) {
            bgshade = 255;
        }

        bg.setColor(Color(bgshade*3/4, bgshade/2, bgshade, 128));
        bg.setPosition(Vector2f(0., 0.));
        AutoTransform(bg);
        window->draw(bg);

        double dt = 1. / 60.;
        if (started) {
            time += dt;
            startTime -= dt;
            if (startTime < 0.) {
                startTime = 0.;
            }

            castle->render(dt);

            spawn.setPosition(275., 75.);
            int rid = (int)(time * 8.) % 4;
            spawn.setTextureRect(IntRect(rid * 32, 0, 32, 32));
            spawnTime -= dt;
            if (spawnTime < 0.) {
                physics.addSnowball(275., 75.);
                spawnTime = (double)(rand()%7 + 1);
            }
            AutoTransform(spawn);
            window->draw(spawn);

            physics.update(dt);
            physics.render(dt, mouseX, mouseY, Mouse::isButtonPressed(Mouse::Left), Mouse::isButtonPressed(Mouse::Right));
            terrain->render(dt);

            if (GAME_SCORE != GAME_SCORE_TO) {
                uint32_t inc = (GAME_SCORE_TO - GAME_SCORE) / 16;
                if (!inc) {
                    inc = (GAME_SCORE_TO - GAME_SCORE);
                }
                GAME_SCORE += inc;
            }

            scoreNumbers.draw(320. - 4. - 8. * 8., 4., GAME_SCORE, Color(255, 255, 127), 1);
            bombNumbers.draw(320. - 4. - 8. * 8. - 5. * 8., 4., NUM_BOMBS, Color(255, 32, 32), 0);
            if (castle->renderHud(dt)) { // death
                terrain->resetLevel();
                physics.clear();
                delete castle;
                castle = new Castle();
                time = 0.;
                started = false;
            }
        }
        else {
            startTime += dt;
            if (startTime > 1.) {
                startTime = 1.;
            }
        }

        title.setColor(Color(shade, shade, shade, shade));
        title.setPosition(Vector2f(0., 0.));
        AutoTransform(title);
        window->draw(title);

        cursor.setPosition(mouseX-8., mouseY-8.);
        AutoTransform(cursor);
        window->draw(cursor);

        window->display();
    }

    delete terrain;
    delete castle;
    delete window;

    return 0;
}