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
unsigned int frameCount = 0;

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
// -------------------------------------------------------------------- Views --------------------------------------------------------------------
View skillTree;
View playerView;
// -------------------------------------------------------------------- variabile globale legate de joc --------------------------------------------------------------------
bool mouseDown = false;           // flag: true daca mouse-ul este apasat
bool leftClick = false;           // flag: true daca butonul stang a fost apasat
bool rightClick = false;          // flag: true daca butonul drept a fost apasat
float mouseX = 0, mouseY = 0;       // coordonatele curente ale mouse-ului
bool keysPressed[256] = { false }; // starea tastelor
bool keysReleased[256] = { false };

int playerX = 200;
int playerY = 300; // pozitia initiala a jucatorului
bool notMoving = true; // flag pentru miscarea jucatorului
int playerSpeed = 1;              // viteza de miscare a jucatorului

string playerHolding = "weapon_basic_sword"; // arma pe care o tine jucatorul init
Vector2f swordHitbox1(0, 0); // hitbox-ul armei (sword1)
Vector2f swordHitbox2(0, 0); // hitbox-ul armei (sword2ss)
int pickupRadius = 40; // raza de pickup pentru obiecte

float playerVx = 0; // velocitatea pe axa x a jucatorului
float playerVy = 0; // velocitatea pe axa y a jucatoruluis
int moveAnimationCounter = 0; // contor pentru animatia de miscare a jucatorului

bool dashing = false; // flag pentru dash
bool canDash = true; // flag pentru cooldown-ul dash-ului
float dashSpeed = 8; // viteza de dash
float dashDuration = 30; // durata dash-ului in cadre
int dashCooldown = 60*3; // cooldown-ul dash-ului in frameuri

int balance = 0; // numarul de $$$
int xp = 0; // xp-ul jucatorului
int level = 1; // starting level
int levelXP = 100; // xp-ul necesar pentru a urca la nivelul urmator
int skillPoints = 0;

int playerHealth = 100; // viata jucatorului
int playerMaxHealth = 100; // viata maxima a jucatorului
int playerArmor = 0; // armura jucatorului
int playerMaxArmor = 100; // armura maxima a jucatorului
float playerDamageMultiplier = 1; // muliplicatorul de damage al jucatorului care va fi inmultit cu damage-ul armei
float totalDamageIncrease=0; // totalul de %damage increase al jucatorului

bool skillTreeDown = true; // folosit pt schimbarea intre playerView si skillTreeView

// ---------------------------------------------------- functii folosite de obiecte (forward decl) -----------------------------------------------------------

void spawnCoinAt(float x, float y);
void spawnXPAt(float x, float y);
void playerTakeDamage(int amount);

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

class TextureManager {
private:
    std::map<std::string, sf::Texture> textures; // mapa pentru a stoca texturile incarcate
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

    sf::Texture& find(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        // verifica daca textura este deja incarcata
        if (textures.find(filename) == textures.end()) {
            sf::Texture texture;
            if (!texture.loadFromFile(filename)) {
                std::cout << "Failed to load texture: " << filename << "\n"; 
            }
            textures[filename] = texture; // adauga textura in mapa
        }
        return textures[filename]; // returneaza textura incarcata
    }

