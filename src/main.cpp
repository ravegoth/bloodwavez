// specific c++ includes
#include <iostream>           // pentru operatii de intrare/iesire
#include <cmath>              // pentru functii matematice (sqrt, pow, etc.)
#include <ctime>              // pentru functii legate de timp (time, srand)
#include <cstdlib>            // pentru functii utilitare (rand, srand)
#include <string>             // pentru manipularea stringurilor
#include <vector>             // pentru utilizarea containerului vector
#include <memory>             // pentru pointeri inteligenti si managementul memoriei
#include <map>                // pentru utilizarea containerului map
#include <algorithm>          // pentru algoritmi standard (sort, etc.)
#include <filesystem>         // pentru manipularea fisierelor si directoarelor
#include <windows.h>          // pentru api-ul windows (winmain, ascundere fereastra cmd)
#include <tlhelp32.h>         // pentru operatii pe procese si thread-uri in windows

// ------ alte fisiere
#include "Inventory.hpp"

// -------------------------------------------------------------------- sfml --------------------------------------------------------------------
#include <SFML/Graphics.hpp>  // pentru grafica (desenare forme, sprites, etc.)
#include <SFML/Window.hpp>    // pentru manipularea ferestrelor si evenimentelor
#include <SFML/System.hpp>    // pentru functii de sistem (timp, thread-uri, etc.)
#include <SFML/Audio.hpp>     // pentru gestionarea sunetelor si muzicii

#include <optional>
#include <variant>
#include <type_traits>

// -------------------------------------------------------------------- namespace --------------------------------------------------------------------
using namespace std;
using namespace sf;

// -------------------------------------------------------------------- constante --------------------------------------------------------------------
const int FPS_LIMIT = 60;           // limita de cadre pe secunda
const int SPEED_LIMIT = 4;          // 3px/sec
const bool RELEASE = false;         // flag pentru release (true = release, false = debug)

// -------------------------------------------------------------------- functii utilitare --------------------------------------------------------------------
// genereaza un numar aleatoriu in intervalul [a, b)
float rand_uniform(float a, float b) {
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}

