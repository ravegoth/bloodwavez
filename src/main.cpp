// specific c++ includes
#include <iostream>           // pentru operatii de intrare/iesire
#include <cmath>              // pentru functii matematice (sqrt, pow, etc.)
#include <ctime>              // pentru functii legate de timp (time, srand)
#include <cstdlib>            // pentru functii utilitare (rand, srand)
#include <string>             // pentru manipularea stringurilor
#include <vector>             // pentru utilizarea containerului vector
#include <memory>             // pentru pointeri inteligenti si managementul memoriei
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

// folosim spatiile de nume
using namespace std;
using namespace sf;

// -------------------------------------------------------------------- constante --------------------------------------------------------------------
const unsigned int FPS_LIMIT = 60;           // limita de cadre pe secunda

// -------------------------------------------------------------------- functii utilitare --------------------------------------------------------------------
float rand_uniform(float a, float b) {
    // genereaza un numar aleatoriu in intervalul [a, b)
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}

// functie template pentru calcularea distantei euclidiene intre doua puncte
template<typename T>
float distance(Vector2<T> a, Vector2<T> b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// -------------------------------------------------------------------- variabile globale --------------------------------------------------------------------
bool mouseDown = false;           // flag: true daca mouse-ul este apasat
bool leftClick = false;           // flag: true daca butonul stang a fost apasat
bool rightClick = false;          // flag: true daca butonul drept a fost apasat
float mouseX = 0, mouseY = 0;       // coordonatele curente ale mouse-ului
bool keysPressed[256] = { false }; // starea tastelor

unsigned playerX = 400, playerY = 300; // pozitia initiala a jucatorului
unsigned playerSpeed = 5;              // viteza de miscare a jucatorului
Inventory playerInventory;
int pickupRadius = 40; // raza de pickup pentru obiecte

// -------------------------------------------------------------------- clasa Object --------------------------------------------------------------------
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

struct ItemObject {
    Object obj;
    std::shared_ptr<Item> item;
    bool pickedUp = false;

    ItemObject(Object obj, std::shared_ptr<Item> item) : obj(obj), item(item) {}
};

vector<ItemObject> worldItems;

class Tile {
public:
    float x, y;       // coordonatele tile-ului
    float width, height; // dimensiunile tile-ului
    Color color;      // culoarea tile-ului
    // constructor
    Tile(float x, float y, float width, float height, Color color) : x(x), y(y), width(width), height(height), color(color) {}
    
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
    }
};

vector<Object> mapObjects;  // container pentru obiectele din harta
vector<Tile> mapTiles;      // container pentru tile-urile din harta

// -------------------------------------------------------------------- controale --------------------------------------------------------------------
void controls() {
    //todo player sa se miste cu playerX dar camera sa "urmareasca" dupa player
    if (keysPressed[static_cast<int>(Keyboard::Key::W)]) {  // daca tasta W este apasata
        playerY -= playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::S)]) {  // daca tasta S este apasata
        playerY += playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::A)]) {  // daca tasta A este apasata
        playerX -= playerSpeed;
    }
    if (keysPressed[static_cast<int>(Keyboard::Key::D)]) {  // daca tasta D este apasata
        playerX += playerSpeed;
    }
}

// -------------------------------------------------------------------- initializare --------------------------------------------------------------------
void init() {
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


    // 80x60
    for (int i = 0; i < 84/4; i++) {
        for (int j = 0; j < 64/4; j++) {
            mapTiles.push_back(Tile(i * 40 - 40, j * 40 - 40, 39, 39, Color::Black));
        }
    }
}

// -------------------------------------------------------------------- update --------------------------------------------------------------------
void update(RenderWindow& window) {
    // limiteaza pozitia jucatorului in fereastra
    if (playerY < 0) playerY = 0;
    if (playerY > 600) playerY = 600;

    // actualizeaza pozitia obiectelor (efect parallax)
    for (Object& obj : mapObjects) {
        if (keysPressed[static_cast<int>(Keyboard::Key::A)]) {
            obj.x += playerSpeed;
        }
        if (keysPressed[static_cast<int>(Keyboard::Key::D)]) {
            obj.x -= playerSpeed;
        }
    }

    for (auto& obj : worldItems) {
        if(obj.pickedUp)
        {
            continue;
        }
        if (keysPressed[static_cast<int>(Keyboard::Key::A)]) {
            obj.obj.x += playerSpeed;
        }
        if (keysPressed[static_cast<int>(Keyboard::Key::D)]) {
            obj.obj.x -= playerSpeed;
        }
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
        if (keysPressed[static_cast<int>(Keyboard::Key::A)]) {
            tile.x += playerSpeed;
            if (tile.x > 800) tile.x -= 840.0f;
            if (tile.x < -40) tile.x += 840.0f;
        }
        if (keysPressed[static_cast<int>(Keyboard::Key::D)]) {
            tile.x -= playerSpeed;
            if (tile.x > 800) tile.x -= 840.0f;
            if (tile.x < -40) tile.x += 840.0f;
        }
    }

}

// -------------------------------------------------------------------- desenare --------------------------------------------------------------------
void draw(RenderWindow& window) {
    // deseneaza fiecare tile din vectorul mapTiles
    for (Tile& tile : mapTiles) {
        tile.draw(window);
    }

    // desenare jucator: un cerc verde
    CircleShape player(10);
    player.setFillColor(Color::Green);
    player.setPosition(Vector2f(200, playerY)); // pozitia jucatorului
    window.draw(player);

    // desenare sabie: un dreptunghi rosu indiat de mouse
    RectangleShape sword(Vector2f(100, 5));
    sword.setFillColor(Color::Red);
    sword.setPosition(Vector2f(200, playerY)); // pozitia jucatorului
    sword.setOrigin(Vector2f(0, 2.5)); // seteaza originea dreptunghiului la mijlocul lui pe verticala
    
    float angle = atan2(mouseY - playerY, mouseX - 200) * 180 / 3.14159; // calculeaza unghiul in radiani si il converteste in grade

    sword.setRotation(degrees(angle)); // conversie in grade (sf::degrees returnează un sf::Angle)
    window.draw(sword);

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