    void justLoad(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        // verifica daca textura este deja incarcata
        if (textures.find(filename) == textures.end()) {
            sf::Texture texture;
            if (!texture.loadFromFile(filename)) {
                std::cout << "Failed to load texture: " << filename << "\n"; 
            }
            textures[filename] = texture; // adauga textura in mapa
            std::cout << "Loaded texture: " << filename << "\n"; // afiseaza mesaj de incarcare
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

constexpr int INVENTORY_WIDTH = 600;
constexpr float INVENTORY_HEIGHT = 400;
constexpr int SLOT_SIZE = 64;
constexpr int PADDING = 10;
constexpr int COLUMNS = 6;

enum class ItemType {
    Weapon,
    Equipment,
    Null
};

struct Item {
    std::string name;
    std::string description;
    std::string texturePath;
    ItemType type;
    sf::Sprite sprite;
    sf::Vector2f position;

    Item(std::string name, std::string description, std::string texturePath, ItemType type)
        : name(name), description(description), texturePath(texturePath), type(type), sprite(TextureManager::getInstance().find(texturePath)) {
            sf::Texture texture = TextureManager::getInstance().find(texturePath);
            sprite.setScale(Vector2f(
            SLOT_SIZE / static_cast<float>(texture.getSize().x),
            SLOT_SIZE / static_cast<float>(texture.getSize().y)
        ));
        }
};

struct ItemObject {
    Object obj;
    Item item;
    bool pickedUp = false;

    ItemObject(Object obj, Item item) : obj(obj), item(item) {}
    ItemObject() : obj(Object(350, 350, 10, Color::Magenta)), 
    item(Item("Null", "", "weapon_basic_sword", ItemType::Null)),
    pickedUp(true) {};
};

vector<ItemObject> worldItems;

class Inventory {
private:
    ItemObject firstWeapon;
    ItemObject secondWeapon;
    std::vector<ItemObject> stackables;
    int itemSlots = 16;
    int currentlyOccupied = 0;

public:

    Inventory() {
        stackables.reserve(itemSlots);
    }

    std::vector<ItemObject>& getEquipment()
    {
        return stackables;
    }

    ItemObject getFirstWeapon()
    {
        return firstWeapon;
    }

    ItemObject getSecondWeapon()
    {
        return secondWeapon;
    }
    //todo implementeaza inventar vizual
    bool pickUp(ItemObject itemobj) {
        std::cout << stackables.size() << "\n";

        switch (itemobj.item.type) {
            case ItemType::Null:
                return false;

            case ItemType::Weapon:
                // Assign to first empty weapon slot
                if (firstWeapon.item.type == ItemType::Null) {
                    firstWeapon = itemobj;
                    std::cout << "Weapon assigned to slot 1\n";
                    return true;
                } else if (secondWeapon.item.type == ItemType::Null) {
                    secondWeapon = itemobj;
                    std::cout << "Weapon assigned to slot 2\n";
                    return true;
                } else {
                    std::cout << "Both weapon slots are full\n";
                    return false; // poate faci un slot de backup
                }

            case ItemType::Equipment:
                if (currentlyOccupied >= itemSlots) return false;

                currentlyOccupied++;
                stackables.push_back(itemobj);
                return true;
        }

        return false;
    }

    void RemoveEquipment(int index)
    {
        if(index >= 0 && index < static_cast<int>(stackables.size())) {
            ItemObject removedItem = stackables[index];

            removedItem.pickedUp = false;
            removedItem.obj.x = rand_uniform(190, 210);
            removedItem.obj.y = rand_uniform(playerY - 10, playerY + 10);

            worldItems.push_back(removedItem);
            
            stackables.erase(stackables.begin() + index);
            currentlyOccupied--;
        }
    }

    void dropWeapon(int slot) {
        ItemObject dropped;

        if (slot == 1 && firstWeapon.item.type != ItemType::Null) {
            dropped = firstWeapon;
            dropped.pickedUp = false;
            dropped.obj.x = rand_uniform(190, 210);
            dropped.obj.y = rand_uniform(playerY - 10, playerY + 10);
            firstWeapon = ItemObject(); // reset to null
        }
        else if (slot == 2 && secondWeapon.item.type != ItemType::Null) {
            dropped = secondWeapon;
            dropped.pickedUp = false;
            dropped.obj.x = rand_uniform(190, 210);
            dropped.obj.y = rand_uniform(playerY - 10, playerY + 10);
            secondWeapon = ItemObject(); // reset to null
        }
        worldItems.push_back(dropped);
    }



};

Inventory playerInventory;

class InventoryWindow {
private:
    sf::RectangleShape background;
    sf::Text titleText;
    std::vector<sf::RectangleShape> weaponSlots;
    sf::Font font;
    sf::Text tooltipText;
    sf::RectangleShape tooltipBackground;
    int hoveredIndex = -1;
    bool removedItemOnClick = false;

public:
    bool isVisible = false;

    void updateBackgroundSize() {
        std::vector<ItemObject>& equipment = playerInventory.getEquipment();
        int rows = static_cast<int>(std::ceil(equipment.size() / static_cast<float>(COLUMNS)));
        int extraHeight = rows * (SLOT_SIZE + PADDING);
        int dynamicHeight = 50 + extraHeight + PADDING;

        background.setSize(sf::Vector2f(INVENTORY_WIDTH, max(50.f, static_cast<float>(dynamicHeight))));
    }

    InventoryWindow(sf::Font& font): titleText(font, "", 27), tooltipText(font, "", 27){
        
        this->font = font;

        background.setSize(sf::Vector2f(INVENTORY_WIDTH, INVENTORY_HEIGHT));
        background.setFillColor(sf::Color(30, 30, 30, 220));
        background.setOutlineThickness(2);
        background.setOutlineColor(sf::Color::White);

        // Initialize weapon slots
        for(int i = 0; i < 2; i++) {
            sf::RectangleShape slot(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
            slot.setPosition(Vector2f(PADDING + i * (SLOT_SIZE + PADDING), 520));
            slot.setFillColor(sf::Color(80, 80, 80));
            slot.setOutlineThickness(1);
            slot.setOutlineColor(sf::Color::White);
            weaponSlots.push_back(slot);
        }

        sf::Text titleText(font, "", 27);
        titleText.setFont(font);
        titleText.setString("Inventory");
        titleText.setCharacterSize(24);
        titleText.setPosition(Vector2f(PADDING, PADDING));

        tooltipBackground.setFillColor(sf::Color(0, 0, 0, 200));
        tooltipText.setFont(font);
        tooltipText.setCharacterSize(16);

        updateBackgroundSize();
    }

    void update(sf::RenderWindow& window, Inventory& inventory) {

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        hoveredIndex = -100;
        
        std::vector<ItemObject>& equipment = playerInventory.getEquipment();
        
        // Check equipment hover
        for(size_t i = 0; i < equipment.size(); i++) {
            if(equipment[i].item.sprite.getGlobalBounds().contains(mousePos)) {
                hoveredIndex = static_cast<int>(i);
                break;
            }
        }

        if(weaponSlots[0].getGlobalBounds().contains(mousePos)) {
            hoveredIndex = static_cast<int>(-1);
        }

        if(weaponSlots[1].getGlobalBounds().contains(mousePos)) {
            hoveredIndex = static_cast<int>(-2);
        }

        // Handle click to drop
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if(hoveredIndex != -100 && removedItemOnClick == false) {
                //inventory.removeItem(hoveredIndex);
                onRemoveItem(hoveredIndex);
                removedItemOnClick = true; // avoid removing multiple items in 1 click
            }
        }
        else
        {
            removedItemOnClick = false;
        }

        updateTooltip(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        std:cout<<"test1";
       if(!isVisible){

            for(auto& slot : weaponSlots) window.draw(slot);

            if (playerInventory.getFirstWeapon().item.type != ItemType::Null) {
                Sprite s = playerInventory.getFirstWeapon().item.sprite;
                s.setPosition(weaponSlots[0].getPosition());
                window.draw(s);
            }

            if (playerInventory.getSecondWeapon().item.type != ItemType::Null) {
                Sprite s = playerInventory.getSecondWeapon().item.sprite;
                s.setPosition(weaponSlots[1].getPosition());
                window.draw(s);
            }

            if(hoveredIndex != -100) {
                window.draw(tooltipBackground);
                window.draw(tooltipText);
            }
            return;
       }

        // Center window
        sf::Vector2f center = window.getView().getCenter();
        background.setPosition(Vector2f(center.x - INVENTORY_WIDTH/2, center.y - INVENTORY_HEIGHT/2));

        positionEquipmentSlots();

        window.draw(background);
        window.draw(titleText);

        for(auto& slot : weaponSlots) window.draw(slot);

        if (playerInventory.getFirstWeapon().item.type != ItemType::Null) {
            Sprite s = playerInventory.getFirstWeapon().item.sprite;
            s.setPosition(weaponSlots[0].getPosition());
            window.draw(s);
        }

        if (playerInventory.getSecondWeapon().item.type != ItemType::Null) {
            Sprite s = playerInventory.getSecondWeapon().item.sprite;
            s.setPosition(weaponSlots[1].getPosition());
            window.draw(s);
        }
        
        std::vector<ItemObject>& equipment = playerInventory.getEquipment();
        // Draw equipment items
        for(auto& itemobj : equipment) {
            window.draw(itemobj.item.sprite);
        }

        // Draw tooltip
        if(hoveredIndex != -100) {
            window.draw(tooltipBackground);
            window.draw(tooltipText);
        }
    }

    bool onAddItem(ItemObject item) {
        if(playerInventory.pickUp(item) == false) return false;
        positionEquipmentSlots();
        updateBackgroundSize();
        return true;
    }

    void positionEquipmentSlots() {
        sf::Vector2f bgPos = background.getPosition();
        const float startX = bgPos.x + PADDING;
        const float startY = bgPos.y + 50;

        std::vector<ItemObject>& equipment = playerInventory.getEquipment();

        for(size_t i = 0; i < equipment.size(); i++) {
            //std::cout<<equipment[i].item.name + "\n";
            int col = i % COLUMNS;
            int row = i / COLUMNS;
            sf::Vector2f pos(
                startX + col * (SLOT_SIZE + PADDING),
                startY + row * (SLOT_SIZE + PADDING)
            );
            equipment[i].item.position = pos;
            equipment[i].item.sprite.setPosition(pos);
        }
    }

    void updateTooltip(sf::Vector2f mousePos) {
        if(hoveredIndex == -100) return;

        if(hoveredIndex == -1) {
            const auto& item = playerInventory.getFirstWeapon().item;
            tooltipText.setString(item.name + "\n" + item.description);
            
            sf::FloatRect bounds = tooltipText.getLocalBounds();
            tooltipBackground.setSize(sf::Vector2f(bounds.size.x + 20, bounds.size.y + 20));
            
            tooltipBackground.setPosition(Vector2f(mousePos.x + 20, mousePos.y + 20));
            tooltipText.setPosition(Vector2f(mousePos.x + 25, mousePos.y + 25));
            return;
        }

        if(hoveredIndex == -2) {
            const auto& item = playerInventory.getSecondWeapon().item;
            tooltipText.setString(item.name + "\n" + item.description);
            
            sf::FloatRect bounds = tooltipText.getLocalBounds();
            tooltipBackground.setSize(sf::Vector2f(bounds.size.x + 20, bounds.size.y + 20));
            
            tooltipBackground.setPosition(Vector2f(mousePos.x + 20, mousePos.y + 20));
            tooltipText.setPosition(Vector2f(mousePos.x + 25, mousePos.y + 25));
            return;
        }

        if(isVisible == false) return;

        std::vector<ItemObject>& equipment = playerInventory.getEquipment();

        const auto& item = equipment[hoveredIndex].item;
        tooltipText.setString(item.name + "\n" + item.description);
        
        sf::FloatRect bounds = tooltipText.getLocalBounds();
        tooltipBackground.setSize(sf::Vector2f(bounds.size.x + 20, bounds.size.y + 20));
        
        tooltipBackground.setPosition(Vector2f(mousePos.x + 20, mousePos.y + 20));
        tooltipText.setPosition(Vector2f(mousePos.x + 25, mousePos.y + 25));
    }

    void updateAlways(sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        hoveredIndex = -100;

        // Check weapon slots hover
        if (playerInventory.getFirstWeapon().item.type != ItemType::Null &&
            playerInventory.getFirstWeapon().item.sprite.getGlobalBounds().contains(mousePos)) {
            hoveredIndex = -1;
        }

        if (playerInventory.getSecondWeapon().item.type != ItemType::Null &&
            playerInventory.getSecondWeapon().item.sprite.getGlobalBounds().contains(mousePos)) {
            hoveredIndex = -2;
        }

        // Handle drop
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (hoveredIndex == -1 && !removedItemOnClick) {
                //dropWeapon(1);
                removedItemOnClick = true;
            }
            else if (hoveredIndex == -2 && !removedItemOnClick) {
                //dropWeapon(2);
                removedItemOnClick = true;
            }
        } else {
            removedItemOnClick = false;
        }

        if (hoveredIndex == -1 || hoveredIndex == -2)
            updateTooltip(mousePos);
    }

    void onRemoveItem(int index) {
        if(index == -1){
            playerInventory.dropWeapon(1);
        }
        if(index == -2){
            playerInventory.dropWeapon(2);
        }
        std::vector<ItemObject> equipment = playerInventory.getEquipment();
        if(index >= 0 && index < static_cast<int>(equipment.size())) {
            playerInventory.RemoveEquipment(index);
            positionEquipmentSlots();
            updateBackgroundSize();
        }
    }
};

sf::Font uiFont;
int nearbyItemIndex;
InventoryWindow inventoryWindow(uiFont);
bool inventoryVisible = false;
bool openedInventoryThisPress = false;
bool pressedE = false;

class SoundManager {
private:
    map<string, SoundBuffer> soundBuffers; // mapa pentru a stoca buffer-ele de sunet incarcate
    map<string, std::shared_ptr<Sound>> sounds; // mapa pentru a stoca pointerii la sunete

    // constructor privat pentru a preveni instantierea directa
    SoundManager() {}

public:
    // singleton pattern
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    static SoundManager& getInstance() {
        static SoundManager instance; // instanta unica
        return instance;
    }

    // metoda pentru a incarca un sunet
    void loadSound(const string& name, const string& filename) {
        string full_filename = "./res/" + filename + ".wav"; // adauga calea catre fisier
        if (soundBuffers.find(name) == soundBuffers.end()) {
            SoundBuffer buffer;
            if (!buffer.loadFromFile(full_filename)) { // incarca fisierul
                cout << "Failed to load sound: " << filename << endl;
                return;
            }
            soundBuffers[name] = buffer; // adauga buffer-ul in mapa
            sounds[name] = std::make_shared<sf::Sound>(soundBuffers[name]); // asociaza buffer-ul cu un pointer la sunet
            cout << "Loaded sound: " << filename << endl;

            // do not play sound when loaded
            sounds[name]->setVolume(0); // seteaza volumul la 0 pentru a nu reda sunetul la incarcare
        }
    }

    // metoda pentru a reda un sunet
    void playSound(const string& name, int volume = 30) {
        if (sounds.find(name) != sounds.end()) {
            // seteaza volumul la 100% (sau orice altceva vrei)
            sounds[name]->setVolume(volume); // seteaza volumul
            sounds[name]->play();
        } else {
            cout << "Sound not found for playing: " << name << endl;

            cout << "Trying to load sound: " << name << endl;

            loadSound(name, name); // incarca sunetul daca nu este gasit
        }
    }

    // metoda pentru a opri un sunet
    void stopSound(const string& name) {
        if (sounds.find(name) != sounds.end()) {
            sounds[name]->stop();
        } else {
            cout << "Sound not found for stopping: " << name << endl;
        }
    }

    // metoda pentru a verifica daca un sunet este incarcat
    bool isLoaded(const string& name) {
        return soundBuffers.find(name) != soundBuffers.end();
    }

    // destructor
    ~SoundManager() {
        sounds.clear();
        soundBuffers.clear();
    }
};

// TODO
// class bgmManager {
// private:
//     Sound bgm; // obiectul pentru sunetul de fundal
//     bool playing = false; // flag pentru a verifica daca muzica este redat
//     map<string, SoundBuffer> soundBuffers; // mapa pentru a stoca buffer-ele de sunet incarcate
//     string currentTrack; // numele melodiei curente
//     // constructor privat pentru a preveni instantierea directa
//     bgmManager() {
//         SoundBuffer buffer;
//         if (!buffer.loadFromFile("./res/default_bgm.wav")) { // Provide a default sound file
//             cout << "Failed to load default BGM" << endl;
//         } else {
//             bgm.setBuffer(buffer); // Initialize bgm with the default buffer
//             soundBuffers["default_bgm"] = buffer; // Store the buffer for reuse
//         }
//     }

// public:
//     // singleton pattern
//     bgmManager(const bgmManager&) = delete;
//     bgmManager& operator=(const bgmManager&) = delete;

//     static bgmManager& getInstance() {
//         static bgmManager instance; // instanta unica
//         return instance;
//     }

//     // metoda pentru a incarca un sunet
//     void loadBGM(const string& name, const string& filename) {
//         string full_filename = "./res/" + filename + ".wav"; // adauga calea catre fisier
//         if (soundBuffers.find(name) == soundBuffers.end()) {
//             SoundBuffer buffer;
//             if (!buffer.loadFromFile(full_filename)) { // incarca fisierul
//                 cout << "Failed to load BGM: " << filename << endl;
//                 return;
//             }
//             soundBuffers[name] = buffer; // adauga buffer-ul in mapa
//             cout << "Loaded BGM: " << filename << endl;
//         }
//     }

//     // metoda pentru a reda un sunet
//     void playBGM(const string& name, int volume = 50) {
//         if (currentTrack == name && playing) return; // daca melodia curenta este deja redata, nu face nimic

//         if (soundBuffers.find(name) != soundBuffers.end()) {
//             bgm.setBuffer(soundBuffers[name]); // seteaza buffer-ul pentru sunet
//             bgm.setLoop(true); // seteaza redarea in bucla
//             bgm.setVolume(volume); // seteaza volumul
//             bgm.play(); // reda melodia
//             currentTrack = name; // actualizeaza melodia curenta
//             playing = true; // seteaza flag-ul de redare
//         } else {
//             cout << "BGM not found: " << name << endl;
//         }
//     }

//     // metoda pentru a opri muzica
//     void stopBGM() {
//         bgm.stop();
//         playing = false;
//         currentTrack = "";
//     }

//     // destructor
//     ~bgmManager() {
//         stopBGM();
//         soundBuffers.clear();
//     }
// };

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
        // RectangleShape rect(Vector2f(width, height)); // creeaza un dreptunghi cu dimensiunile specificate
        // rect.setFillColor(color);                      // seteaza culoarea
        // rect.setPosition(Vector2f(x, y));              // seteaza pozitia (folosim vector2f)
        // // white stroke
        // rect.setOutlineColor(Color::White);            // seteaza culoarea conturului
        // rect.setOutlineThickness(1);                   // seteaza grosimea conturului
        // rect.setPosition(Vector2f(x, y));              // seteaza pozitia (folosim vector2f)
        // window.draw(rect);                             // deseneaza dreptunghiul

        // if type == "dirt", draw ./res/dirt.png
        if (type == "dirt") {
            Texture& texture = TextureManager::getInstance().find("dirt");
            Sprite sprite(texture);
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(40.0f / texture.getSize().x, 40.0f / texture.getSize().y));
            window.draw(sprite); // deseneaza sprite-ul
        }

        // if type == "stone", draw ./res/stone.png
        if (type == "stone") {
            Texture& texture = TextureManager::getInstance().find("stone");
            Sprite sprite(texture);
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(40.0f / texture.getSize().x, 40.0f / texture.getSize().y));
            window.draw(sprite); // deseneaza sprite-ul
        }