// functie template pentru calcularea distantei euclidiene intre doua puncte
template<typename T>
float distance(Vector2<T> a, Vector2<T> b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// -------------------------------------------------------------------- variabile globale legate de joc --------------------------------------------------------------------
bool mouseDown = false;           // flag: true daca mouse-ul este apasat
bool leftClick = false;           // flag: true daca butonul stang a fost apasat
bool rightClick = false;          // flag: true daca butonul drept a fost apasat
float mouseX = 0, mouseY = 0;       // coordonatele curente ale mouse-ului
bool keysPressed[256] = { false }; // starea tastelor

int playerX = 0;
int playerY = 300; // pozitia initiala a jucatorului
bool notMoving = true; // flag pentru miscarea jucatorului
int playerSpeed = 1;              // viteza de miscare a jucatorului
Inventory playerInventory;
string playerHolding = "weapon_basic_sword"; // arma pe care o tine jucatorul init
int pickupRadius = 40; // raza de pickup pentru obiecte

float playerVx = 0; // velocitatea pe axa x a jucatorului
float playerVy = 0; // velocitatea pe axa y a jucatoruluis
int moveAnimationCounter = 0; // contor pentru animatia de miscare a jucatorului

bool dashing = false; // flag pentru dash
bool canDash = true; // flag pentru cooldown-ul dash-ului
float dashSpeed = 8; // viteza de dash
float dashDuration = 30; // durata dash-ului in cadre
int dashCooldown = 60*3; // cooldown-ul dash-ului in frameuri

// -------------------------------------------------------------------- obiecte --------------------------------------------------------------------

// obiect abstract (nu exista scop inca)
class Object {
public:
    float x, y;       // coordonatele obiectului
    float radius;     // raza obiectului (pentru desenare)
    Color color;      // culoarea obiectului
    // constructor
    Object(float x, float y, float radius, Color color) : x(x), y(y), radius(radius), color(color) {}
    
    // metoda pentru desenarea obiectului in fereastra
    void draw(RenderWindow& window) {
        CircleShape circle(radius);                // creeaza un cerc cu raza specificata
        circle.setFillColor(color);                  // seteaza culoarea
        circle.setPosition(Vector2f(x, y));          // seteaza pozitia (folosim vector2f)
        window.draw(circle);                         // deseneaza cercul
    }
};

// pt fi picked up
struct ItemObject {
    Object obj;
    std::shared_ptr<Item> item;
    bool pickedUp = false;

    ItemObject(Object obj, std::shared_ptr<Item> item) : obj(obj), item(item) {}
};

vector<ItemObject> worldItems;

class TextureManager {
private:
    map<std::string, sf::Texture> textures; // mapa pentru a stoca texturile incarcate
    // contructor privat pentru a preveni instantierea directa 
    // singleton pattern
    TextureManager() {}

public:
    // bla bla bla singleton stuff
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // instancea singletons
    static TextureManager& getInstance() {
        static TextureManager instance; // Instanta unica
        return instance;
    }

    Texture& find(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        // verifica daca textura este deja incarcata
        if (textures.find(filename) == textures.end()) {
            Texture texture;
            if (!texture.loadFromFile(filename)) {
                cout << "Failed to load texture: " << filename << endl; 
            }
            textures[filename] = texture; // adauga textura in mapa
        }
        return textures[filename]; // returneaza textura incarcata
    }

    void justLoad(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        // verifica daca textura este deja incarcata
        if (textures.find(filename) == textures.end()) {
            Texture texture;
            if (!texture.loadFromFile(filename)) {
                cout << "Failed to load texture: " << filename << endl; 
            }
            textures[filename] = texture; // adauga textura in mapa
            cout << "Loaded texture: " << filename << endl; // afiseaza mesaj de incarcare
        }
    }

    void clear() {
        textures.clear(); // sterge toate textele incarcate
    }
    // destructor
    ~TextureManager() {
        clear(); // sterge toate textele incarcate
    }
    // metoda pentru a verifica daca textura este incarcata
    bool isLoaded(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        return textures.find(filename) != textures.end(); // verifica daca textura este incarcata
    }
};

class Tile {
private:
    float x, y;       // coordonatele tile-ului
    float width, height; // dimensiunile tile-ului
    Color color;      // culoarea tile-ului
    string type;

public:
    // constructor
    // default tile is dirt bcs idk
    Tile(float x, float y, float width, float height, Color color, string type = "dirt") : x(x), y(y), width(width), height(height), color(color), type(type) {}
    
    // metoda pentru desenarea tile-ului in fereastra
    void draw(RenderWindow& window) {
        RectangleShape rect(Vector2f(width, height)); // creeaza un dreptunghi cu dimensiunile specificate
        rect.setFillColor(color);                      // seteaza culoarea
        rect.setPosition(Vector2f(x, y));              // seteaza pozitia (folosim vector2f)
        // white stroke
        rect.setOutlineColor(Color::White);            // seteaza culoarea conturului
        rect.setOutlineThickness(1);                   // seteaza grosimea conturului
        rect.setPosition(Vector2f(x, y));              // seteaza pozitia (folosim vector2f)
        window.draw(rect);                             // deseneaza dreptunghiul

        // if type == "dirt", draw ./res/dirt.png
        if (type == "dirt") {
            Texture& texture = TextureManager::getInstance().find("dirt");
            Sprite sprite(texture);
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(width / texture.getSize().x, height / texture.getSize().y)); // seteaza scalarea pentru a se potrivi dimensiunilor
            window.draw(sprite); // deseneaza sprite-ul
        }

        // if type == "stone", draw ./res/stone.png
        if (type == "stone") {
            Texture& texture = TextureManager::getInstance().find("stone");
            Sprite sprite(texture);
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(width / texture.getSize().x, height / texture.getSize().y)); // seteaza scalarea pentru a se potrivi dimensiunilor
            window.draw(sprite); // deseneaza sprite-ul
        }
    }

    // setters and getters
    float getX() const { return x; }                // getter pentru x
    float getY() const { return y; }                // getter pentru y
    float getWidth() const { return width; }        // getter pentru width
    float getHeight() const { return height; }      // getter pentru height
    Color getColor() const { return color; }        // getter pentru culoare
    string getType() const { return type; }        // getter pentru tip
    void setX(float x) { this->x = x; }             // setter pentru x
    void setY(float y) { this->y = y; }             // setter pentru y
    void setWidth(float width) { this->width = width; } // setter pentru width
    void setHeight(float height) { this->height = height; } // setter pentru height
    void setColor(Color color) { this->color = color; } // setter pentru culoare
    void setType(string type) { this->type = type; } // setter pentru tip
};

// clasa asta sigur va fi mostenita
class Enemy {

};

// -------------------------------------------------------------------- containere --------------------------------------------------------------------

vector<Object> mapObjects;  // container pentru obiectele din harta
vector<Tile> mapTiles;      // container pentru tile-urile din harta