        // factorul de scaling folosit este 1.25 deoarece stim ca fiecare tile esete 32/32 dar noi
        // avem celule de 40/40 in matrice deci 32*1.25=40

        // if type == "grass1", draw ./res/grass1.png
        if (type == "grass1") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture,IntRect({0,0},{32,32}));
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite); // deseneaza sprite-ul
        }

        // if type == "grass2", draw ./res/grass2.png
        if (type == "grass2") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture,IntRect({32,0},{32,32}));
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite); // deseneaza sprite-ul
        }

        // if type == "grass3", draw ./res/grass3.png
        if (type == "grass3") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture,IntRect({0,32},{32,32}));
            sprite.setPosition(Vector2f(x, y)); // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
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
protected:
    int health; // viata inamicului
    int maxHealth; // viata maxima a inamicului
    int damage; // damage-ul inamicului
    bool isMelee;
    float x, y;
    float vx, vy; // viteza pe axa x si y
    int animation; // animatia inamicului (de la 1 la 100)
    bool toBeDeleted = false; // DEAD
    bool isAttacking = false; // flag pentru atac (va schima animatia, si daca e aproape de player si melee va da damage)
    bool canAttack = true; // flag pentru cooldown-ul atacului
    int attackCooldown = 60; // current cooldown in frameuri
    int attackCooldownStart = 60; // attackCooldown dupa atac = attackCooldownStart
    int xpOnDrop = 0; // xp-ul pe care il drop-eaza inamicul
    int coinsOnDrop = 0; // banii pe care ii drop-eaza inamicul
    float speed = 0.5f; // viteza de miscare a inamicului
    float maxSpeed = 2.5f; // viteza maxima a inamicului
    float enemyHeight = 50; // dimensiunile inamicului (pentru hitbox)
    float enemyWidth = 30; // dimensiunile inamicului (pentru hitbox)
    float attackRadius = 40; // raza de atac a inamicului (pentru hitbox)
    int isAttackingAnimation = 0; // animatia de atac (30 frameuri)
public:
// Constructor
    Enemy(float x, float y, int health, int damage, bool isMelee) {
        this->x = x; // seteaza pozitia pe axa x
        this->y = y; // seteaza pozitia pe axa y
        this->health = health; // seteaza viata inamicului
        this->damage = damage; // seteaza damage-ul inamicului
        this->isMelee = isMelee; // seteaza daca inamicul este melee sau ranged
        vx = 0; // initializare viteza pe axa x
        vy = 0; // initializare viteza pe axa y
        animation = 0; // initializare animatie
        toBeDeleted = false; // initializare DEAD
        isAttacking = false; // initializare atac
        canAttack = true; // initializare cooldown atac
        attackCooldown = 60; // initializare cooldown atac
        attackCooldownStart = 60; // initializare cooldown atac
        xpOnDrop = 0; // initializare xp drop
        coinsOnDrop = 0; // initializare bani drop
        speed = 0.5f; // initializare viteza de miscare
        maxSpeed = 2.5f; // initializare viteza maxima
        enemyHeight = 50; // initializare inaltime inamic
        enemyWidth = 30; // initializare latime inamic
        attackRadius = 40; // initializare raza de atac
        isAttackingAnimation = 0; // initializare animatie de atac
    }

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getDamage() const { return damage; }
    bool isMeleeEnemy() const { return isMelee; }
    bool isToBeDeleted() const { return toBeDeleted; }
    bool getIsAttacking() const { return isAttacking; }

    // Setters
    void setX(float x) { this->x = x; }
    void setY(float y) { this->y = y; }
    void setHealth(int health) { this->health = health; }
    void setToBeDeleted(bool deleted) { this->toBeDeleted = deleted; }

    // Methods
    void takeDamage(float amount) {
        if (toBeDeleted) return; // Already dead

        health -= amount;
        // hit_enemy sound effect:
        SoundManager::getInstance().playSound("hit_enemy"); // Play hit sound

        if (health <= 0) {
            health = 0; // Ensure health doesn't go negative
            toBeDeleted = true;

            // Drop XP and coins
            for (int i = 0; i < coinsOnDrop; ++i) {
                // Spawn coin at enemy's position
                spawnCoinAt(x, y); // Function to spawn coin at enemy's position
            }
            for (int i = 0; i < xpOnDrop; ++i) {
                // Spawn XP item at enemy's position
                spawnXPAt(x, y); // Function to spawn XP item at enemy's position
            }
            
            // death sound effect:
            SoundManager::getInstance().playSound("dead_enemy"); // Play death sound
        }
    }

    virtual void update(RenderWindow& window) {
        if (toBeDeleted) return;
    }

    virtual void draw(RenderWindow& window) {   };
};

class EnemyGoblin : public Enemy {
public:
    EnemyGoblin(float x, float y, int health, int damage, bool isMelee)
        : Enemy(x, y, health, damage, isMelee) {
        xpOnDrop = 10 + rand() % 10; // XP dropped when killed
        coinsOnDrop = 7; // Coins dropped when killed
        damage = 5; // Damage dealt to player
        maxHealth = 20 + rand() % 10; // Health of the goblin
        health = maxHealth; // Set current health to max health
        attackCooldown = 30; // Cooldown for attack in frames
        attackCooldownStart = 30; // Cooldown for attack in frames
        isMelee = true; // Goblin is a melee enemy
        vx = 0; // Initial velocity on x-axis
        vy = 0; // Initial velocity on y-axis
        animation = 0; // Initial animation frame
        toBeDeleted = false; // Goblin is not dead
        isAttacking = false; // Goblin is not attacking
        isAttackingAnimation = 0; // Keep that sprite for 30 frames
        canAttack = true; // Goblin can attack
        speed = 0.5f;
        maxSpeed = 2.5f; // Maximum speed of the goblin
        this->x = x; // Set goblin's x position
        this->y = y; // Set goblin's y position
        this->vx = 0; // Set goblin's x velocity
        this->vy = 0; // Set goblin's y velocity
        this->enemyHeight = 50; // Height of the goblin
        this->enemyWidth = 30; // Width of the goblin
        attackRadius = 40;
    }