// -------------------------------------------------------------------- controale --------------------------------------------------------------------
void controls() {
    if (keysPressed[static_cast<int>(Keyboard::Key::W)]) {  // daca tasta W este apasata
        playerVy -= playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::S)]) {  // daca tasta S este apasata
        playerVy += playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::A)]) {  // daca tasta A este apasata
        playerVx += playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::D)]) {  // daca tasta D este apasata
        playerVx -= playerSpeed;
    }

    // dashing
    if (keysPressed[static_cast<int>(Keyboard::Key::Space)]) {  // daca tasta Space este apasata
        if (canDash) {
            dashing = true; // activeaza dash-ul
            playerVx *= dashSpeed; // seteaza viteza de dash
            playerVy *= dashSpeed; // seteaza viteza de dash
        }
    }

    // if (keysPressed[static_cast<int>(Keyboard::Key::Escape)]) {  // daca tasta Escape este apasata
    //     exit(0); // iese din program
    // }

    // cout << "dashing data: " << "dashing: " << dashing << " canDash: " << canDash << " dashDuration: " << dashDuration << " dashCooldown: " << dashCooldown << endl;
}

// -------------------------------------------------------------------- initializare --------------------------------------------------------------------
void init() {
    // map textures
    TextureManager::getInstance().justLoad("dirt");
    TextureManager::getInstance().justLoad("stone");
    // player textures
    TextureManager::getInstance().justLoad("player_still");
    TextureManager::getInstance().justLoad("player_move_1");
    TextureManager::getInstance().justLoad("player_move_2");
    TextureManager::getInstance().justLoad("player_move_1_mirror");
    TextureManager::getInstance().justLoad("player_move_2_mirror");
    TextureManager::getInstance().justLoad("player_dash");
    TextureManager::getInstance().justLoad("player_dash_mirror");
    // weapon textures
    TextureManager::getInstance().justLoad("weapon_basic_sword");

    srand(time(NULL));  // initializeaza generatorul de numere aleatorii
    // adauga obiecte in containerul mapObjects
    mapObjects.push_back(Object(100, 150, 20, Color::Red));
    mapObjects.push_back(Object(200, 210, 30, Color::Blue));
    mapObjects.push_back(Object(300, 350, 20, Color::Green));
    mapObjects.push_back(Object(400, 480, 50, Color::Yellow));
    mapObjects.push_back(Object(500, 300, 30, Color::Magenta));
    mapObjects.push_back(Object(600, 170, 40, Color::Cyan));

    worldItems.push_back(ItemObject(Object(150, 150, 10, Color::Red), std::make_shared<Item>("Sword", ItemType::Weapon)));
    worldItems.push_back(ItemObject(Object(250, 250, 10, Color::Cyan), std::make_shared<Item>("Dark Gloves", ItemType::Equipment)));
    worldItems.push_back(ItemObject(Object(350, 350, 10, Color::Magenta), std::make_shared<Item>("Rare Gloves", ItemType::Equipment)));
    worldItems.push_back(ItemObject(Object(450, 450, 10, Color::Yellow), std::make_shared<Item>("Gold Coin", ItemType::Coin, 10)));

    // init player pos
    playerX = 200;
    playerY = 300;
    playerVx = 0;
    playerVy = 0;

    // 80x60
    for (int i = 0; i < 84/4; i++) {
        for (int j = 0; j < 64/4; j++) {
            mapTiles.push_back(Tile(i * 40 - 40, j * 40 - 40, 39, 39, Color::Black));

            if (rand_uniform(0, 10) > 1) {
                mapTiles.back().setType("dirt");
            } else {
                mapTiles.back().setType("stone");
            }   
        }
    }

}

// -------------------------------------------------------------------- update --------------------------------------------------------------------
void update(RenderWindow& window) {
    // limiteaza pozitia jucatorului in fereastra
    if (playerY < 0) playerY = 0;
    if (playerY > 600) playerY = 600;

    // limiteaza videocitatea jucatorului
    if (!dashing) {
        if (playerVx > SPEED_LIMIT) playerVx = SPEED_LIMIT;
        if (playerVx < -SPEED_LIMIT) playerVx = -SPEED_LIMIT;
        if (playerVy > SPEED_LIMIT) playerVy = SPEED_LIMIT;
        if (playerVy < -SPEED_LIMIT) playerVy = -SPEED_LIMIT;
    } else {
        if (playerVx > dashSpeed) playerVx = dashSpeed;
        if (playerVx < -dashSpeed) playerVx = -dashSpeed;
        if (playerVy > dashSpeed) playerVy = dashSpeed;
        if (playerVy < -dashSpeed) playerVy = -dashSpeed;
    }

    if (abs(playerVx) < 0.1f) playerVx = 0; // opreste miscarea pe axa x daca viteza este foarte mica
    if (abs(playerVy) < 0.1f) playerVy = 0; // opreste miscarea pe axa y daca viteza este foarte mica
    // pt ca la dash sa nu se miste jucatorul in diagonala random
    if (abs(playerVx) == 0 && abs(playerVy) == 0) {
        notMoving = true; // jucatorul nu se misca
    } else {
        notMoving = false; // jucatorul se misca
    }

    // dash mechanics
    if (dashing) {
        playerSpeed += 0.1f; // creste viteza in dash
        dashDuration -= 1; // scade durata dash-ului
        if (dashDuration <= 0) {
            dashing = false; // opreste dash-ul
            canDash = false; // cooldown-ul incepe
            dashDuration = 30; // resetare durata dash-ului
        }
    } else {
        if (!canDash) {
            dashCooldown -= 1; // scade cooldown-ul
            if (dashCooldown <= 0) {
                canDash = true; // dash-ul este disponibil din nou
                dashCooldown = 60*3; // resetare cooldown
            }
        }
    }

    // actualizeaza pozitia jucatorului in functie de viteza si input-ul tastaturii
    playerX += playerVx;
    playerY += playerVy;

    // frictiune movement player
    playerVx *= 0.95f; // reduce viteza pe axa x
    playerVy *= 0.95f; // reduce viteza pe axa y

    // actualizeaza pozitia obiectelor
    for (Object& obj : mapObjects) {
        obj.x += playerVx;
    }

    for (auto& obj : worldItems) {
        if(obj.pickedUp) {
            continue;
        }
        obj.obj.x += playerVx;
    }

    if (keysPressed[static_cast<int>(Keyboard::Key::E)]) {
        for (auto& worldItem : worldItems) {
            if (worldItem.pickedUp) continue;
            float dist = distance(Vector2f(200, playerY), Vector2f(worldItem.obj.x, worldItem.obj.y));
    
            if (dist < pickupRadius) { // pickup radius
                playerInventory.pickUp(worldItem.item);
                worldItem.pickedUp = true;
            }
        }
    }

    // move tiles but their x % 10 so they repeat
    for (Tile& tile : mapTiles) {
        tile.setX(tile.getX() + playerVx);
        if (tile.getX() > 800) tile.setX(tile.getX() - 840.0f);
        if (tile.getX() < -40) tile.setX(tile.getX() + 840.0f);
    }
}

// -------------------------------------------------------------------- desenare --------------------------------------------------------------------
void drawPlayerAt(RenderWindow& window, float x, float y, float speed = 0, float scale = 0.1f) {
    // deseneaza jucatorul in functie de viteza si pozitie
    Texture* texture;
    if (notMoving) {
        texture = &TextureManager::getInstance().find("player_still");
    } else if (playerVx < 0 && moveAnimationCounter % 40 < 20) {
        texture = &TextureManager::getInstance().find("player_move_2");
        moveAnimationCounter ++;
    } else if (playerVx < 0 && moveAnimationCounter % 40 >= 20) {
        texture = &TextureManager::getInstance().find("player_move_1");
        moveAnimationCounter ++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 < 20) {
        texture = &TextureManager::getInstance().find("player_move_2_mirror");
        moveAnimationCounter ++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 >= 20) {
        texture = &TextureManager::getInstance().find("player_move_1_mirror");
        moveAnimationCounter ++;
    } else {
        texture = &TextureManager::getInstance().find("player_still");
    }

    if (dashing && playerVx < 0) {
        texture = &TextureManager::getInstance().find("player_dash");
    } else if (dashing && playerVx > 0) {
        texture = &TextureManager::getInstance().find("player_dash_mirror");
    }

    Sprite sprite(*texture);
    sprite.setOrigin(Vector2f(texture->getSize().x / 2, texture->getSize().y / 2)); // seteaza originea sprite-ului la mijlocul lui
    sprite.setScale(Vector2f(scale, scale)); // seteaza scalarea sprite-ului
    sprite.setPosition(Vector2f(x, y)); // seteaza pozitia sprite-ului (folosim vector2f)
    window.draw(sprite); // deseneaza sprite-ul
}