    void draw(RenderWindow& window) override {
        // textures:
        // walk 1 (animation % 50 < 25) = enemy_goblin_walk1.png
        // walk 2 (animation % 50 >= 25) = enemy_goblin_walk2.png
        // attack (when isAttacking) = enemy_goblin_attack.png
        // if he's in the right of the player:
        // walk 1 mirrored (animation % 50 < 25) = enemy_goblin_walk1_mirror.png
        // walk 2 mirrored (animation % 50 >= 25) = enemy_goblin_walk2_mirror.png
        // attack mirrored (when isAttacking) = enemy_goblin_attack_mirror.png

        //individual size 82/128

        Texture& mobTexture = TextureManager::getInstance().find("Goblinset");

        Sprite sprite(mobTexture,IntRect({82*2,82},{82,128})); // Initialize with a valid texture

        // draw (centered) at x, y
        sprite.setPosition(Vector2f(82/2, 128/2)); // center the sprite
        // auto scale the sprite to 30x50 px
        sprite.setScale(Vector2f(0.365, 0.390)); // scale the sprite
        
        // draw
        if (isAttackingAnimation > 0) {
            // draw attack animation
            if (getX() < 200) {
                sprite.setTextureRect(IntRect({0,0},{82,128})); // set texture to attack
            } else {
                sprite.setTextureRect(IntRect({82,0},{82,128}));  // set texture to attack mirrored
            }
        } else {
            // draw walk animation
            if (animation % 50 < 25) {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82*2,0},{82,128})); // set texture to walk1
                } else {
                    sprite.setTextureRect(IntRect({0,128},{82,128})); // set texture to walk1 mirrored
                }
            } else {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82,128},{82,128})); // set texture to walk2
                } else {
                    sprite.setTextureRect(IntRect({82*2,128},{82,128})); // set texture to walk2 mirrored
                }
            }
        }

        // set the scale to 30x50 px (can increase for bigger enemies)
        // scale set to equal 30x50 from 82x128
        sprite.setScale(Vector2f(0.365, 0.390)); // scale the sprite
        // set the position to x, y
        sprite.setPosition(Vector2f(getX() - 30.0f / 2, getY() - 50.0f / 2)); // center the sprite
        // draw the sprite  
        window.draw(sprite); // draw the sprite
        // update animation
        animation = (animation % 100) + 1; // update animation
    }

    // update
    void update(RenderWindow& window) override {
        if (toBeDeleted) return; // Already dead

        // update animation
        animation = (animation % 100) + 1; // update animation
        if (isAttackingAnimation > 0) isAttackingAnimation--; // update attack animation

        // update cooldown
        if (attackCooldown > 0) {
            attackCooldown--;
        } else {
            canAttack = true;
        }

        // move towards player
        if (getX() < 200) {
            vx += speed; // move right
        } else {
            vx += -speed; // move left
        }
        if (getY() < playerY) {
            vy += speed; // move down
        } else {
            vy += -speed; // move up
        }

        // limit
        if (vx > maxSpeed) vx = maxSpeed; // limit speed
        if (vx < -maxSpeed) vx = -maxSpeed; // limit speed
        if (vy > maxSpeed) vy = maxSpeed; // limit speed
        if (vy < -maxSpeed) vy = -maxSpeed; // limit speed

        // check distance to player
        float distToPlayer = distance(Vector2f(getX(), getY()), Vector2f(200, playerY));
        float angleToPlayer = atan2(playerY - getY(), 200 - getX()); // calculate angle to player
        Angle angle(radians(angleToPlayer)); // convert to Angle object

        // // debug: desenaza o linie de la inamic la player
        // sf::Vertex line[] = {
        //     {{getX(), getY()}, sf::Color::Red},
        //     {{200.f, static_cast<float>(playerY)}, sf::Color::Red}
        // };

        // // debug: draw 30x50 hitbox
        // sf::Vertex hitbox[] = {
        //     {{getX() - 30.0f / 2, getY() - 50.0f / 2}, sf::Color::Red},
        //     {{getX() + 30.0f / 2, getY() - 50.0f / 2}, sf::Color::Red},
        //     {{getX() + 30.0f / 2, getY() + 50.0f / 2}, sf::Color::Red},
        //     {{getX() - 30.0f / 2, getY() + 50.0f / 2}, sf::Color::Red}
        // };
        // // draw hitbox
        // window.draw(hitbox, 4, sf::PrimitiveType::LineStrip); // draw the hitbox
        
        // line[0].color = sf::Color::Red; // set color to red
        // line[1].color = sf::Color::Red; // set color to red
        // window.draw(line, 2, sf::PrimitiveType::Lines); // draw the line

        // if swordhitbox1 or swordhitbox2 is inside the enemy hitbox, take damage
        // and knockback (velocity 7 * cos/sin) in the opposite direction of the player
        // not using distance, but using hitbox collision
        if (swordHitbox1.x > getX() - enemyWidth / 2 && swordHitbox1.x < getX() + enemyWidth / 2 &&
            swordHitbox1.y > getY() - enemyHeight / 2 && swordHitbox1.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * damage); // take damage from player
            vx += -7 * cos(angleToPlayer); // knockback in the opposite direction of the player
            vy += -7 * sin(angleToPlayer); // knockback in the opposite direction of the player
        }
        if (swordHitbox2.x > getX() - enemyWidth / 2 && swordHitbox2.x < getX() + enemyWidth / 2 &&
            swordHitbox2.y > getY() - enemyHeight / 2 && swordHitbox2.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * damage); // take damage from player
            vx += -7 * cos(angleToPlayer); // knockback in the opposite direction of the player
            vy += -7 * sin(angleToPlayer); // knockback in the opposite direction of the player
        }

        if (distToPlayer < attackRadius && canAttack) { // if close enough to attack
            isAttacking = true;
            canAttack = false;
            attackCooldown = attackCooldownStart; // reset cooldown
            if (isMelee) {
                playerTakeDamage(damage); // deal damage to player
                // knockback player
                playerVx += -1.5 * cos(angleToPlayer); // knockback in the opposite direction of the player
                playerVy += -1.5 * sin(angleToPlayer); // knockback in the opposite direction of the player
                isAttackingAnimation = 30; // set attack animation
                SoundManager::getInstance().playSound("took_damage"); // Play hit sound for player
            }
        } else {
            isAttacking = false;
        }

        // apply velocity to position
        x += vx * 0.8f; // apply velocity to x position
        y += vy * 0.8f; // apply velocity to y position

        x += playerVx; // apply player velocity to x position
        // y += playerVy; // apply player velocity to y position
    }
};

class EnemyBaphomet : public Enemy {
public:
    EnemyBaphomet(float x, float y)
        : Enemy(x, y, /*health*/ 0, /*damage*/ 0, /*isMelee*/ true) {
        // Baphomet is twice as powerful and a bit faster than goblin
        xpOnDrop         = 20 + rand() % 20;       // 20-39 XP
        coinsOnDrop      = 14;                     // twice goblin
        damage           = 12;                     // twice goblin's damage
        maxHealth        = 60 + rand() % 20;       // 40-59 health
        health           = maxHealth;
        attackCooldownStart = attackCooldown = 20; // faster attacks
        isMelee          = true;
        speed            = 0.8f;                   // increased speed
        maxSpeed         = 4.0f;                   // higher top speed
        animation        = rand() % 100; // random animation start
        toBeDeleted      = false;                 // not dead
        canAttack        = true;
        isAttacking      = false;
        isAttackingAnimation = 0;
        enemyWidth       = 50;                     // larger hitbox
        enemyHeight      = 70;
        attackRadius     = 60;                     // bigger reach
        this->x = x;
        this->y = y;
        vx = vy = 0;
    }

    void draw(RenderWindow& window) override {
        // Load or retrieve Baphomet textures
        //individual size 82/128

        Texture& mobTexture = TextureManager::getInstance().find("Baphometset");

        Sprite sprite(mobTexture,IntRect({82*2,82},{82,128})); // Initialize with a valid texture

        // draw (centered) at x, y
        sprite.setPosition(Vector2f(82/2, 128/2)); // center the sprite
        // auto scale the sprite to 30x50 px
        sprite.setScale(Vector2f(0.365, 0.390)); // scale the sprite

        // draw
        if (isAttackingAnimation > 0) {
            // draw attack animation
            if (getX() < 200) {
                sprite.setTextureRect(IntRect({0,0},{82,128})); // set texture to attack
            } else {
                sprite.setTextureRect(IntRect({82,0},{82,128}));  // set texture to attack mirrored
            }
        } else {
            // draw walk animation
            if (animation % 50 < 25) {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82*2,0},{82,128})); // set texture to walk1
                } else {
                    sprite.setTextureRect(IntRect({0,128},{82,128})); // set texture to walk1 mirrored
                }
            } else {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82,128},{82,128})); // set texture to walk2
                } else {
                    sprite.setTextureRect(IntRect({82*2,128},{82,128})); // set texture to walk2 mirrored
                }
            }
        }

        // set the scale to 50x70 px (can increase for bigger enemies)
        // scale set to equal 50x70 from 82x128
        sprite.setScale(Vector2f(0.609, 0.546)); // scale the sprite
        // set the position to x, y
        sprite.setPosition(Vector2f(getX() - 30.0f / 2, getY() - 50.0f / 2)); // center the sprite
        // draw the sprite
        window.draw(sprite); // draw the sprite
        // update animation
        animation = (animation % 100) + 1;
        if (isAttackingAnimation > 0) --isAttackingAnimation;
    }

    void update(RenderWindow& window) override {
        if (toBeDeleted) return;

        // cooldowns
        if (attackCooldown > 0) --attackCooldown;
        else canAttack = true;

        // movement: home in
        float dx = 200 - getX();
        float dy = playerY - getY();
        float dist = std::sqrt(dx*dx + dy*dy);
        if (getX() < 200) vx += speed; else vx -= speed;
        if (getY() < playerY) vy += speed; else vy -= speed;

        // chaotic vertical jitter when animation hits a specific frame
        if (animation == 2) {
            vy += rand_uniform(-3.f, 3.f);
        }

        // clamp
        vx = std::clamp(vx, -maxSpeed, maxSpeed);
        vy = std::clamp(vy, -maxSpeed, maxSpeed);

        // attack logic
        if (dist < attackRadius && canAttack) {
            isAttacking = true;
            canAttack = false;
            attackCooldown = attackCooldownStart;
            isAttackingAnimation = 30;
            playerTakeDamage(damage); // deal damage to player
            // knockback player
            playerVx += -3 * cos(atan2(dy, dx)); // knockback in the opposite direction of the player
            playerVy += -3 * sin(atan2(dy, dx)); // knockback in the opposite direction of the player
            SoundManager::getInstance().playSound("took_damage");
        } else {
            isAttacking = false;
        }

        // take damage logic from sword
        Angle towardsPlayerAngle = sf::radians(atan2(dy, dx)); // angle towards player
        // check if sword hitboxes are inside enemy hitbox
        if (swordHitbox1.x > getX() - enemyWidth / 2 && swordHitbox1.x < getX() + enemyWidth / 2 &&
            swordHitbox1.y > getY() - enemyHeight / 2 && swordHitbox1.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * 10); // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians()); // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians()); // knockback in the opposite direction of the player
        }
        if (swordHitbox2.x > getX() - enemyWidth / 2 && swordHitbox2.x < getX() + enemyWidth / 2 &&
            swordHitbox2.y > getY() - enemyHeight / 2 && swordHitbox2.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * 10); // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians()); // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians()); // knockback in the opposite direction of the player
        }

        // apply movement
        x += vx * 0.8f + playerVx;
        y += vy * 0.8f;
    }
};


// clasa de bani
class Coin {
private:
    float x, y;       // coordonatele monedei
    int animation;    // animatia monedei (de la 1 la 100)
    float vx, vy; // viteza pe axa x si y (nu e folosita inca)
    bool toBeDeleted = false; // flag pentru autodistrugere

public:
    // constructor
    Coin(float x, float y) : x(x), y(y) {
        animation = rand() % 100 + 1; // genereaza o animatie aleatoare intre 1 si 100
        vx = rand_uniform(-2, 2); // genereaza o viteza aleatoare pe axa x
        vy = rand_uniform(-2, 2); // genereaza o viteza aleatoare pe axa y
        toBeDeleted = false; // seteaza flag-ul de autodistrugere la false
    }

    // getters
    float getX() const { return x; }
    float getY() const { return y; }
    int getAnimation() const { return animation; }

    // setters
    void setX(float x) { this->x = x; }
    void setY(float y) { this->y = y; }
    void setAnimation(int animation) { this->animation = animation; }

    // update
    void update() {
        // actualizeaza pozitia monedei in functie de viteza
        x += vx;
        y += vy;

        // verifica daca moneda a iesit din ecran
        if (x < 0 || x > 800 || y < 0 || y > 600) {
            vx = -vx; // inverseaza viteza pe axa x
            vy = -vy; // inverseaza viteza pe axa y
        }

        if (x < 0) x = 0, vx += 1;
        if (x > 800) x = 800, vx -= 1;
        if (y < 0) y = 0, vy += 1;
        if (y > 600) y = 600, vy -= 1;

        // daca distanta catre player < 50, go to player
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 50) {
            vx = (200 - x) / 10; // seteaza viteza pe axa x
            vy = (playerY - y) / 10; // seteaza viteza pe axa y
        }

        // daca distanta catre player < 10, da bani si autodistrugere
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 10) {
            balance += 1; // adauga 1 la balanta
            toBeDeleted = true; // seteaza flag-ul de autodistrugere la true
            SoundManager::getInstance().playSound("pickup_coin"); // reda sunetul de pickup
            cout << "DEBUG: played sound cuz x=" << x << " y=" << y << ", dist = " << distance(Vector2f(x, y), Vector2f(200, playerY)) << endl;
        }

        // frictiune 
        vx *= 0.96; // aplica frictiunea pe axa x
        vy *= 0.96; // aplica frictiunea pe axa y

        // se misca in functie de player
        x += playerVx;
        // y += playerVy;
    }

    // metoda pentru desenarea monedei
    void draw(RenderWindow& window) {
        // selecteaza textura in functie de animatie
        //Texture& texture = (animation <= 50)
        //    ? TextureManager::getInstance().find("coin1")
        //    : TextureManager::getInstance().find("coin2");

        Texture& texture = TextureManager::getInstance().find("Coinset");

        Sprite sprite(texture);
        if (animation<=50) sprite.setTextureRect(IntRect({0,0},{32,32}));
        else sprite.setTextureRect(IntRect({32,0},{32,32}));

        sprite.setPosition(Vector2f(x, y)); // seteaza pozitia sprite-ului
        //schimba factorul de scaling pentru textura mai mare sau mica
        sprite.setScale(Vector2f(0.35, 0.35)); // seteaza scalarea sprite-ului
        window.draw(sprite); // deseneaza sprite-ul

        // actualizeaza animatia
        animation = (animation % 100) + 1;
    }

    // getters setters deletion
    bool isToBeDeleted() const { return toBeDeleted; } // getter pentru flag-ul de autodistrugere
    void setToBeDeleted(bool toBeDeleted) { this->toBeDeleted = toBeDeleted; } // setter pentru flag-ul de autodistrugere
};

// clasa de exp
class ExpOrb {
private:
    float x, y;       // coordonatele orb-ului
    int animation;    // animatia orb-ului (de la 1 la 100)
    float vx, vy;     // viteza pe axa x si y
    bool toBeDeleted = false; // flag pentru autodistrugere
    Angle towardsPlayerAngle = sf::radians(0); // unghiul catre player
public:
    // constructor
    ExpOrb(float x, float y) : x(x), y(y) {
        animation = rand() % 100 + 1; // genereaza o animatie aleatoare intre 1 si 100
        vx = rand_uniform(-5, 5); // genereaza o viteza aleatoare pe axa x
        vy = rand_uniform(-5, 5); // genereaza o viteza aleatoare pe axa y
        toBeDeleted = false; // seteaza flag-ul de autodistrugere la false
    }

    // getters
    float getX() const { return x; }
    float getY() const { return y; }
    int getAnimation() const { return animation; }

    // setters
    void setX(float x) { this->x = x; }
    void setY(float y) { this->y = y; }
    void setAnimation(int animation) { this->animation = animation; }

    // update
    void update() {
        // calculeaza viteza catre player
        float dx = 200 - x;
        float dy = playerY - y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 5) {
            float angle = atan2(dy, dx); // calculeaza unghiul catre player
            towardsPlayerAngle = sf::radians(angle); // seteaza unghiul catre player folosind sfml::Angle
            vx += cos(angle) / 2; // seteaza viteza pe axa x
            vy += sin(angle) / 2; // seteaza viteza pe axa y

        }
        // limit viteza orb-ului
        if (vx > 5) vx = 5;
        if (vx < -5) vx = -5;
        if (vy > 5) vy = 5;
        if (vy < -5) vy = -5;

        // actualizeaza pozitia orb-ului
        x += vx;
        y += vy;

        // player
        x += playerVx;
        // y += playerVy;

        // daca distanta catre player < 10, creste xp level si autodistrugere
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 10) {
            // Creste nivelul de experienta
            xp += rand() % 10 + 1; // adauga un numar aleatoriu de xp (1-10)
            cout << "DEBUG: picked up exp orb, xp = " << xp << endl; // afiseaza xp-ul
            toBeDeleted = true; // seteaza flag-ul de autodistrugere la true
            SoundManager::getInstance().playSound("pickup_exp"); // reda sunetul de pickup
        }

        // frictiune mai mare decat Coin
        vx *= 0.96; // aplica frictiunea pe axa x
        vy *= 0.96; // aplica frictiunea pe axa y
    }

    // metoda pentru desenarea orb-ului
    void draw(RenderWindow& window) {
        // selecteaza textura in functie de animatie
        Texture& texture = TextureManager::getInstance().find("Expset");

        Sprite sprite(texture);
        if (animation<=50) sprite.setTextureRect(IntRect({0,0},{32,32}));
        else sprite.setTextureRect(IntRect({32,0},{32,32}));

        sprite.setPosition(Vector2f(x, y)); // seteaza pozitia sprite-ului
        sprite.setScale(Vector2f(0.25, 0.25)); // seteaza scalarea sprite-ului
        window.draw(sprite); // deseneaza sprite-ul

        // actualizeaza animatia
        animation = (animation % 100) + 1;
    }

    // getters setters deletion
    bool isToBeDeleted() const { return toBeDeleted; } // getter pentru flag-ul de autodistrugere
    void setToBeDeleted(bool toBeDeleted) { this->toBeDeleted = toBeDeleted; } // setter pentru flag-ul de autodistrugere
};

// clasa de noduri pentru skill tree
class skillTreeNode {
private:
    bool active = false;    // flag pentru verificarea daca nodul este alocat

    int currentAlloc = 0;   // cate puncte alocate sunt in nod
    int maxAlloc = 0;   // maximul de puncte alocate
    int cost = 0;   // cate skillpointuri costa un punct alocat
    int levelReq = 1; // nivelul minim de experienta necesar pentru a aloca un punct
    string description; // descriptia nodului care este afisata in dreapta sus
    CircleShape node = CircleShape(30); // nodul care este desenat pe ecran
    float x = 0,y = 0; // positia nodului

public:

    // constructor default
    skillTreeNode() {
        active = false;
        cost = 0;
        description = "Test";
    }

    // constructor cu circleshape
   skillTreeNode(int cost, string description, CircleShape node, float x, float y) {
        this->cost = cost;
        this->description = description;
        this->node = node;
        this->x = node.getPosition().x;
        this->y = node.getPosition().y;
   }

    // constructor pentru date principale
    skillTreeNode(int maxAlloc,int cost, int levelReq, string description, float x, float y) {
        this->maxAlloc = maxAlloc;
        this->cost = cost;
        this->levelReq = levelReq;
        this->description = description;
        this->node.setPosition(Vector2f(x,y));
        this->x = x;
        this->y = y;
    }

    // functie pentru updatarea si desenarea nodului
    void draw(RenderWindow &window, Font &font) {
        if (node.getGlobalBounds().contains( window.mapPixelToCoords(Mouse::getPosition(window)))) {
            // facem nodul negru daca suntem cu mouse-ul pe el
            node.setFillColor(Color::Black);

            //  scrisul pentru alocare care este desenat in dreapta jos
            Text alloc(font, to_string(currentAlloc) + "/" + to_string(maxAlloc), 18);
            alloc.setFont(font);
            alloc.setPosition(node.getPosition() - Vector2f(-40, -65));

            // background-ul pentru alocare
            RectangleShape allocBack;
            allocBack.setSize(Vector2f(alloc.getGlobalBounds().size.x, alloc.getGlobalBounds().size.y+15));
            allocBack.setFillColor(Color::Black);
            allocBack.setPosition(alloc.getPosition());

            // scrisul pentru descriptia nodului care este desenat in dreapta sus
            Text desc(font, description, 18);
            desc.setFillColor(Color::White);
            desc.setPosition(node.getPosition()-Vector2f(-40, 40));

            // background-ul pentru descriptie
            RectangleShape descBack;
            descBack.setSize(Vector2f(desc.getGlobalBounds().size.x, desc.getGlobalBounds().size.y+15));
            descBack.setFillColor(Color::Black);
            descBack.setPosition(desc.getPosition());

            // desenarea fiecaru-ia (ordinea conteaza!!)

            window.draw(node);

            window.draw(descBack);
            window.draw(desc);

            window.draw(allocBack);
            window.draw(alloc);

        }else node.setFillColor(Color::White); // daca nu mai suntem cu mouse-ul pe nod il facem inapoi alb

        // daca am alocat toate punctele dintr-un nod il facem verde
        if (currentAlloc == maxAlloc) {
            node.setFillColor(Color::Green);
        }

        // daca nu avem destul level pentru noduri atunci sunt facute rosu
        if (level<levelReq) {
            node.setFillColor(Color::Red);
        }

        //ordinea functiilor de mai sus conteaza!!!(in teorie)
    }