void drawPlayerWeapon(RenderWindow& window) {
    int handX = 214 - ((playerVx > 0)?30:0); // pozitia mainii pe axa x

    if (playerHolding == "weapon_basic_sword") {
        Texture& texture = TextureManager::getInstance().find("weapon_basic_sword");
        Sprite sprite(texture);
        sprite.setOrigin(Vector2f(texture.getSize().x / 2, texture.getSize().y / 3 * 2)); // seteaza originea sprite-ului la mijlocul X si 1/3 din Y (manerul)
        sprite.setScale(Vector2f(0.11f, 0.11f)); // seteaza scalarea sprite-ului
        sprite.setPosition(Vector2f(handX, playerY)); // seteaza pozitia sprite-ului (folosim vector2f)
        
        // Calculeaza unghiul de rotatie catre mouse
        float angle = atan2(mouseY - playerY, mouseX - (212)) * 180 / 3.14159f + 90; // +90 pentru a alinia sprite-ul
        sprite.setRotation(sf::degrees(angle)); // seteaza rotatia sprite-uluis
        
        window.draw(sprite); // deseneaza sprite-ul

        // draw hitbox-ul 1 (DEBUG) = tip-ul sabiei
        CircleShape circle(5); // creeaza un cerc cu raza 20
        circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 30, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 30)); // seteaza pozitia cercului (folosim vector2f)
        window.draw(circle); // deseneaza cercul
        // draw hitbox-ul 2 (DEBUG) = tip-ul sabiei
        circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 37, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 37)); // seteaza pozitia cercului (folosim vector2f)
        window.draw(circle); // deseneaza cercul
    }
}

void draw(RenderWindow& window) {
    // deseneaza fiecare tile din vectorul mapTiles
    for (Tile& tile : mapTiles) {
        tile.draw(window);
    }

    // weapon
    drawPlayerWeapon(window); // desenare sabie la pozitia (210, playerY)
    // desenare jucator: un cerc verde
    drawPlayerAt(window, 200, playerY); // desenare jucator la pozitia (200, playerY) cu viteza 0

    // deseneaza fiecare obiect din vectorul mapObjects
    for (Object& obj : mapObjects) {
        obj.draw(window);
    }

    // deseneaza fiecare obiect din vectorul worldItems
    for (auto& worldItem : worldItems) {
        if (!worldItem.pickedUp) {
            worldItem.obj.draw(window);
        }
    }
}

// -------------------------------------------------------------------- main --------------------------------------------------------------------
int main() {
    AllocConsole(); // aloca o consola pentru aplicatie PT DEBUG
    freopen("CONOUT$", "w", stdout); // redirectioneaza stdout catre consola alocata
    freopen("CONOUT$", "w", stderr); // redirectioneaza stderr catre consola alocata
    if (RELEASE) ShowWindow(GetConsoleWindow(), SW_HIDE); // ascunde fereastra consolei cand va fi RELEASEs

    // creeaza fereastra jocului cu dimensiuni 800x600 si titlul "project"
    RenderWindow window(VideoMode({800, 600}, 32), "project");
    window.setFramerateLimit(FPS_LIMIT);
    init();

    // in sfml 3.0, pollEvent returneaza un std::optional<sf::Event>
    while (window.isOpen()) {
        while (optional<Event> eventOpt = window.pollEvent()) {
            const Event& event = *eventOpt;
            // folosim noile metode is<T>() și getIf<T>()
            if (event.is<Event::Closed>()) {
                window.close();
            }
            if (event.is<Event::KeyPressed>()) {
                auto keyEv = event.getIf<Event::KeyPressed>();
                if(keyEv)
                    keysPressed[static_cast<int>(keyEv->code)] = true;
            }
            if (event.is<Event::KeyReleased>()) {
                auto keyEv = event.getIf<Event::KeyReleased>();
                if(keyEv)
                    keysPressed[static_cast<int>(keyEv->code)] = false;
            }
            if (event.is<Event::MouseButtonPressed>()) {
                auto mouseEv = event.getIf<Event::MouseButtonPressed>();
                if(mouseEv) {
                    if(mouseEv->button == Mouse::Button::Left)
                        leftClick = true;
                    if(mouseEv->button == Mouse::Button::Right)
                        rightClick = true;
                }
            }
            if (event.is<Event::MouseButtonReleased>()) {
                auto mouseEv = event.getIf<Event::MouseButtonReleased>();
                if(mouseEv) {
                    if(mouseEv->button == Mouse::Button::Left)
                        leftClick = false;
                    if(mouseEv->button == Mouse::Button::Right)
                        rightClick = false;
                }
            }
            if (event.is<Event::MouseMoved>()) {
                auto mouseEv = event.getIf<Event::MouseMoved>();
                if(mouseEv) {
                    mouseX = mouseEv->position.x;
                    mouseY = mouseEv->position.y;
                }
            }
        }

        window.clear();     // sterge continutul ferestrei
        controls();         // proceseaza input-ul jucatorului
        update(window);       // actualizeaza starea jocului
        draw(window);       // deseneaza totul in fereastra
        window.display();   // afiseaza continutul desenat pe ecran
    }
    return 0;  // intoarce 0 la terminarea executiei
}