    // functie pentru verificarea alocarii unui nod
    void allocate(RenderWindow &window) {
        // verificam daca mouse-ul este pe nod, daca apasam, daca avem destul level, daca avem destule
        // skillpoint-uri si daca mai sunt puncte de alocat
        if(node.getGlobalBounds().contains( window.mapPixelToCoords(Mouse::getPosition(window)))) {
            if (leftClick && level>=levelReq && skillPoints>=cost
                && currentAlloc<maxAlloc) {
                active = true; // nodul devine alocat
                skillPoints-=cost;
                currentAlloc+=1;
                cout << "Skill activated" << endl;
                leftClick=false; // resetam butonul de apasare pentru a nu apasa de prea multe ori
                }
        }
    }

    // setters
    void setActive(bool active) {
       this->active = active;
    }

    void setPosition(float x, float y) {
        this->node.setPosition(Vector2f(x,y));
        this->x = node.getPosition().x;
        this->y = node.getPosition().y;
   }

    void setCurrentAlloc(int currentAlloc) {
        this->currentAlloc = currentAlloc;
    }

    // getters
    bool getActive() {
       return active;
   }

    float getX() {
       return x;
   }

    float getY() {
       return y;
    }

    CircleShape getNode() {
        return node;
    }

    string getDescription() {
        return description;
    }

    int getCost() {
        return cost;
    }

    int getMaxAlloc() {
        return maxAlloc;
    }

    int getLevelReq() {
        return levelReq;
    }

    int getCurrentAlloc() {
        return currentAlloc;
    }
};

// -------------------------------------------------------------------- containere --------------------------------------------------------------------

vector<Object> mapObjects;  // container pentru obiectele din harta
vector<Tile> mapTiles;      // container pentru tile-urile din harta
vector<Coin> mapCoins;     // container pentru monedele din harta
vector<ExpOrb> mapExpOrbs; // container pentru orb-urile de exp din harta
// inamici & entitati
vector<EnemyGoblin> mapGoblins; // container pentru inamicii de tip goblin
vector<EnemyBaphomet> mapBaphomets; // container pentru inamicii de tip Baphomet
// noduri skilltree
skillTreeNode skills[10];

// ---------------------------------------------------------- functiile folosite de obiecte --------------------------------------------------------------------

void spawnCoinAt(float x, float y) {
    mapCoins.push_back(Coin(x, y)); // adauga o moneda la coordonatele x, y
}

void spawnXPAt(float x, float y) {
    mapExpOrbs.push_back(ExpOrb(x, y)); // adauga un orb de exp la coordonatele x, y
}

void playerTakeDamage(int damage) {
    // daca are scut, scade din scut
    // daca nu are scut, scade din viata
    if (playerArmor > 0) {
        playerArmor -= damage; // scade din scut
        if (playerArmor < 0) {
            playerHealth += playerArmor; // scade din viata
            playerArmor = 0; // seteaza scutul la 0
        }
    } else {
        playerHealth -= damage; // scade din viata
    }
}

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

    // // debug: C = spawns coins at 500, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::C)]) {  // daca tasta C este apasata
    //     mapCoins.push_back(Coin(500, 300)); // adauga o moneda la coordonatele 500, 300
    // }

    // // debug: V = spawns exp orbs at 500, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::V)]) {  // daca tasta V este apasata
    //     mapExpOrbs.push_back(ExpOrb(500, 300)); // adauga un exp orb la coordonatele 500, 300
    // }

    // debug: X = spawns goblin at 500, 300
    if (keysPressed[static_cast<int>(Keyboard::Key::X)] && frameCount % 10 == 0) {  // daca tasta X este apasata si frameCount % 10 == 0
        mapGoblins.push_back(EnemyGoblin(500, 300, 100, 10, true)); // adauga un goblin la coordonatele 500, 300
        cout << "DEBUG: spawned goblin at 500, 300" << endl; // afiseaza mesaj de spawn
    }

    // debug: B = spawns baphomet at 600, 300
    if (keysPressed[static_cast<int>(Keyboard::Key::B)] && frameCount % 10 == 0) {  // daca tasta B este apasata si frameCount % 10 == 0
        mapBaphomets.push_back(EnemyBaphomet(600, 300)); // adauga un baphomet la coordonatele 600, 300
        cout << "DEBUG: spawned baphomet at 600, 300" << endl; // afiseaza mesaj de spawn
    }

    // cout << "dashing data: " << "dashing: " << dashing << " canDash: " << canDash << " dashDuration: " << dashDuration << " dashCooldown: " << dashCooldown << endl;
}

// -------------------------------------------------------------------- initializare --------------------------------------------------------------------
void init() {
    frameCount = 0; // initializeaza frameCount
    // map textures
    TextureManager::getInstance().justLoad("Tileset");
    //TextureManager::getInstance().justLoad("dirt");
    //TextureManager::getInstance().justLoad("stone");
    // player textures
    TextureManager::getInstance().justLoad("Playerset");
    // weapon textures
    TextureManager::getInstance().justLoad("weapon_basic_sword");
    // coin textures
    TextureManager::getInstance().justLoad("Coinset");
    // exp textures
    TextureManager::getInstance().justLoad("Expset");
    // icons
    TextureManager::getInstance().justLoad("Miscset");
    // enemy gobllin
    TextureManager::getInstance().justLoad("Goblinset");
    // enemy baphomet
    TextureManager::getInstance().justLoad("Baphometset");

    if (!uiFont.openFromFile("./res/PixelPurl.ttf")) { 
            std::cerr << "Error loading font!\n";
    }

    srand(time(NULL));  // initializeaza generatorul de numere aleatorii
    // adauga obiecte in containerul mapObjects
    mapObjects.push_back(Object(100, 150, 20, Color::Red));
    mapObjects.push_back(Object(200, 210, 30, Color::Blue));
    mapObjects.push_back(Object(300, 350, 20, Color::Green));
    mapObjects.push_back(Object(400, 480, 50, Color::Yellow));
    mapObjects.push_back(Object(500, 300, 30, Color::Magenta));
    mapObjects.push_back(Object(600, 170, 40, Color::Cyan));

    worldItems.push_back(ItemObject(Object(150, 150, 10, Color::Red), Item("Sword", "basic ass sword", "weapon_basic_sword", ItemType::Weapon)));
    worldItems.push_back(ItemObject(Object(250, 250, 10, Color::Cyan), Item("Dark Gloves", "Dark forces lie within these gloves", "weapon_basic_sword", ItemType::Equipment)));
    worldItems.push_back(ItemObject(Object(350, 350, 10, Color::Magenta), Item("Rare Gloves", "gloves that are rare", "weapon_basic_sword", ItemType::Equipment)));
    
    // init player pos
    playerX = 200;
    playerY = 300;
    playerVx = 0;
    playerVy = 0;

    // 80x60
    for (int i = 0; i < 84/4; i++) {
        for (int j = 0; j < 64/4; j++) {
            mapTiles.push_back(Tile(i * 40 - 40, j * 40 - 40, 40, 40, Color::Black));

            auto random = rand_uniform(0, 100);
            if (random < 8) { // 8% chance
                mapTiles.back().setType("grass1");
            } else if (random < 8 + 10) { // 10% chance
                mapTiles.back().setType("grass3");
            } else { // 82% chance
                mapTiles.back().setType("grass2");
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

    nearbyItemIndex = -1;
    float closestDistance = -1;

    for (auto& worldItem : worldItems) {
        if (worldItem.pickedUp) continue;
        
        sf::Vector2f itemPos(worldItem.obj.x, worldItem.obj.y);
        float dist = distance(Vector2f(200, playerY), itemPos);
        
        if (dist < pickupRadius && (dist < closestDistance || closestDistance < 0)) {
            closestDistance = dist;
            nearbyItemIndex = static_cast<int>(&worldItem - &worldItems[0]);
        }
    }
    // Handle pickup input
    
    if (keysPressed[static_cast<int>(Keyboard::Key::E)]) {
        if (pressedE == false && nearbyItemIndex != -1 &&  worldItems[nearbyItemIndex].item.type != ItemType::Null) {
            if(worldItems[nearbyItemIndex].pickedUp == false && inventoryWindow.onAddItem(worldItems[nearbyItemIndex])){
                worldItems[nearbyItemIndex].pickedUp = true;
                nearbyItemIndex = -1;
            }
        }
        pressedE = true;
    }
    else
    {
        pressedE = false;
    }

    if(keysPressed[static_cast<int>(Keyboard::Key::I)]) {
        if(openedInventoryThisPress == false)
        {
            inventoryVisible = !inventoryVisible;
            inventoryWindow.isVisible = inventoryVisible;
            openedInventoryThisPress = true;
        }
    }
    else
    {
        openedInventoryThisPress = false;
    }

    inventoryWindow.updateAlways(window);
    inventoryWindow.update(window, playerInventory);

    // move tiles but their x % 10 so they repeat
    for (Tile& tile : mapTiles) {
        tile.setX(tile.getX() + playerVx);
        if (tile.getX() > 800) tile.setX(tile.getX() - 840.0f);
        if (tile.getX() < -40) tile.setX(tile.getX() + 840.0f);
    }

    // xp mechanics
    if (xp >= levelXP) {
        level += 1; // creste nivelul jucatorului
        xp = 0; // resetare xp
        skillPoints++;
        levelXP += levelXP / 9;

        // creste armura maxima
        playerMaxArmor += rand() % 10 + 2; // adauga un numar aleatoriu de armura maxima (1-5)
        
        // creste viata maxima
        playerMaxHealth += 5; 

        // creste viata curenta cu 50%
        playerHealth += playerMaxHealth / 2; // creste viata curenta 
        if (playerHealth > playerMaxHealth) playerHealth = playerMaxHealth; // limiteaza viata curenta la maxima
        
        // creste armura curenta cu 50%
        playerArmor += playerMaxArmor / 2; // creste armura curenta
        if (playerArmor > playerMaxArmor) playerArmor = playerMaxArmor; // limiteaza armura curenta la maxima

        playerDamageMultiplier += rand_uniform(0.1f, 0.3f); // creste damage multiplier-ul
    }

    // player health mechanics
    if (playerHealth <= 0) {
        playerHealth = 0; // limiteaza viata curenta la 0
        playerArmor = 0; // limiteaza armura curenta la 0
        cout << "DEBUG: player is dead" << endl; // afiseaza mesaj de moarte
        // play dead.wav
        SoundManager::getInstance().playSound("dead"); // reda sunetul de moarte
        // messagebox with you died
        MessageBoxA(NULL, "You died!", "Game Over", MB_OK | MB_ICONERROR); // afiseaza mesaj de moarte

        // restart the exe
        system("start /B /WAIT /MIN cmd /C start \"\" \"./bin/bloodwavez.exe\""); // restarteaza jocul

        exit(0); // iese din program
    } else if (playerHealth > playerMaxHealth) {
        playerHealth = playerMaxHealth; // limiteaza viata curenta la maxima
    }
    if (playerArmor > playerMaxArmor) {
        playerArmor = playerMaxArmor; // limiteaza armura curenta la maxima
    }
    if (playerArmor < 0) {
        playerArmor = 0; // limiteaza armura curenta la 0
    }

    //cout<<mouseX<<" "<<mouseY<<endl;

    // allows switching between the player screen and the skill tree
    if (keysPressed[static_cast<int>(Keyboard::Key::P)]) {
        if (window.getView().getCenter()==playerView.getCenter() && skillTreeDown) {
            window.setView(skillTree);
            skillTreeDown = false;
        }
        else if (window.getView().getCenter()==skillTree.getCenter() && skillTreeDown) {
            window.setView(playerView);
            skillTreeDown = false;

        }
    }

    if (keysReleased[static_cast<int>(Keyboard::Key::P)]) skillTreeDown = true;
}

// -------------------------------------------------------------------- desenare --------------------------------------------------------------------
void drawPlayerAt(RenderWindow& window, float x, float y, float speed = 0, float scale = 0.45f) {
    // deseneaza jucatorul in functie de viteza si pozitie
    Texture& texture = TextureManager::getInstance().find("Playerset");;
    Sprite sprite(texture);

    //individual texture size 90/128

    if (notMoving) {
        sprite.setTextureRect(IntRect({0,128*2},{90,128}));
    } else if (playerVx < 0 && moveAnimationCounter % 40 < 20) {
        sprite.setTextureRect(IntRect({90,128},{90,128}));
        moveAnimationCounter ++;
    } else if (playerVx < 0 && moveAnimationCounter % 40 >= 20) {
        sprite.setTextureRect(IntRect({90*2,0},{90,128}));
        moveAnimationCounter ++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 < 20) {
        sprite.setTextureRect(IntRect({90*2,128},{90,128}));
        moveAnimationCounter ++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 >= 20) {
        sprite.setTextureRect(IntRect({0,128},{90,128}));
        moveAnimationCounter ++;
    } else {
        sprite.setTextureRect(IntRect({0,128*2},{90,128}));
    }

    if (dashing && playerVx < 0) {
        sprite.setTextureRect(IntRect({0,0},{90,128}));
    } else if (dashing && playerVx > 0) {
        sprite.setTextureRect(IntRect({90,0},{90,128}));
    }

    sprite.setOrigin(Vector2f(45, 64)); // seteaza originea sprite-ului la mijlocul lui
    sprite.setScale(Vector2f(scale, scale)); // seteaza scalarea sprite-ului
    sprite.setPosition(Vector2f(x, y)); // seteaza pozitia sprite-ului (folosim vector2f)
    window.draw(sprite); // deseneaza sprite-ul
}

void drawItemInfo(sf::RenderWindow& window) {
    if(nearbyItemIndex == -1) return;
    if (worldItems[nearbyItemIndex].item.type == ItemType::Null) return;

    // Create text elements
    sf::Text nameText(uiFont, "", 27);
    sf::Text descText(uiFont, "", 27);
    sf::RectangleShape underline(sf::Vector2f(200, 2));

    // Configure name text
    nameText.setFont(uiFont);
    nameText.setString(worldItems[nearbyItemIndex].item.name);
    nameText.setCharacterSize(24);
    nameText.setFillColor(sf::Color::White);
    nameText.setStyle(sf::Text::Bold);

    // Configure description text
    descText.setFont(uiFont);
    descText.setString(worldItems[nearbyItemIndex].item.description);
    descText.setCharacterSize(18);
    descText.setFillColor(sf::Color(200, 200, 200));

    // Position elements
    sf::FloatRect nameBounds = nameText.getLocalBounds();
    sf::Vector2f basePosition(400, 500);
    
    nameText.setPosition(basePosition);
    underline.setPosition(Vector2f(basePosition.x, basePosition.y + nameBounds.size.y + 12));
    descText.setPosition(Vector2f(basePosition.x, basePosition.y + nameBounds.size.y + 15));

    // Draw elements
    window.draw(nameText);
    window.draw(underline);
    window.draw(descText);
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
        float angle = atan2(mouseY - playerY, mouseX - (handX)) * 180 / 3.14159f + 90; // +90 pentru a alinia sprite-ul
        sprite.setRotation(sf::degrees(angle)); // seteaza rotatia sprite-uluis
        
        window.draw(sprite); // deseneaza sprite-ul

        // // draw hitbox-ul 1 (DEBUG) = tip-ul sabiei
        // CircleShape circle(5); // creeaza un cerc cu raza 20
        // circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        // circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        // circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 30, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 30)); // seteaza pozitia cercului (folosim vector2f)
        // window.draw(circle); // deseneaza cercul
        // // draw hitbox-ul 2 (DEBUG) = tip-ul sabiei
        // circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        // circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        // circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 37, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 37)); // seteaza pozitia cercului (folosim vector2f)
        // window.draw(circle); // deseneaza cercul

        swordHitbox1 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 30, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 30); // seteaza pozitia hitbox-ului sabiei (folosim vector2f)
        swordHitbox2 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 37, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 37); // seteaza pozitia hitbox-ului sabiei (folosim vector2f)
    }
}

void drawCoins(RenderWindow& window) {
    for (Coin& coin : mapCoins) {
        coin.update(); // actualizeaza moneda
        coin.draw(window); // deseneaza moneda

        // deja exista o limitare in obiect dar asta nu strica
        if (coin.getX() < 0) coin.setX(0); // limiteaza moneda pe axa x
        if (coin.getX() > 800) coin.setX(800); // limiteaza moneda pe axa x
        if (coin.getY() < 0) coin.setY(0); // limiteaza moneda pe axa y
        if (coin.getY() > 600) coin.setY(600); // limiteaza moneda pe axa y

        // delete moneda daca este autodistrusa
        if (coin.isToBeDeleted()) {
            auto it = remove_if(mapCoins.begin(), mapCoins.end(), [](Coin& c) { return c.isToBeDeleted(); });
            mapCoins.erase(it, mapCoins.end()); // sterge moneda din vector
        }
    }
}

void drawExpOrbs(RenderWindow& window) {
    for (ExpOrb& orb : mapExpOrbs) {
        orb.update(); // actualizeaza orb-ul
        orb.draw(window); // deseneaza orb-ul

        // delete orb daca este autodistrus
        if (orb.isToBeDeleted()) {
            auto it = remove_if(mapExpOrbs.begin(), mapExpOrbs.end(), [](ExpOrb& o) { return o.isToBeDeleted(); });
            mapExpOrbs.erase(it, mapExpOrbs.end()); // sterge orb-ul din vector
        }
    }
}

void drawBars(RenderWindow& window) {
    // draw health bar
    RectangleShape healthBar(Vector2f(200, 10)); // creeaza un dreptunghi cu dimensiunile specificate
    healthBar.setFillColor(Color::Green); // seteaza culoarea
    healthBar.setPosition(Vector2f(10, 10)); // seteaza pozitia (folosim vector2f)
    healthBar.setSize(Vector2f(200 * (float)((float)playerHealth / (float)playerMaxHealth), 10)); // seteaza dimensiunile
    window.draw(healthBar); // deseneaza dreptunghiul

    // draw armor bar
    RectangleShape armorBar(Vector2f(200, 20)); // creeaza un dreptunghi cu dimensiunile specificate
    armorBar.setFillColor(Color::Blue); // seteaza culoarea
    armorBar.setPosition(Vector2f(10, 30)); // seteaza pozitia (folosim vector2f)
    armorBar.setSize(Vector2f(200 * (float)((float)playerArmor / (float)playerMaxArmor), 10)); // seteaza dimensiunile
    window.draw(armorBar); // deseneaza dreptunghiul

    // draw xp bar
    RectangleShape xpBar(Vector2f(200, 30)); // creeaza un dreptunghi cu dimensiunile specificate
    xpBar.setFillColor(Color::Yellow); // seteaza culoarea
    xpBar.setPosition(Vector2f(10, 50)); // seteaza pozitia (folosim vector2f)
    xpBar.setSize(Vector2f(200 * (float)((float)xp / (float)levelXP), 10)); // seteaza dimensiunile
    window.draw(xpBar); // deseneaza dreptunghiul

    // draw the 2px strok for each bar above
    RectangleShape healthBarStroke(Vector2f(200, 10)); // creeaza un dreptunghi cu dimensiunile specificate
    healthBarStroke.setFillColor(Color::Transparent); // seteaza culoarea
    RectangleShape armorBarStroke(Vector2f(200, 10)); // creeaza un dreptunghi cu dimensiunile specificate
    armorBarStroke.setFillColor(Color::Transparent); // seteaza culoarea
    RectangleShape xpBarStroke(Vector2f(200, 10)); // creeaza un dreptunghi cu dimensiunile specificate
    xpBarStroke.setFillColor(Color::Transparent); // seteaza culoarea

    healthBarStroke.setOutlineThickness(2); // seteaza grosimea conturului
    healthBarStroke.setOutlineColor(Color::Black); // seteaza culoarea conturului
    armorBarStroke.setOutlineThickness(2); // seteaza grosimea conturului
    armorBarStroke.setOutlineColor(Color::Black); // seteaza culoarea conturului
    xpBarStroke.setOutlineThickness(2); // seteaza grosimea conturului
    xpBarStroke.setOutlineColor(Color::Black); // seteaza culoarea conturului

    healthBarStroke.setPosition(Vector2f(10, 10)); // seteaza pozitia (folosim vector2f)
    armorBarStroke.setPosition(Vector2f(10, 30)); // seteaza pozitia (folosim vector2f)
    xpBarStroke.setPosition(Vector2f(10, 50)); // seteaza pozitia (folosim vector2f)

    window.draw(healthBarStroke); // deseneaza dreptunghiul
    window.draw(armorBarStroke); // deseneaza dreptunghiul
    window.draw(xpBarStroke); // deseneaza dreptunghiul

    // get icon set
    // each icon size is 32x32
    Texture& texture = TextureManager::getInstance().find("Miscset");

    Sprite sprite(texture);

    //change for smaller or bigger icons
    float scale = 0.6f;

    sprite.setTextureRect(IntRect({0,0},{32,32}));
    sprite.setPosition(Vector2f(217, 11-5)); // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale)); // seteaza scalarea sprite-ului
    window.draw(sprite); // deseneaza sprite-ul

    sprite.setTextureRect(IntRect({32,0},{32,32}));
    sprite.setPosition(Vector2f(217, 29-5)); // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale)); // seteaza scalarea sprite-ului
    window.draw(sprite); // deseneaza sprite-ul

    sprite.setTextureRect(IntRect({0,32},{32,32}));
    sprite.setPosition(Vector2f(217, 48)); // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale)); // seteaza scalarea sprite-ului
    window.draw(sprite); // deseneaza sprite-ul
}

void drawText(RenderWindow& window) {
    // show lvl and money under the bars
    Font font;
    if (!font.openFromFile("./res/PixelPurl.ttf")) {
        cout << "Failed to load font: PixelPurl.ttf" << endl;
        return;
    }

    Text levelText(font, "LVL " + to_string(level) + " | " + to_string(balance) + "$", 27); // creeaza un text cu font-ul specificat
    levelText.setFont(font); // seteaza font-ul text-ului
    levelText.setFillColor(Color::White);
    levelText.setPosition(Vector2f(10, 60));

    // Create shadow text
    Text shadowText = levelText; // Copy the original text properties
    shadowText.setFillColor(Color::Black); // Set shadow color
    shadowText.setPosition(levelText.getPosition() + Vector2f(2, 2)); // Offset the shadow

    window.draw(shadowText); // Draw the shadow first
    window.draw(levelText); // Draw the original text on top

    // show health/max health and armor/max and xp/next xp to the right of the bars
    Text healthText(font, to_string(playerHealth) + "/" + to_string(playerMaxHealth), 18);
    healthText.setFont(font);
    healthText.setFillColor(Color::White);
    healthText.setPosition(Vector2f(240, 8-5)); // Position to the right of the health bar icon

    Text armorText(font, to_string(playerArmor) + "/" + to_string(playerMaxArmor), 18);
    armorText.setFont(font);
    armorText.setFillColor(Color::White);
    armorText.setPosition(Vector2f(240, 28-5)); // Position to the right of the armor bar icon

    Text xpText(font, to_string(xp) + "/" + to_string(levelXP), 18);
    xpText.setFont(font);
    xpText.setFillColor(Color::White);
    xpText.setPosition(Vector2f(240, 48-5)); // Position to the right of the xp bar icon

    // Create shadow texts
    Text healthShadow = healthText;
    healthShadow.setFillColor(Color::Black);
    healthShadow.setPosition(healthText.getPosition() + Vector2f(1, 1)); // Offset shadow

    Text armorShadow = armorText;
    armorShadow.setFillColor(Color::Black);
    armorShadow.setPosition(armorText.getPosition() + Vector2f(1, 1)); // Offset shadow

    Text xpShadow = xpText;
    xpShadow.setFillColor(Color::Black);
    xpShadow.setPosition(xpText.getPosition() + Vector2f(1, 1)); // Offset shadow

    // Draw shadows first
    window.draw(healthShadow);
    window.draw(armorShadow);
    window.draw(xpShadow);

    // Draw original texts on top
    window.draw(healthText);
    window.draw(armorText);
    window.draw(xpText);
}

// draw all enemies
void drawEnemies(RenderWindow& window) {
    // GOBLINS
    for (EnemyGoblin& enemy : mapGoblins) {
        enemy.update(window); // actualizeaza inamicul
        enemy.draw(window); // deseneaza inamicul

        // delete inamic daca este autodistrus
        if (enemy.isToBeDeleted()) {
            auto it = remove_if(mapGoblins.begin(), mapGoblins.end(), [](EnemyGoblin& e) { return e.isToBeDeleted(); });
            mapGoblins.erase(it, mapGoblins.end()); // sterge inamicul din vector
        }
    }

    // BAPHOMETS
    for (EnemyBaphomet& enemy : mapBaphomets) {
        enemy.update(window); // actualizeaza inamicul
        enemy.draw(window); // deseneaza inamicul

        // delete inamic daca este autodistrus
        if (enemy.isToBeDeleted()) {
            auto it = remove_if(mapBaphomets.begin(), mapBaphomets.end(), [](EnemyBaphomet& e) { return e.isToBeDeleted(); });
            mapBaphomets.erase(it, mapBaphomets.end()); // sterge inamicul din vector
        }
    }
}

// bonusul total adaugat de skill tree la damageMultiplier

/*
vector<pair<bool,pair<int,String>>> func {
                    {false,{10,"% increased damage"}},
                    {false,{10,"% increased health"}},
                    {false,{10,"% chance to deal double damage"}},
                    {false,{10,"% of life added as flat damage to your weapon"}},
                    {false,{5,"% chance to not take damage from hits" }},
                    {false,{20,"% increased damage for 3s after dashing"}},
                    {false,{30,"% increased weapon flat damage"}}
};

*/

void drawSkillTree(RenderWindow& window) {

    Texture& texture = TextureManager::getInstance().find("SkillTree");

    //creare background
    Sprite skillTree(texture);
    skillTree.setPosition(Vector2f(-400, 700));
    skillTree.setScale(Vector2f(8, 5));

    window.draw(skillTree);

    // generarea nodurilor
    int nrSkills = 10;

    int x=-350,y=950, offx=200, offy=150; // x,y = pozitia nodului, offx,offy = offset-ul nodului
    int levelReq=1,levelScale=5;

    // initializare o singura data
    if (skills[0].getCost()==0) {
        skills[0] = skillTreeNode(5,1, levelReq, "Test",x, y);

        skills[1] = skillTreeNode(5,5, levelReq*levelScale, "Test",x+offx, y-offy);
        skills[2] = skillTreeNode(5,5, levelReq*levelScale, "Test",x+offx, y);
        skills[3] = skillTreeNode(5,5, levelReq*levelScale, "Test",x+offx, y+offy);

        skills[4] = skillTreeNode(5,5, levelReq*levelScale*2, "Test",x+2*offx,y-offy);
        skills[5] = skillTreeNode(5,5, levelReq*levelScale*2, "Test",x+2*offx,y);
        skills[6] = skillTreeNode(5,5, levelReq*levelScale*2, "Test",x+2*offx,y+offy);

        skills[7] = skillTreeNode(5,5, levelReq*levelScale*3, "Test",x+3*offx,y-offy);
        skills[8] = skillTreeNode(5,5, levelReq*levelScale*3, "Test",x+3*offx,y);
        skills[9] = skillTreeNode(5,5, levelReq*levelScale*3, "Test",x+3*offx,y+offy);
    }


    Font font;
    if (!font.openFromFile("./res/PixelPurl.ttf")) {
        cout << "Failed to load font: PixelPurl.ttf" << endl;
        return;
    }

    // textul de sus afisand cate skillpoint-uri are jucatorul
    Text nrSkp(font,"You have " + to_string(skillPoints) + " skill points.");
    nrSkp.setFillColor(Color::Black);
    nrSkp.setPosition(Vector2f(-100,700));
    nrSkp.setCharacterSize(40);

    window.draw(nrSkp);

    // foru-l principal pentru updatarea skilltree-ului
    for (int i=0;i<nrSkills;i++) {
        window.draw(skills[i].getNode());
        skills[i].draw(window, font);
        skills[i].allocate(window);
    }

    //playerDamageMultiplier=1.0f + 1.0f * totalDamageIncrease/100;
    //cout<<playerDamageMultiplier<<endl;
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

    drawEnemies(window); // desenare inamici

    drawExpOrbs(window); // desenare orb-uri de exp
    drawCoins(window); // desenare monede

    // those needs 2 be on top
    drawBars(window); // desenare bare de viata, armura si xp
    drawText(window); // desenare text cu nivelul si banii

    drawSkillTree(window);
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

    skillTree.setCenter(Vector2f(0,1000));
    skillTree.setSize(window.getView().getSize());

    playerView.setCenter(window.getView().getCenter());
    playerView.setSize(window.getView().getSize());

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
                if(keyEv) {
                    keysPressed[static_cast<int>(keyEv->code)] = true;
                    keysReleased[static_cast<int>(keyEv->code)] = false;
                }
            }
            if (event.is<Event::KeyReleased>()) {
                auto keyEv = event.getIf<Event::KeyReleased>();
                if(keyEv) {
                    keysPressed[static_cast<int>(keyEv->code)] = false;
                    keysReleased[static_cast<int>(keyEv->code)] = true;
                }
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

        frameCount++; // creste numarul de frame-uri
    }
    return 0;  // intoarce 0 la terminarea executiei
}
