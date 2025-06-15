// specific c++ includes
#include <algorithm>    // pentru algoritmi standard (sort, etc.)
#include <cmath>        // pentru functii matematice (sqrt, pow, etc.)
#include <cstdlib>      // pentru functii utilitare (rand, srand)
#include <ctime>        // pentru functii legate de timp (time, srand)
#include <filesystem>   // pentru manipularea fisierelor si directoarelor
#include <iostream>     // pentru operatii de intrare/iesire
#include <map>          // pentru utilizarea containerului map
#include <windows.h>    // pentru api-ul windows (winmain, ascundere fereastra cmd)
#include <tlhelp32.h>   // pentru operatii pe procese si thread-uri in windows
#include <memory>       // pentru pointeri inteligenti si managementul memoriei
#include <optional>     // pentru std::optional
#include <random>       // pentru std::mt19937
#include <string>       // pentru manipularea stringurilor
#include <type_traits>  // pentru std::is_same si alte tipuri de verificari
#include <variant>      // pentru std::variant
#include <vector>       // pentru utilizarea containerului vector

// ------ alte fisiere
#include "ai.h"

// -------------------------------------------------------------------- sfml --------------------------------------------------------------------
#include <SFML/Audio.hpp>     // pentru gestionarea sunetelor si muzicii
#include <SFML/Graphics.hpp>  // pentru grafica (desenare forme, sprites, etc.)
#include <SFML/System.hpp>    // pentru functii de sistem (timp, thread-uri, etc.)
#include <SFML/Window.hpp>    // pentru manipularea ferestrelor si evenimentelor

// -------------------------------------------------------------------- namespace --------------------------------------------------------------------
using namespace std;
using namespace sf;

// -------------------------------------------------------------------- constante --------------------------------------------------------------------
const int FPS_LIMIT = 60;    // limita de cadre pe secunda
const int SPEED_LIMIT = 4;   // 3px/sec
const bool RELEASE = false;  // flag pentru release (true = release, false = debug)

constexpr int INVENTORY_WIDTH = 600;
constexpr float INVENTORY_HEIGHT = 400;
constexpr int SLOT_SIZE = 64;
constexpr int PADDING = 10;
constexpr int COLUMNS = 6;

unsigned int frameCount = 0;

// -------------------------------------------------------------------- functii utilitare --------------------------------------------------------------------
// genereaza un numar aleatoriu in intervalul [a, b)
float rand_uniform(float a, float b) {
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}

// functie template pentru calcularea distantei euclidiene intre doua puncte
template <typename T>
float distance(Vector2<T> a, Vector2<T> b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// -------------------------------------------------------------------- Views --------------------------------------------------------------------
View skillTree;
View playerView;

// -------------------------------------------------------------------- variabile globale legate de joc
bool mouseDown = false;           // flag: true daca mouse-ul este apasat
bool leftClick = false;           // flag: true daca butonul stang a fost apasat
bool rightClick = false;          // flag: true daca butonul drept a fost apasat
float mouseX = 0, mouseY = 0;     // coordonatele curente ale mouse-ului
bool keysPressed[256] = {false};  // starea tastelor
bool keysReleased[256] = {false};

int playerX = 200;
int playerY = 300;      // pozitia initiala a jucatorului
bool notMoving = true;  // flag pentru miscarea jucatorului
int playerSpeed = 1;    // viteza de miscare a jucatorului

string playerHolding = "none";       // arma pe care o tine jucatorul init
int currentHoldingSwordDamage = 10;  // damage-ul sabiei pe care o tine jucatorul
Vector2f swordHitbox1(0, 0);         // hitbox-ul armei (sword1)
Vector2f swordHitbox2(0, 0);         // hitbox-ul armei (sword2ss)
int pickupRadius = 40;               // raza de pickup pentru obiecte

// arc
bool canFireArrows = false;        // flag pentru a verifica daca se poate trage cu arcul
string arrowType = "arrow_basic";  // tipul sagetii pe care o trage jucatorul
int arrowSpeed = 5;                // viteza sagetii
int arrowDamage = 10;              // damage-ul sagetii
int arrowCooldown = 60;            // cooldown-ul pentru tragerea cu arcul (in frame-uri)
int currentBowCooldown = 0;        // cooldown-ul curent pentru arcul jucatorului

float playerVx = 0;            // velocitatea pe axa x a jucatorului
float playerVy = 0;            // velocitatea pe axa y a jucatoruluis
int moveAnimationCounter = 0;  // contor pentru animatia de miscare a jucatorului

bool dashing = false;       // flag pentru dash
bool canDash = true;        // flag pentru cooldown-ul dash-ului
float dashSpeed = 8;        // viteza de dash
float dashDuration = 30;    // durata dash-ului in cadre
int dashCooldown = 60 * 3;  // cooldown-ul dash-ului in frameuri

int balance = 0;               // numarul de $$$
float moneyMultiplier = 1.0f;  // multiplicatorul de bani al jucatorului
int xp = 0;                    // xp-ul jucatorului
int level = 1;                 // starting level
int levelXP = 100;             // xp-ul necesar pentru a urca la nivelul urmator
int skillPoints = 0;
unsigned long long levelProgress = 0;  // progresul la singurul nivel

int playerHealth = 100;             // viata jucatorului
int playerMaxHealth = 100;          // viata maxima a jucatorului
int playerArmor = 0;                // armura jucatorului
int regenPer60Frames = 5;           // regenerare la fiecare 60 de secunde
float armorPowerMultiplier = 1.0f;  // multiplicatorul de putere al armurii (scade damage-ul primit)s
int playerMaxArmor = 100;           // armura maxima a jucatorului
float playerDamageMultiplier = 1;   // muliplicatorul de damage al jucatorului care va fi inmultit cu damage-ul armei
float totalDamageIncrease = 0;      // totalul de %damage increase al jucatorului

bool skillTreeDown = true;  // folosit pt schimbarea intre playerView si skillTreeView

int speedSkillLevel = 0;
std::vector<float> speedMultipliers = {1.0f, 1.01f, 1.02f, 1.03f, 1.04f, 1.06f};

// folosit pt inventar si pickuping
sf::Font uiFont;
int nearbyItemIndex;
bool inventoryVisible = false;
bool openedInventoryThisPress = false;
bool pressedE = false;

// boss spawnat
bool bossSpawned = false;  // flag pentru a verifica daca boss-ul a fost spawnat
// bos batut?
bool bossDefeated = false;  // flag pentru a verifica daca boss-ul a fost batut
// daca a spre sfarsit gen ultimul nivel
bool inHell = false;  // flag pentru a verifica daca jucatorul este in infern

// ---------------------------------------------------- functii folosite de obiecte (forward decl) -----------------------------------------------------------

void spawnCoinAt(float x, float y);
void spawnXPAt(float x, float y);
void playerTakeDamage(int amount);

// -------------------------------------------------------------------- obiecte --------------------------------------------------------------------

// obiect abstract (nu exista scop inca)
class Object {
public:
    float x, y;    // coordonatele obiectului
    float radius;  // raza obiectului (pentru desenare)
    Color color;   // culoarea obiectului
    // bool hasTexture = false; // flag pentru a verifica daca obiectul are textura
    // constructor
    Object(float x, float y, float radius, Color color) : x(x), y(y), radius(radius), color(color) {}

    // metoda pentru desenarea obiectului in fereastra
    void draw(RenderWindow& window) {
        CircleShape circle(radius);          // creeaza un cerc cu raza specificata
        circle.setFillColor(color);          // seteaza culoarea
        circle.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
        window.draw(circle);                 // deseneaza cercul
    }

    ~Object() {
        // destructor
    }
};

class TextureManager {
private:
    std::map<std::string, sf::Texture> textures;  // mapa pentru a stoca texturile incarcate
    // contructor privat pentru a preveni instantierea directa
    // singleton pattern
    TextureManager() {}

public:
    // bla bla bla singleton stuff
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // instancea singletons
    static TextureManager& getInstance() {
        static TextureManager instance;  // Instanta unica
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
            textures[filename] = texture;  // adauga textura in mapa
        }
        return textures[filename];  // returneaza textura incarcata
    }

    void justLoad(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        // verifica daca textura este deja incarcata
        if (textures.find(filename) == textures.end()) {
            sf::Texture texture;
            if (!texture.loadFromFile(filename)) {
                std::cout << "Failed to load texture: " << filename << "\n";
            }
            textures[filename] = texture;                         // adauga textura in mapa
            std::cout << "Loaded texture: " << filename << "\n";  // afiseaza mesaj de incarcare
        }
    }

    void clear() {
        textures.clear();  // sterge toate textele incarcate
    }
    // destructor
    ~TextureManager() {
        clear();  // sterge toate textele incarcate
    }
    // metoda pentru a verifica daca textura este incarcata
    bool isLoaded(const std::string& name) {
        std::string filename = "./res/" + name + ".png";
        return textures.find(filename) != textures.end();  // verifica daca textura este incarcata
    }
};

enum class ItemType { Weapon, Equipment, Null };

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
        sprite.setScale(Vector2f(SLOT_SIZE / static_cast<float>(texture.getSize().x), SLOT_SIZE / static_cast<float>(texture.getSize().y)));
    }

    ~Item() {
        // destructor
        // nu este necesar sa facem nimic aici, deoarece sprite-ul este gestionat de TextureManager
    }
};

struct ItemObject {
    double x, y;    // coordonatele obiectului
    double radius;  // raza obiectului
    Item item;
    bool pickedUp = false;
    string textureName;

    ItemObject(double x, double y, double radius, Item item) : x(x), y(y), radius(radius), item(item), pickedUp(false) {}
    ItemObject() : x(0), y(0), radius(0), item("", "", "", ItemType::Null), textureName(""), pickedUp(true){};

    // metoda pentru desenarea obiectului in fereastra
    void draw(RenderWindow& window) {
        if (item.type == ItemType::Null)
            return;  // nu desena daca item-ul este null

        sf::Texture& texture = TextureManager::getInstance().find(item.texturePath);
        sf::Sprite sprite(texture);
        sprite.setScale(sf::Vector2f(55.0f / texture.getSize().x, 55.0f / texture.getSize().y));
        sprite.setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));
        window.draw(sprite);
    }

    ~ItemObject() {
        // destructor
        // nu este necesar sa facem nimic aici, deoarece item-ul si obiectul sunt gestionate de alte clase
    }
};

vector<ItemObject> worldItems;
void pickupSoundEffect();

class Inventory {
private:
    ItemObject firstWeapon;
    ItemObject secondWeapon;
    std::vector<ItemObject> stackables;
    int itemSlots = 16;
    int currentlyOccupied = 0;

public:
    Inventory() { stackables.reserve(itemSlots); }

    std::vector<ItemObject>& getEquipment() { return stackables; }

    ItemObject getFirstWeapon() { return firstWeapon; }

    ItemObject getSecondWeapon() { return secondWeapon; }

    // todo implementeaza inventar vizual
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
                    pickupSoundEffect();  // Play pickup sound effect
                    return true;
                } else if (secondWeapon.item.type == ItemType::Null) {
                    secondWeapon = itemobj;
                    std::cout << "Weapon assigned to slot 2\n";
                    pickupSoundEffect();  // Play pickup sound effect
                    return true;
                } else {
                    std::cout << "Both weapon slots are full\n";
                    return false;  // poate faci un slot de backup
                }

            case ItemType::Equipment:
                if (currentlyOccupied >= itemSlots)
                    return false;

                currentlyOccupied++;
                stackables.push_back(itemobj);
                return true;
        }

        return false;
    }

    void removeEquipment(int index) {
        if (index >= 0 && index < static_cast<int>(stackables.size())) {
            ItemObject removedItem = stackables[index];

            removedItem.pickedUp = false;
            removedItem.x = rand_uniform(190, 210);
            removedItem.y = rand_uniform(playerY - 10, playerY + 10);

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
            dropped.x = rand_uniform(190, 210);
            dropped.y = rand_uniform(playerY - 10, playerY + 10);
            firstWeapon = ItemObject();  // reset to null
        } else if (slot == 2 && secondWeapon.item.type != ItemType::Null) {
            dropped = secondWeapon;
            dropped.pickedUp = false;
            dropped.x = rand_uniform(190, 210);
            dropped.y = rand_uniform(playerY - 10, playerY + 10);
            secondWeapon = ItemObject();  // reset to null
        }
        worldItems.push_back(dropped);
    }

    ~Inventory() {
        // destructor
        // nu este necesar sa facem nimic aici, deoarece item-urile si obiectele sunt gestionate de alte clase
    }
};

Inventory playerInventory;

// clasa pentru interfata inventarului
class InventoryWindow {
private:
    sf::RectangleShape background;
    sf::Text titleText;
    std::vector<sf::RectangleShape> weaponSlots;
    sf::Font font;
    sf::Text tooltipText;
    sf::Text equipmentText;
    sf::Text weaponText;
    sf::RectangleShape tooltipBackground;
    int hoveredIndex = -1;
    bool removedItemOnClick = false;

public:
    bool isVisible = false;

    // updateaza dimensiunea fundalului inventarului in functie de numarul de iteme
    void updateBackgroundSize() {
        // std::vector<ItemObject>& equipment = playerInventory.getEquipment();
        // int rows = static_cast<int>(std::ceil(equipment.size() / static_cast<float>(COLUMNS)));
        // int extraHeight = rows * (SLOT_SIZE + PADDING);
        // int dynamicHeight = 90 + extraHeight;

        // background.setSize(sf::Vector2f(INVENTORY_WIDTH, std::max(150.f, static_cast<float>(dynamicHeight))));

        std::vector<ItemObject>& equipment = playerInventory.getEquipment();
        int rows = 3;
        int extraHeight = rows * (SLOT_SIZE + PADDING);
        int dynamicHeight = 90 + extraHeight;

        background.setSize(sf::Vector2f(INVENTORY_WIDTH, std::max(150.f, static_cast<float>(dynamicHeight))));
    }

    // constructor
    InventoryWindow(sf::Font& font)
        : tooltipText(font, "Tooltip", 14), equipmentText(font, "Equipment", 18), weaponText(font, "Weapon", 18), titleText(font, "Inventory", 24) {
        this->font = font;

        background.setSize(sf::Vector2f(INVENTORY_WIDTH, INVENTORY_HEIGHT));
        background.setFillColor(sf::Color(20, 20, 20, 200));
        background.setOutlineThickness(2);
        background.setOutlineColor(sf::Color(100, 100, 255));

        for (int i = 0; i < 2; i++) {
            sf::RectangleShape slot(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
            slot.setFillColor(sf::Color(80, 80, 80));
            slot.setOutlineThickness(1);
            slot.setOutlineColor(sf::Color::White);
            weaponSlots.push_back(slot);
        }

        titleText.setFont(font);
        titleText.setString("Inventory");
        titleText.setCharacterSize(24);
        titleText.setFillColor(sf::Color::White);

        tooltipBackground.setFillColor(sf::Color(0, 0, 0, 220));
        tooltipText.setFont(font);
        tooltipText.setCharacterSize(14);
        tooltipText.setStyle(sf::Text::Regular | sf::Text::Italic);
        tooltipText.setFillColor(sf::Color::White);

        updateBackgroundSize();
    }

    // deseneaza tot inventarul
    void draw(sf::RenderWindow& window) {
        // if not visible, just draw in the bottom left corner the two main weapons
        // (+ their tooltip)
        if (!isVisible) {
            for (int i = 0; i < 2; ++i) {
                weaponSlots[i].setPosition(Vector2f(10 + i * (SLOT_SIZE + PADDING), window.getSize().y - SLOT_SIZE - 10));
                if (hoveredIndex == -1 && i == 0) {
                    weaponSlots[i].setFillColor(sf::Color(50, 50, 50));  // Darker background
                    weaponSlots[i].setOutlineColor(sf::Color::Cyan);     // Lighter border
                } else if (hoveredIndex == -2 && i == 1) {
                    weaponSlots[i].setFillColor(sf::Color(50, 50, 50));  // Darker background
                    weaponSlots[i].setOutlineColor(sf::Color::Cyan);     // Lighter border
                } else {
                    weaponSlots[i].setFillColor(sf::Color(80, 80, 80));  // Default background
                    weaponSlots[i].setOutlineColor(sf::Color::White);    // Default border
                }
                window.draw(weaponSlots[i]);
            }

            if (playerInventory.getFirstWeapon().item.type != ItemType::Null) {
                Sprite s = playerInventory.getFirstWeapon().item.sprite;
                s.setOrigin(Vector2f(0, 0));
                s.setRotation(sf::degrees(0));  // Reset rotation
                s.setPosition(weaponSlots[0].getPosition());
                window.draw(s);
            }

            if (playerInventory.getSecondWeapon().item.type != ItemType::Null) {
                Sprite s = playerInventory.getSecondWeapon().item.sprite;
                s.setOrigin(Vector2f(0, 0));
                s.setRotation(sf::degrees(0));  // Reset rotation
                s.setPosition(weaponSlots[1].getPosition());
                window.draw(s);
            }

            if (hoveredIndex == -1 || hoveredIndex == -2) {
                sf::Text shadow = tooltipText;
                shadow.setFillColor(sf::Color::Black);
                shadow.setPosition(tooltipText.getPosition() + Vector2f(1, 1));

                window.draw(tooltipBackground);
                window.draw(shadow);
                tooltipText.setFillColor(sf::Color::White);
                window.draw(tooltipText);
            }
            return;
        }

        if (!isVisible)
            return;

        sf::Vector2f center = window.getView().getCenter();
        background.setPosition(Vector2f(center.x - INVENTORY_WIDTH / 2, center.y - INVENTORY_HEIGHT / 2));

        window.draw(background);

        titleText.setPosition(background.getPosition() + Vector2f(PADDING, 5));

        titleText.setFillColor(sf::Color::White);
        tooltipText.setFillColor(sf::Color::White);
        window.draw(titleText);
        window.draw(tooltipText);

        // weaponText should be under the title at the top with proper spacing
        weaponText.setPosition(background.getPosition() + Vector2f(PADDING, 30));
        window.draw(weaponText);
        // equipmentText should be under the weaponText with proper spacing
        equipmentText.setPosition(background.getPosition() + Vector2f(PADDING, 44));
        window.draw(equipmentText);

        // plaseaza sloturile de arme deasupra
        for (int i = 0; i < 2; ++i) {
            weaponSlots[i].setPosition(background.getPosition() + Vector2f(INVENTORY_WIDTH - (2 - i) * (SLOT_SIZE + PADDING), 40));
        }

        for (auto& slot : weaponSlots) {
            window.draw(slot);
        }

        // deseneaza armele echipate
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

        // deseneaza sloturile de echipament
        drawEquipmentGrid(window);

        if (hoveredIndex != -100) {
            sf::Text shadow = tooltipText;
            shadow.setFillColor(sf::Color::Black);
            shadow.setPosition(tooltipText.getPosition() + Vector2f(1, 1));
            window.draw(tooltipBackground);
            window.draw(shadow);
            tooltipText.setFillColor(sf::Color::White);
            // cout << "DEBUG: SHOW TOOLTIP\n";
            window.draw(tooltipText);
        }
    }

    // deseneaza sloturile + itemele din grid-ul de echipamente
    void drawEquipmentGrid(sf::RenderWindow& window) {
        sf::Vector2f bgPos = background.getPosition();
        const float startX = bgPos.x + PADDING;
        const float startY = bgPos.y + 90;

        std::vector<ItemObject>& equipment = playerInventory.getEquipment();

        int maxSlots = 16;
        for (int i = 0; i < maxSlots; ++i) {
            int col = i % COLUMNS;
            int row = i / COLUMNS;
            sf::Vector2f pos(startX + col * (SLOT_SIZE + PADDING), startY + row * (SLOT_SIZE + PADDING));

            sf::RectangleShape slot(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
            slot.setPosition(pos);
            slot.setFillColor(sf::Color(50, 50, 50, 150));
            slot.setOutlineThickness(1);

            if (i == hoveredIndex)
                slot.setOutlineColor(sf::Color::Cyan);
            else
                slot.setOutlineColor(sf::Color::White);

            window.draw(slot);
        }

        for (size_t i = 0; i < equipment.size(); ++i) {
            int col = i % COLUMNS;
            int row = i / COLUMNS;
            sf::Vector2f pos(startX + col * (SLOT_SIZE + PADDING), startY + row * (SLOT_SIZE + PADDING));

            equipment[i].item.position = pos;
            equipment[i].item.sprite.setPosition(pos);
            window.draw(equipment[i].item.sprite);
        }
    }

    // updateaza tooltip-ul cand mouse-ul e deasupra unui slot
    void updateTooltip(sf::Vector2f mousePos) {
        const auto setTooltip = [&](const Item& item) {
            tooltipText.setString(item.name + "\n" + item.description);
            sf::FloatRect bounds = tooltipText.getLocalBounds();
            tooltipBackground.setSize(sf::Vector2f(bounds.size.x + 20, bounds.size.y + 20));
            tooltipBackground.setPosition(mousePos + Vector2f(20, 20));
            tooltipText.setPosition(mousePos + Vector2f(25, 25));
        };

        if (hoveredIndex == -1)
            setTooltip(playerInventory.getFirstWeapon().item);
        else if (hoveredIndex == -2)
            setTooltip(playerInventory.getSecondWeapon().item);
        else if (hoveredIndex >= 0 && hoveredIndex < (int)playerInventory.getEquipment().size())
            setTooltip(playerInventory.getEquipment()[hoveredIndex].item);
        else {
            tooltipText.setString("");                      // clear tooltip if not hovering over a valid item
            tooltipBackground.setSize(sf::Vector2f(0, 0));  // clear background size
        }
    }

    // logica de update - hover si click pe sloturi
    void update(sf::RenderWindow& window, Inventory& inventory) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        hoveredIndex = -100;

        for (size_t i = 0; i < playerInventory.getEquipment().size(); i++) {
            if (playerInventory.getEquipment()[i].item.sprite.getGlobalBounds().contains(mousePos)) {
                hoveredIndex = static_cast<int>(i);
                break;
            }
        }

        if (weaponSlots[0].getGlobalBounds().contains(mousePos))
            hoveredIndex = -1;
        if (weaponSlots[1].getGlobalBounds().contains(mousePos))
            hoveredIndex = -2;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (hoveredIndex != -100 && removedItemOnClick == false) {
                onRemoveItem(hoveredIndex);
                removedItemOnClick = true;
            }
        } else {
            removedItemOnClick = false;
        }

        // update the existing equipmentText and weaponText with the current inventory status
        equipmentText.setString("Equipments: " + std::to_string(playerInventory.getEquipment().size()) + " / " + std::to_string(16));
        equipmentText.setPosition(background.getPosition() + Vector2f(PADDING, background.getSize().y - 30));

        weaponText.setString(
            "Weapons: " +
            std::to_string((playerInventory.getFirstWeapon().item.type != ItemType::Null) + (playerInventory.getSecondWeapon().item.type != ItemType::Null)) +
            " / 2");
        weaponText.setPosition(background.getPosition() + Vector2f(PADDING, background.getSize().y - 50));

        updateTooltip(mousePos);
    }

    // sterge un item din inventar (drop)
    void onRemoveItem(int index) {
        if (index == -1) {
            if (playerHolding == playerInventory.getFirstWeapon().item.texturePath)
                playerHolding = "none";
            playerInventory.dropWeapon(1);
        } else if (index == -2) {
            if (playerHolding == playerInventory.getSecondWeapon().item.texturePath)
                playerHolding = "none";
            playerInventory.dropWeapon(2);
        } else {
            std::vector<ItemObject>& equipment = playerInventory.getEquipment();
            if (index >= 0 && index < (int)equipment.size()) {
                if (playerHolding == equipment[index].item.texturePath)
                    playerHolding = "none";
                playerInventory.removeEquipment(index);
                updateBackgroundSize();
            }
        }
    }

    // adauga un item nou si updateaza UI
    bool onAddItem(ItemObject item) {
        if (!playerInventory.pickUp(item))
            return false;
        updateBackgroundSize();
        return true;
    }

    // update global (hover pentru arme si tooltip)
    void updateAlways(sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        hoveredIndex = -100;

        if (playerInventory.getFirstWeapon().item.type != ItemType::Null && playerInventory.getFirstWeapon().item.sprite.getGlobalBounds().contains(mousePos))
            hoveredIndex = -1;

        if (playerInventory.getSecondWeapon().item.type != ItemType::Null && playerInventory.getSecondWeapon().item.sprite.getGlobalBounds().contains(mousePos))
            hoveredIndex = -2;

        if (hoveredIndex == -1 || hoveredIndex == -2)
            updateTooltip(mousePos);
    }
};

InventoryWindow inventoryWindow(uiFont);

class SoundManager {
private:
    map<string, SoundBuffer> soundBuffers;       // mapa pentru a stoca buffer-ele de sunet incarcate
    map<string, std::shared_ptr<Sound>> sounds;  // mapa pentru a stoca pointerii la sunete

    // constructor privat pentru a preveni instantierea directa
    SoundManager() {}

public:
    // singleton pattern
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    static SoundManager& getInstance() {
        static SoundManager instance;  // instanta unica
        return instance;
    }

    // metoda pentru a incarca un sunet
    void loadSound(const string& name, const string& filename) {
        string full_filename = "./res/" + filename + ".wav";  // adauga calea catre fisier
        if (soundBuffers.find(name) == soundBuffers.end()) {
            SoundBuffer buffer;
            if (!buffer.loadFromFile(full_filename)) {  // incarca fisierul
                cout << "Failed to load sound: " << filename << endl;
                return;
            }
            soundBuffers[name] = buffer;                                     // adauga buffer-ul in mapa
            sounds[name] = std::make_shared<sf::Sound>(soundBuffers[name]);  // asociaza buffer-ul cu un pointer la sunet
            cout << "Loaded sound: " << filename << endl;

            // do not play sound when loaded
            sounds[name]->setVolume(0);  // seteaza volumul la 0 pentru a nu reda sunetul la incarcare
        }
    }

    // metoda pentru a reda un sunet
    void playSound(const string& name, int volume = 30) {
        if (sounds.find(name) != sounds.end()) {
            // seteaza volumul la 100% (sau orice altceva vrei)
            sounds[name]->setVolume(volume);  // seteaza volumul
            sounds[name]->play();
        } else {
            cout << "Sound not found for playing: " << name << endl;

            cout << "Trying to load sound: " << name << endl;

            loadSound(name, name);  // incarca sunetul daca nu este gasit

            // daca sunetul a fost incarcat, reda-l
            if (sounds.find(name) != sounds.end()) {
                sounds[name]->setVolume(volume);  // seteaza volumul
                sounds[name]->play();
                cout << "Sound loaded and played: " << name << endl;
            } else {
                cout << "Failed to load sound: " << name << endl;  // daca nu a reusit sa il incarce
            }
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
    bool isLoaded(const string& name) { return soundBuffers.find(name) != soundBuffers.end(); }

    // destructor
    ~SoundManager() {
        sounds.clear();
        soundBuffers.clear();
    }
};

void pickupSoundEffect() {
    SoundManager::getInstance().playSound("pickup", 55);  // Play the pickup sound effect
}

class bgmManager {
public:
    static bgmManager& getInstance() {
        static bgmManager instance;
        return instance;
    }

    // Load all BGM files from a directory matching the prefix (e.g., "bgmN.mp3")
    void loadAll(const std::string& directory = "./res/", const std::string& prefix = "bgm", const std::string& ext = ".mp3") {
        playlist.clear();
        for (int i = 1; i <= 9; ++i) {
            std::string filename = directory + prefix + std::to_string(i) + ext;
            if (std::filesystem::exists(filename)) {
                playlist.push_back(filename);
                std::cout << "Loaded BGM: " << filename << std::endl;
            } else {
                std::cerr << "BGM file not found: " << filename << std::endl;
            }
        }
        // seed random engine
        rng.seed(time(nullptr));
    }

    // Start playing a random track
    void playRandom(int volume = 50) {
        if (playlist.empty()) {
            std::cerr << "Playlist is empty. Call loadAll() first." << std::endl;
            return;
        }
        std::uniform_int_distribution<std::size_t> dist(0, playlist.size() - 1);
        cout << "Randomly selecting BGM from playlist of size: " << playlist.size() << "\n";
        currentIndex = dist(rng);
        playCurrent(volume);
        playing = true;
    }

    // Play specific file
    void playFile(const std::string& filename, int volume = 50) {
        if (!music)
            music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filename)) {
            std::cerr << "Failed to open BGM: " << filename << std::endl;
            return;
        }
        music->setLooping(true);
        music->setVolume(static_cast<float>(volume));
        music->play();
        playing = true;
        currentVolume = volume;
    }

    // Stop playback
    void stop() {
        if (music && music->getStatus() == sf::Music::Status::Playing) {
            music->stop();
        }
        playing = false;
    }

    // Must be called periodically (e.g., each frame) to auto-advance
    void update() {
        if (music && playing && music->getStatus() == sf::Music::Status::Stopped) {
            cout << "piesa s-a terminat, trecem la urmatoarea\n";
            cout << "indexul curent: " << currentIndex << "\n";
            playRandom(currentVolume);
            cout << "am trecut la piesa: " << playlist[currentIndex] << "\n";
        }
    }

private:
    bgmManager() : currentIndex(0), playing(false), currentVolume(50) {}
    ~bgmManager() { stop(); }
    bgmManager(const bgmManager&) = delete;
    bgmManager& operator=(const bgmManager&) = delete;

    // Play the track at currentIndex
    void playCurrent(int volume) {
        if (currentIndex >= playlist.size())
            return;
        if (!music)
            music = std::make_unique<sf::Music>();
        if (!music->openFromFile(playlist[currentIndex])) {
            std::cerr << "Failed to open BGM: " << playlist[currentIndex] << std::endl;
            return;
        }
        music->setLooping(false);
        music->setVolume(static_cast<float>(volume));
        music->play();
        playing = true;
        currentVolume = volume;
    }

    std::vector<std::string> playlist;
    std::unique_ptr<sf::Music> music;
    std::mt19937 rng;
    std::size_t currentIndex;
    bool playing;
    int currentVolume;
};

class Tile {
private:
    float x, y;           // coordonatele tile-ului
    float width, height;  // dimensiunile tile-ului
    Color color;          // culoarea tile-ului
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
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(40.0f / texture.getSize().x, 40.0f / texture.getSize().y));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "stone", draw ./res/stone.png
        if (type == "stone") {
            Texture& texture = TextureManager::getInstance().find("stone");
            Sprite sprite(texture);
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(40.0f / texture.getSize().x, 40.0f / texture.getSize().y));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // factorul de scaling folosit este 1.25 deoarece stim ca fiecare tile esete 32/32 dar noi
        // avem celule de 40/40 in matrice deci 32*1.25=40

        // if type == "grass1"
        if (type == "grass1") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture, IntRect({0, 0}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "grass2"
        if (type == "grass2") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture, IntRect({32, 0}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "grass3"
        if (type == "grass3") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture, IntRect({0, 32}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "grass4"
        if (type == "grass4") {
            Texture& texture = TextureManager::getInstance().find("Tileset");
            Sprite sprite(texture, IntRect({32, 32}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "hell1"
        if (type == "hell1") {
            Texture& texture = TextureManager::getInstance().find("HellTileset");
            Sprite sprite(texture, IntRect({0, 0}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "hell2"
        if (type == "hell2") {
            Texture& texture = TextureManager::getInstance().find("HellTileset");
            Sprite sprite(texture, IntRect({32, 0}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "hell3"
        if (type == "hell3") {
            Texture& texture = TextureManager::getInstance().find("HellTileset");
            Sprite sprite(texture, IntRect({0, 32}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }

        // if type == "hell4"
        if (type == "hell4") {
            Texture& texture = TextureManager::getInstance().find("HellTileset");
            Sprite sprite(texture, IntRect({32, 32}, {32, 32}));
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia (folosim vector2f)
            sprite.setScale(Vector2f(1.25, 1.25));
            window.draw(sprite);  // deseneaza sprite-ul
        }
    }

    // setters and getters
    float getX() const { return x; }                         // getter pentru x
    float getY() const { return y; }                         // getter pentru y
    float getWidth() const { return width; }                 // getter pentru width
    float getHeight() const { return height; }               // getter pentru height
    Color getColor() const { return color; }                 // getter pentru culoare
    string getType() const { return type; }                  // getter pentru tip
    void setX(float x) { this->x = x; }                      // setter pentru x
    void setY(float y) { this->y = y; }                      // setter pentru y
    void setWidth(float width) { this->width = width; }      // setter pentru width
    void setHeight(float height) { this->height = height; }  // setter pentru height
    void setColor(Color color) { this->color = color; }      // setter pentru culoare
    void setType(string type) { this->type = type; }         // setter pentru tip

    ~Tile() {
        // destructor
        // nu este necesar sa facem nimic aici, deoarece culoarea si tipul sunt gestionate de alte clase
    }
};

// clasa asta sigur va fi mostenita
class Enemy {
protected:
    int health;     // viata inamicului
    int maxHealth;  // viata maxima a inamicului
    int damage;     // damage-ul inamicului
    bool isMelee;
    float x, y;
    float vx, vy;                  // viteza pe axa x si y
    int animation;                 // animatia inamicului (de la 1 la 100)
    bool toBeDeleted = false;      // DEAD
    bool isAttacking = false;      // flag pentru atac (va schima animatia, si daca e aproape de player si melee va da damage)
    bool canAttack = true;         // flag pentru cooldown-ul atacului
    int attackCooldown = 60;       // current cooldown in frameuri
    int attackCooldownStart = 60;  // attackCooldown dupa atac = attackCooldownStart
    int xpOnDrop = 0;              // xp-ul pe care il drop-eaza inamicul
    int coinsOnDrop = 0;           // banii pe care ii drop-eaza inamicul
    float speed = 0.5f;            // viteza de miscare a inamicului
    float maxSpeed = 2.5f;         // viteza maxima a inamicului
    float enemyHeight = 50;        // dimensiunile inamicului (pentru hitbox)
    float enemyWidth = 30;         // dimensiunile inamicului (pentru hitbox)
    float attackRadius = 40;       // raza de atac a inamicului (pentru hitbox)
    int isAttackingAnimation = 0;  // animatia de atac (30 frameuri)
public:
    // Constructor
    Enemy(float x, float y, int health, int damage, bool isMelee) {
        this->x = x;               // seteaza pozitia pe axa x
        this->y = y;               // seteaza pozitia pe axa y
        this->health = health;     // seteaza viata inamicului
        this->damage = damage;     // seteaza damage-ul inamicului
        this->isMelee = isMelee;   // seteaza daca inamicul este melee sau ranged
        vx = 0;                    // initializare viteza pe axa x
        vy = 0;                    // initializare viteza pe axa y
        animation = 0;             // initializare animatie
        toBeDeleted = false;       // initializare DEAD
        isAttacking = false;       // initializare atac
        canAttack = true;          // initializare cooldown atac
        attackCooldown = 60;       // initializare cooldown atac
        attackCooldownStart = 60;  // initializare cooldown atac
        xpOnDrop = 0;              // initializare xp drop
        coinsOnDrop = 0;           // initializare bani drop
        speed = 0.5f;              // initializare viteza de miscare
        maxSpeed = 2.5f;           // initializare viteza maxima
        enemyHeight = 50;          // initializare inaltime inamic
        enemyWidth = 30;           // initializare latime inamic
        attackRadius = 40;         // initializare raza de atac
        isAttackingAnimation = 0;  // initializare animatie de atac
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
        if (toBeDeleted)
            return;  // Already dead

        health -= amount;
        // hit_enemy sound effect:
        SoundManager::getInstance().playSound("hit_enemy");  // Play hit sound

        if (health <= 0) {
            health = 0;  // Ensure health doesn't go negative
            toBeDeleted = true;

            // coinsOnDrop is multiplied with moneyMultiplier
            coinsOnDrop = (int)((float)(coinsOnDrop)*moneyMultiplier);  // Apply money multiplier to coins dropped

            // Drop XP and coins
            for (int i = 0; i < coinsOnDrop; ++i) {
                // Spawn coin at enemy's position
                spawnCoinAt(x, y);  // Function to spawn coin at enemy's position
            }
            for (int i = 0; i < xpOnDrop; ++i) {
                // Spawn XP item at enemy's position
                spawnXPAt(x, y);  // Function to spawn XP item at enemy's position
            }

            // death sound effect:
            SoundManager::getInstance().playSound("dead_enemy");  // Play death sound
        }
    }

    virtual void update(RenderWindow& window) {
        if (toBeDeleted)
            return;
    }

    virtual void draw(RenderWindow& window) {};

    ~Enemy() {
        // destructor
        // nu este necesar sa facem nimic aici, deoarece inamicul este gestionat de alte clase
    }
};

class EnemyGoblin : public Enemy {
private:
    EnemyBrain ai;  // AI for the goblin
public:
    EnemyGoblin(float x, float y, int health, int damage, bool isMelee) : Enemy(x, y, health, damage, isMelee) {
        xpOnDrop = 10 + rand() % 10;   // XP dropped when killed
        coinsOnDrop = 7;               // Coins dropped when killed
        damage = 5;                    // Damage dealt to player
        maxHealth = 20 + rand() % 10;  // Health of the goblin
        health = maxHealth;            // Set current health to max health
        attackCooldown = 30;           // Cooldown for attack in frames
        attackCooldownStart = 30;      // Cooldown for attack in frames
        isMelee = true;                // Goblin is a melee enemy
        vx = 0;                        // Initial velocity on x-axis
        vy = 0;                        // Initial velocity on y-axis
        animation = 0;                 // Initial animation frame
        toBeDeleted = false;           // Goblin is not dead
        isAttacking = false;           // Goblin is not attacking
        isAttackingAnimation = 0;      // Keep that sprite for 30 frames
        canAttack = true;              // Goblin can attack
        speed = 0.5f;
        maxSpeed = 2.5f;         // Maximum speed of the goblin
        this->x = x;             // Set goblin's x position
        this->y = y;             // Set goblin's y position
        this->vx = 0;            // Set goblin's x velocity
        this->vy = 0;            // Set goblin's y velocity
        this->enemyHeight = 50;  // Height of the goblin
        this->enemyWidth = 30;   // Width of the goblin
        attackRadius = 40;

        EnemyBrain ai;
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

        // individual size 82/128

        Texture& mobTexture = TextureManager::getInstance().find("Goblinset");

        Sprite sprite(mobTexture, IntRect({82 * 2, 82}, {82, 128}));  // Initialize with a valid texture

        // draw (centered) at x, y
        sprite.setPosition(Vector2f(82 / 2, 128 / 2));  // center the sprite
        // auto scale the sprite to 30x50 px
        sprite.setScale(Vector2f(0.365, 0.390));  // scale the sprite

        // draw
        if (isAttackingAnimation > 0) {
            // draw attack animation
            if (getX() < 200) {
                sprite.setTextureRect(IntRect({0, 0}, {82, 128}));  // set texture to attack
            } else {
                sprite.setTextureRect(IntRect({82, 0}, {82, 128}));  // set texture to attack mirrored
            }
        } else {
            // draw walk animation
            if (animation % 50 < 25) {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82 * 2, 0}, {82, 128}));  // set texture to walk1
                } else {
                    sprite.setTextureRect(IntRect({0, 128}, {82, 128}));  // set texture to walk1 mirrored
                }
            } else {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82, 128}, {82, 128}));  // set texture to walk2
                } else {
                    sprite.setTextureRect(IntRect({82 * 2, 128}, {82, 128}));  // set texture to walk2 mirrored
                }
            }
        }

        // set the scale to 30x50 px (can increase for bigger enemies)
        // scale set to equal 30x50 from 82x128
        sprite.setScale(Vector2f(0.365, 0.390));  // scale the sprite
        // set the position to x, y
        sprite.setPosition(Vector2f(getX() - 30.0f / 2, getY() - 50.0f / 2));  // center the sprite
        // draw the sprite
        window.draw(sprite);  // draw the sprite
        // update animation
        animation = (animation % 100) + 1;  // update animation
    }

    // update
    void update(RenderWindow& window) override {
        if (toBeDeleted)
            return;  // Already dead

        // update animation
        animation = (animation % 100) + 1;  // update animation
        if (isAttackingAnimation > 0)
            isAttackingAnimation--;  // update attack animation

        // update cooldown
        if (attackCooldown > 0) {
            attackCooldown--;
        } else {
            canAttack = true;
        }

        // move towards player
        if (getX() < 200) {
            vx += speed;  // move right
        } else {
            vx += -speed;  // move left
        }
        if (getY() < playerY) {
            vy += speed;  // move down
        } else {
            vy += -speed;  // move up
        }

        // limit
        if (vx > maxSpeed)
            vx = maxSpeed;  // limit speed
        if (vx < -maxSpeed)
            vx = -maxSpeed;  // limit speed
        if (vy > maxSpeed)
            vy = maxSpeed;  // limit speed
        if (vy < -maxSpeed)
            vy = -maxSpeed;  // limit speed

        // check distance to player
        float distToPlayer = distance(Vector2f(getX(), getY()), Vector2f(200, playerY));
        float angleToPlayer = atan2(playerY - getY(), 200 - getX());  // calculate angle to player
        Angle angle(radians(angleToPlayer));                          // convert to Angle object

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
        if (swordHitbox1.x > getX() - enemyWidth / 2 && swordHitbox1.x < getX() + enemyWidth / 2 && swordHitbox1.y > getY() - enemyHeight / 2 &&
            swordHitbox1.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(angleToPlayer);                // knockback in the opposite direction of the player
            vy += -7 * sin(angleToPlayer);                // knockback in the opposite direction of the player
        }
        if (swordHitbox2.x > getX() - enemyWidth / 2 && swordHitbox2.x < getX() + enemyWidth / 2 && swordHitbox2.y > getY() - enemyHeight / 2 &&
            swordHitbox2.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(angleToPlayer);                // knockback in the opposite direction of the player
            vy += -7 * sin(angleToPlayer);                // knockback in the opposite direction of the player
        }

        if (distToPlayer < attackRadius && canAttack) {  // if close enough to attack
            isAttacking = true;
            canAttack = false;
            attackCooldown = attackCooldownStart;  // reset cooldown
            if (isMelee) {
                playerTakeDamage(damage);  // deal damage to player
                // knockback player
                playerVx += -1.5 * cos(angleToPlayer);                 // knockback in the opposite direction of the player
                playerVy += -1.5 * sin(angleToPlayer);                 // knockback in the opposite direction of the player
                isAttackingAnimation = 30;                             // set attack animation
                SoundManager::getInstance().playSound("took_damage");  // Play hit sound for player
            }
        } else {
            isAttacking = false;
        }

        // apply velocity to position
        x += vx * 0.8f;  // apply velocity to x position
        y += vy * 0.8f;  // apply velocity to y position

        x += playerVx;  // apply player velocity to x position
        // y += playerVy; // apply player velocity to y position

        // ai movement
        angleToPlayer = atan2(playerY - y, 200 - x);  // calculate angle to player (from 0 to 360)
        float angleDeg = angleToPlayer * 180 / M_PI;
        angleDeg = fmod(angleDeg + 360, 360);  // normalize to [0, 360)
        auto aiResult = ai.result(playerHealth / playerMaxHealth, x, y, 200, playerY, angleDeg);
        // aiResult[0]: move towards (1) or away (0), aiResult[1]: angle in degrees
        bool moveTowardsPlayer = (aiResult[0] > 0.5);
        float moveAngle = aiResult[1] * M_PI / 180;  // convert to radians
        if (moveTowardsPlayer) {
            vx += speed * cos(moveAngle) * 0.9f;
            vy += speed * sin(moveAngle) * 0.9f;
        } else {
            // vx -= speed * cos(moveAngle) * 0.6f;
            // vy -= speed * sin(moveAngle) * 0.6f;
        }

        // keep it within bounds
        // keep goblin within bounds
        if (x < 0) {
            x = 0;
            vx += 1;
        }
        if (x > 800) {
            x = 800;
            vx -= 1;
        }
        if (y < 0) {
            y = 0;
            vy += 1;
        }
        if (y > 600) {
            y = 600;
            vy -= 1;
        }
    }

    float getEnemyWidth() const { return enemyWidth; }                                       // getter for enemy width
    float getEnemyHeight() const { return enemyHeight; }                                     // getter for enemy height
    float getAttackRadius() const { return attackRadius; }                                   // getter for attack radius
    int getXpOnDrop() const { return xpOnDrop; }                                             // getter for XP on drop
    int getCoinsOnDrop() const { return coinsOnDrop; }                                       // getter for coins on drop
    int getDamage() const { return damage; }                                                 // getter for damage dealt by goblin
    bool isMeleeEnemy() const { return isMelee; }                                            // getter for melee enemy
    bool isToBeDeleted() const { return toBeDeleted; }                                       // getter for toBeDeleted flag
    bool getIsAttacking() const { return isAttacking; }                                      // getter for isAttacking flag
    int getIsAttackingAnimation() const { return isAttackingAnimation; }                     // getter for isAttackingAnimation
    void setIsAttackingAnimation(int value) { isAttackingAnimation = value; }                // setter for isAttackingAnimation
    void setToBeDeleted(bool deleted) { toBeDeleted = deleted; }                             // setter for toBeDeleted flag
    void setHealth(int health) { this->health = health; }                                    // setter for health
    void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }                        // setter for max health
    void setDamage(int damage) { this->damage = damage; }                                    // setter for damage
    void setXpOnDrop(int xp) { xpOnDrop = xp; }                                              // setter for XP on drop
    void setCoinsOnDrop(int coins) { coinsOnDrop = coins; }                                  // setter for coins on drop
    void setIsMelee(bool isMelee) { this->isMelee = isMelee; }                               // setter for isMelee flag
    void setSpeed(float speed) { this->speed = speed; }                                      // setter for speed
    void setMaxSpeed(float maxSpeed) { this->maxSpeed = maxSpeed; }                          // setter for max speed
    void setAttackRadius(float radius) { attackRadius = radius; }                            // setter for attack radius
    void setEnemyWidth(float width) { enemyWidth = width; }                                  // setter for enemy width
    void setEnemyHeight(float height) { enemyHeight = height; }                              // setter for enemy height
    void setAttackCooldown(int cooldown) { attackCooldown = cooldown; }                      // setter for attack cooldown
    void setAttackCooldownStart(int cooldownStart) { attackCooldownStart = cooldownStart; }  // setter for attack cooldown start
    void setCanAttack(bool canAttack) { this->canAttack = canAttack; }                       // setter for canAttack flag
    void setVx(float vx) { this->vx = vx; }                                                  // setter for x velocity
    void setVy(float vy) { this->vy = vy; }                                                  // setter for y velocity
    void setX(float x) { this->x = x; }                                                      // setter for x position
    void setY(float y) { this->y = y; }                                                      // setter for y position
    void setAnimation(int animation) { this->animation = animation; }                        // setter for animation
    void setIsAttacking(bool isAttacking) { this->isAttacking = isAttacking; }               // setter for isAttacking flag
    void setIsMeleeEnemy(bool isMelee) { this->isMelee = isMelee; }                          // setter for isMelee flag
    void setEnemyBrain(const EnemyBrain& ai) { this->ai = ai; }                              // setter for AI
    EnemyBrain getEnemyBrain() const { return ai; }                                          // getter for AI

    ~EnemyGoblin() {
        // destructor
    }
};

class EnemyBaphomet : public Enemy {
private:
    EnemyBrain ai;  // AI for Baphomet
public:
    EnemyBaphomet(float x, float y) : Enemy(x, y, /*health*/ 0, /*damage*/ 0, /*isMelee*/ true) {
        // Baphomet is twice as powerful and a bit faster than goblin
        xpOnDrop = 20 + rand() % 20;   // 20-39 XP
        coinsOnDrop = 14;              // twice goblin
        damage = 12;                   // twice goblin's damage
        maxHealth = 60 + rand() % 20;  // 60-79 Health
        health = maxHealth;
        attackCooldownStart = attackCooldown = 20;  // faster attacks
        isMelee = true;
        speed = 0.8f;              // increased speed
        maxSpeed = 4.0f;           // higher top speed
        animation = rand() % 100;  // random animation start
        toBeDeleted = false;       // not dead
        canAttack = true;
        isAttacking = false;
        isAttackingAnimation = 0;
        enemyWidth = 50;  // larger hitbox
        enemyHeight = 70;
        attackRadius = 60;  // bigger reach
        this->x = x;
        this->y = y;
        vx = vy = 0;
        EnemyBrain ai = EnemyBrain();  // ai
    }

    void draw(RenderWindow& window) override {
        // Load or retrieve Baphomet textures
        // individual size 82/128

        Texture& mobTexture = TextureManager::getInstance().find("Baphometset");

        Sprite sprite(mobTexture, IntRect({82 * 2, 82}, {82, 128}));  // Initialize with a valid texture

        // draw (centered) at x, y
        sprite.setPosition(Vector2f(82 / 2, 128 / 2));  // center the sprite
        // auto scale the sprite to 30x50 px
        sprite.setScale(Vector2f(0.365, 0.390));  // scale the sprite

        // draw
        if (isAttackingAnimation > 0) {
            // draw attack animation
            if (getX() < 200) {
                sprite.setTextureRect(IntRect({0, 0}, {82, 128}));  // set texture to attack
            } else {
                sprite.setTextureRect(IntRect({82, 0}, {82, 128}));  // set texture to attack mirrored
            }
        } else {
            // draw walk animation
            if (animation % 50 < 25) {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82 * 2, 0}, {82, 128}));  // set texture to walk1
                } else {
                    sprite.setTextureRect(IntRect({0, 128}, {82, 128}));  // set texture to walk1 mirrored
                }
            } else {
                if (getX() < 200) {
                    sprite.setTextureRect(IntRect({82, 128}, {82, 128}));  // set texture to walk2
                } else {
                    sprite.setTextureRect(IntRect({82 * 2, 128}, {82, 128}));  // set texture to walk2 mirrored
                }
            }
        }

        // set the scale to 50x70 px (can increase for bigger enemies)
        // scale set to equal 50x70 from 82x128
        sprite.setScale(Vector2f(0.609, 0.546));  // scale the sprite
        // set the position to x, y
        sprite.setPosition(Vector2f(getX() - 30.0f / 2, getY() - 50.0f / 2));  // center the sprite
        // draw the sprite
        window.draw(sprite);  // draw the sprite
        // update animation
        animation = (animation % 100) + 1;
        if (isAttackingAnimation > 0)
            --isAttackingAnimation;
    }

    void update(RenderWindow& window) override {
        if (toBeDeleted)
            return;

        // cooldowns
        if (attackCooldown > 0)
            --attackCooldown;
        else
            canAttack = true;

        // movement: home in
        float dx = 200 - getX();
        float dy = playerY - getY();
        float dist = std::sqrt(dx * dx + dy * dy);
        if (getX() < 200)
            vx += speed;
        else
            vx -= speed;
        if (getY() < playerY)
            vy += speed;
        else
            vy -= speed;

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
            playerTakeDamage(damage);  // deal damage to player
            // knockback player
            playerVx += -3 * cos(atan2(dy, dx));  // knockback in the opposite direction of the player
            playerVy += -3 * sin(atan2(dy, dx));  // knockback in the opposite direction of the player
            SoundManager::getInstance().playSound("took_damage");
        } else {
            isAttacking = false;
        }

        // take damage logic from sword
        Angle towardsPlayerAngle = sf::radians(atan2(dy, dx));  // angle towards player
        // check if sword hitboxes are inside enemy hitbox
        if (swordHitbox1.x > getX() - enemyWidth / 2 && swordHitbox1.x < getX() + enemyWidth / 2 && swordHitbox1.y > getY() - enemyHeight / 2 &&
            swordHitbox1.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
        }
        if (swordHitbox2.x > getX() - enemyWidth / 2 && swordHitbox2.x < getX() + enemyWidth / 2 && swordHitbox2.y > getY() - enemyHeight / 2 &&
            swordHitbox2.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
        }

        // apply movement
        x += vx * 0.8f + playerVx;
        y += vy * 0.8f;

        // move based on AI
        auto angleToPlayer = atan2(playerY - y, 200 - x);  // calculate angle to player (from 0 to 360)
        angleToPlayer *= 180 / M_PI;                       // convert to degrees
        angleToPlayer = fmod(angleToPlayer + 360, 360);    // normalize to [0, 360)
        auto aiResult = ai.result(playerHealth / playerMaxHealth, x, y, 200, playerY, angleToPlayer);
        // cout << "querying: " << playerHealth / playerMaxHealth << ", " << x << ", " << y << ", " << 200 << ", " << playerY << ", " << angleToPlayer << "
        // result: " << aiResult[0] << ", " << aiResult[1] << endl; o sa rezulte: aiResult[0] = 1 sau 0, 1 = te misti spre player, 0, nu si aiResult[1] =
        // unghiul la care sa se miste
        auto moveTowardsPlayer = (aiResult[0] > 0.5);
        auto moveAngle = aiResult[1] * M_PI / 180;  // convert to radians
        if (moveTowardsPlayer) {
            // move towards player
            vx += speed * cos(moveAngle);
            vy += speed * sin(moveAngle);
        } else {
            // move away from player
            vx -= speed * cos(moveAngle);
            vy -= speed * sin(moveAngle);
        }

        // if baphomet out of bounds, push it back
        if (x < 0)
            x = 0, vx += 1;
        if (x > 800)
            x = 800, vx -= 1;
        if (y < 0)
            y = 0, vy += 1;
        if (y > 600)
            y = 600, vy -= 1;
    }

    float getEnemyWidth() const { return enemyWidth; }                                       // getter for enemy width
    float getEnemyHeight() const { return enemyHeight; }                                     // getter for enemy height
    float getAttackRadius() const { return attackRadius; }                                   // getter for attack radius
    int getXpOnDrop() const { return xpOnDrop; }                                             // getter for XP on drop
    int getCoinsOnDrop() const { return coinsOnDrop; }                                       // getter for coins on drop
    int getDamage() const { return damage; }                                                 // getter for damage dealt by baphomet
    bool isMeleeEnemy() const { return isMelee; }                                            // getter for melee enemy
    bool isToBeDeleted() const { return toBeDeleted; }                                       // getter for toBeDeleted flag
    bool getIsAttacking() const { return isAttacking; }                                      // getter for isAttacking flag
    int getIsAttackingAnimation() const { return isAttackingAnimation; }                     // getter for isAttackingAnimation
    void setIsAttackingAnimation(int value) { isAttackingAnimation = value; }                // setter for isAttackingAnimation
    void setToBeDeleted(bool deleted) { toBeDeleted = deleted; }                             // setter for toBeDeleted flag
    void setHealth(int health) { this->health = health; }                                    // setter for health
    void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }                        // setter for max health
    void setDamage(int damage) { this->damage = damage; }                                    // setter for damage
    void setXpOnDrop(int xp) { xpOnDrop = xp; }                                              // setter for XP on drop
    void setCoinsOnDrop(int coins) { coinsOnDrop = coins; }                                  // setter for coins on drop
    void setIsMelee(bool isMelee) { this->isMelee = isMelee; }                               // setter for isMelee flag
    void setSpeed(float speed) { this->speed = speed; }                                      // setter for speed
    void setMaxSpeed(float maxSpeed) { this->maxSpeed = maxSpeed; }                          // setter for max speed
    void setAttackRadius(float radius) { attackRadius = radius; }                            // setter for attack radius
    void setEnemyWidth(float width) { enemyWidth = width; }                                  // setter for enemy width
    void setEnemyHeight(float height) { enemyHeight = height; }                              // setter for enemy height
    void setAttackCooldown(int cooldown) { attackCooldown = cooldown; }                      // setter for attack cooldown
    void setAttackCooldownStart(int cooldownStart) { attackCooldownStart = cooldownStart; }  // setter for attack cooldown start
    void setCanAttack(bool canAttack) { this->canAttack = canAttack; }                       // setter for canAttack flag
    void setVx(float vx) { this->vx = vx; }                                                  // setter for x velocity
    void setVy(float vy) { this->vy = vy; }                                                  // setter for y velocity
    void setX(float x) { this->x = x; }                                                      // setter for x position
    void setY(float y) { this->y = y; }                                                      // setter for y position
    void setAnimation(int animation) { this->animation = animation; }                        // setter for animation
    void setIsAttacking(bool isAttacking) { this->isAttacking = isAttacking; }               // setter for isAttacking flag
    void setIsMeleeEnemy(bool isMelee) { this->isMelee = isMelee; }                          // setter for isMelee flag
    void setEnemyBrain(const EnemyBrain& ai) { this->ai = ai; }                              // setter for AI
    EnemyBrain getEnemyBrain() const { return ai; }                                          // getter for AI

    ~EnemyBaphomet() {
        // destructor
    }
};

class EnemyReaper : public Enemy {
private:
    EnemyBrain ai;  // AI for Reaper
public:
    EnemyReaper(float x, float y) : Enemy(x, y, /*health*/ 0, /*damage*/ 0, /*isMelee*/ true) {
        // Reaper is twice as powerful and a bit faster than goblin
        xpOnDrop = 20 + rand() % 50;   //
        coinsOnDrop = 30;              // EVEN MOREEE
        damage = 35;                   // EVEN MOREEE
        maxHealth = 80 + rand() % 20;  // 60-79 healths
        health = maxHealth;
        attackCooldownStart = attackCooldown = 20;  // faster attacks
        isMelee = true;
        speed = 0.8f;              // increased speed
        maxSpeed = 5.0f;           // higher top speed
        animation = rand() % 100;  // random animation start
        toBeDeleted = false;       // not dead
        canAttack = true;
        isAttacking = false;
        isAttackingAnimation = 0;
        enemyWidth = 52;  // larger hitbox
        enemyHeight = 73;
        attackRadius = 70;  // bigger reach
        this->x = x;
        this->y = y;
        vx = vy = 0;
        EnemyBrain ai = EnemyBrain();  // ai
    }

    void draw(RenderWindow& window) override {
        // Load or retrieve Reaper textures
        // individual size 82/128

        Texture& mobTextureWalk1 = TextureManager::getInstance().find("enemy_reaper_walk_1");
        Texture& mobTextureWalk2 = TextureManager::getInstance().find("enemy_reaper_walk_2");
        Texture& mobTextureAttack = TextureManager::getInstance().find("enemy_reaper_attack");
        // mirrors
        Texture& mobTextureWalk1Mirror = TextureManager::getInstance().find("enemy_reaper_walk_1_mirror");
        Texture& mobTextureWalk2Mirror = TextureManager::getInstance().find("enemy_reaper_walk_2_mirror");
        Texture& mobTextureAttackMirror = TextureManager::getInstance().find("enemy_reaper_attack_mirror");

        Sprite sprite = Sprite(mobTextureWalk1);

        // draw (centered) at x, y
        sprite.setPosition(Vector2f(this->enemyWidth / 2, this->enemyHeight / 2));  // center the sprite
        // auto scale the sprite to 62x80 px
        sprite.setScale(Vector2f(62.0f / mobTextureWalk1.getSize().x, 80.0f / mobTextureWalk1.getSize().y));  // scale the sprite

        // draw
        if (isAttackingAnimation > 0) {
            // draw attack animation
            if (getX() < 200) {
                sprite.setTexture(mobTextureAttack);  // set texture to attack
            } else {
                sprite.setTexture(mobTextureAttackMirror);  // set texture to attack mirrored
            }
        } else {
            // draw walk animation
            if (animation % 50 < 25) {
                if (getX() < 200) {
                    // walk1
                    sprite.setTexture(mobTextureWalk1);  // set texture to walk1
                } else {
                    // walk1 mirrored
                    sprite.setTexture(mobTextureWalk1Mirror);  // set texture to walk1 mirrored
                }
            } else {
                if (getX() < 200) {
                    // walk2
                    sprite.setTexture(mobTextureWalk2);  // set texture to walk2
                } else {
                    // walk2 mirrored
                    sprite.setTexture(mobTextureWalk2Mirror);  // set texture to walk2 mirrored
                }
            }
        }

        // set the position to x, y
        sprite.setPosition(Vector2f(getX() - this->enemyWidth / 2, getY() - this->enemyHeight / 2));  // center the sprite
        // draw the sprite
        window.draw(sprite);  // draw the sprite
        // update animation
        animation = (animation % 100) + 1;
        if (isAttackingAnimation > 0)
            --isAttackingAnimation;
    }

    void update(RenderWindow& window) override {
        if (toBeDeleted)
            return;

        // cooldowns
        if (attackCooldown > 0)
            --attackCooldown;
        else
            canAttack = true;

        // movement: home in
        float dx = 200 - getX();
        float dy = playerY - getY();
        float dist = std::sqrt(dx * dx + dy * dy);
        if (getX() < 200)
            vx += speed;
        else
            vx -= speed;
        if (getY() < playerY)
            vy += speed;
        else
            vy -= speed;

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
            playerTakeDamage(damage);  // deal damage to player
            // knockback player
            playerVx += -4 * cos(atan2(dy, dx));  // knockback in the opposite direction of the player
            playerVy += -4 * sin(atan2(dy, dx));  // knockback in the opposite direction of the player
            SoundManager::getInstance().playSound("took_damage");
        } else {
            isAttacking = false;
        }

        // take damage logic from sword
        Angle towardsPlayerAngle = sf::radians(atan2(dy, dx));  // angle towards player
        // check if sword hitboxes are inside enemy hitbox
        if (swordHitbox1.x > getX() - enemyWidth / 2 && swordHitbox1.x < getX() + enemyWidth / 2 && swordHitbox1.y > getY() - enemyHeight / 2 &&
            swordHitbox1.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
        }
        if (swordHitbox2.x > getX() - enemyWidth / 2 && swordHitbox2.x < getX() + enemyWidth / 2 && swordHitbox2.y > getY() - enemyHeight / 2 &&
            swordHitbox2.y < getY() + enemyHeight / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage);  // take damage from player
            vx += -7 * cos(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
            vy += -7 * sin(towardsPlayerAngle.asRadians());                  // knockback in the opposite direction of the player
        }

        // apply movement
        x += vx * 0.8f + playerVx;
        y += vy * 0.8f;

        // move based on AI
        auto angleToPlayer = atan2(playerY - y, 200 - x);  // calculate angle to player (from 0 to 360)
        angleToPlayer *= 180 / M_PI;                       // convert to degrees
        angleToPlayer = fmod(angleToPlayer + 360, 360);    // normalize to [0, 360)
        auto aiResult = ai.result(playerHealth / playerMaxHealth, x, y, 200, playerY, angleToPlayer);
        // cout << "querying: " << playerHealth / playerMaxHealth << ", " << x << ", " << y << ", " << 200 << ", " << playerY << ", " << angleToPlayer << "
        // result: " << aiResult[0] << ", " << aiResult[1] << endl; o sa rezulte: aiResult[0] = 1 sau 0, 1 = te misti spre player, 0, nu si aiResult[1] =
        // unghiul la care sa se miste
        auto moveTowardsPlayer = (aiResult[0] > 0.5);
        auto moveAngle = aiResult[1] * M_PI / 180;  // convert to radians
        if (moveTowardsPlayer) {
            // move towards player
            vx += speed * cos(moveAngle) * 1.1f;
            vy += speed * sin(moveAngle) * 1.1f;
        } else {
            // move away from player
            vx -= speed * cos(moveAngle) * 1.1f;
            vy -= speed * sin(moveAngle) * 1.1f;
        }

        // if Reaper out of bounds, push it back
        if (x < 0)
            x = 0, vx += 1;
        if (x > 800)
            x = 800, vx -= 1;
        if (y < 0)
            y = 0, vy += 1;
        if (y > 600)
            y = 600, vy -= 1;
    }

    float getEnemyWidth() const { return enemyWidth; }                                       // getter for enemy width
    float getEnemyHeight() const { return enemyHeight; }                                     // getter for enemy height
    float getAttackRadius() const { return attackRadius; }                                   // getter for attack radius
    int getXpOnDrop() const { return xpOnDrop; }                                             // getter for XP on drop
    int getCoinsOnDrop() const { return coinsOnDrop; }                                       // getter for coins on drop
    int getDamage() const { return damage; }                                                 // getter for damage dealt by Reaper
    bool isMeleeEnemy() const { return isMelee; }                                            // getter for melee enemy
    bool isToBeDeleted() const { return toBeDeleted; }                                       // getter for toBeDeleted flag
    bool getIsAttacking() const { return isAttacking; }                                      // getter for isAttacking flag
    int getIsAttackingAnimation() const { return isAttackingAnimation; }                     // getter for isAttackingAnimation
    void setIsAttackingAnimation(int value) { isAttackingAnimation = value; }                // setter for isAttackingAnimation
    void setToBeDeleted(bool deleted) { toBeDeleted = deleted; }                             // setter for toBeDeleted flag
    void setHealth(int health) { this->health = health; }                                    // setter for health
    void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }                        // setter for max health
    void setDamage(int damage) { this->damage = damage; }                                    // setter for damage
    void setXpOnDrop(int xp) { xpOnDrop = xp; }                                              // setter for XP on drop
    void setCoinsOnDrop(int coins) { coinsOnDrop = coins; }                                  // setter for coins on drop
    void setIsMelee(bool isMelee) { this->isMelee = isMelee; }                               // setter for isMelee flag
    void setSpeed(float speed) { this->speed = speed; }                                      // setter for speed
    void setMaxSpeed(float maxSpeed) { this->maxSpeed = maxSpeed; }                          // setter for max speed
    void setAttackRadius(float radius) { attackRadius = radius; }                            // setter for attack radius
    void setEnemyWidth(float width) { enemyWidth = width; }                                  // setter for enemy width
    void setEnemyHeight(float height) { enemyHeight = height; }                              // setter for enemy height
    void setAttackCooldown(int cooldown) { attackCooldown = cooldown; }                      // setter for attack cooldown
    void setAttackCooldownStart(int cooldownStart) { attackCooldownStart = cooldownStart; }  // setter for attack cooldown start
    void setCanAttack(bool canAttack) { this->canAttack = canAttack; }                       // setter for canAttack flag
    void setVx(float vx) { this->vx = vx; }                                                  // setter for x velocity
    void setVy(float vy) { this->vy = vy; }                                                  // setter for y velocity
    void setX(float x) { this->x = x; }                                                      // setter for x position
    void setY(float y) { this->y = y; }                                                      // setter for y position
    void setAnimation(int animation) { this->animation = animation; }                        // setter for animation
    void setIsAttacking(bool isAttacking) { this->isAttacking = isAttacking; }               // setter for isAttacking flag
    void setIsMeleeEnemy(bool isMelee) { this->isMelee = isMelee; }                          // setter for isMelee flag
    void setEnemyBrain(const EnemyBrain& ai) { this->ai = ai; }                              // setter for AI
    EnemyBrain getEnemyBrain() const { return ai; }                                          // getter for AI

    ~EnemyReaper() {
        // destructor
    }
};

// clasa de bani
class Coin {
private:
    float x, y;                // coordonatele monedei
    int animation;             // animatia monedei (de la 1 la 100)
    float vx, vy;              // viteza pe axa x si y (nu e folosita inca)
    bool toBeDeleted = false;  // flag pentru autodistrugere

public:
    // constructor
    Coin(float x, float y) : x(x), y(y) {
        animation = rand() % 100 + 1;  // genereaza o animatie aleatoare intre 1 si 100
        vx = rand_uniform(-2, 2);      // genereaza o viteza aleatoare pe axa x
        vy = rand_uniform(-2, 2);      // genereaza o viteza aleatoare pe axa y
        toBeDeleted = false;           // seteaza flag-ul de autodistrugere la false
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
            vx = -vx;  // inverseaza viteza pe axa x
            vy = -vy;  // inverseaza viteza pe axa y
        }

        if (x < 0)
            x = 0, vx += 1;
        if (x > 800)
            x = 800, vx -= 1;
        if (y < 0)
            y = 0, vy += 1;
        if (y > 600)
            y = 600, vy -= 1;

        // daca distanta catre player < 50, go to player
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 50) {
            vx = (200 - x) / 10;      // seteaza viteza pe axa x
            vy = (playerY - y) / 10;  // seteaza viteza pe axa y
        }

        // daca distanta catre player < 10, da bani si autodistrugere
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 10) {
            balance += 1;                                          // adauga 1 la balanta
            toBeDeleted = true;                                    // seteaza flag-ul de autodistrugere la true
            SoundManager::getInstance().playSound("pickup_coin");  // reda sunetul de pickup
            cout << "DEBUG: played sound cuz x=" << x << " y=" << y << ", dist = " << distance(Vector2f(x, y), Vector2f(200, playerY)) << endl;
        }

        // frictiune
        vx *= 0.96;  // aplica frictiunea pe axa x
        vy *= 0.96;  // aplica frictiunea pe axa y

        // se misca in functie de player
        x += playerVx;
        // y += playerVy;
    }

    // metoda pentru desenarea monedei
    void draw(RenderWindow& window) {
        // selecteaza textura in functie de animatie
        // Texture& texture = (animation <= 50)
        //    ? TextureManager::getInstance().find("coin1")
        //    : TextureManager::getInstance().find("coin2");

        Texture& texture = TextureManager::getInstance().find("Coinset");

        Sprite sprite(texture);
        if (animation <= 50)
            sprite.setTextureRect(IntRect({0, 0}, {32, 32}));
        else
            sprite.setTextureRect(IntRect({32, 0}, {32, 32}));

        sprite.setPosition(Vector2f(x, y));  // seteaza pozitia sprite-ului
        // schimba factorul de scaling pentru textura mai mare sau mica
        sprite.setScale(Vector2f(0.35, 0.35));  // seteaza scalarea sprite-ului
        window.draw(sprite);                    // deseneaza sprite-ul

        // actualizeaza animatia
        animation = (animation % 100) + 1;
    }

    // getters setters deletion
    bool isToBeDeleted() const { return toBeDeleted; }                          // getter pentru flag-ul de autodistrugere
    void setToBeDeleted(bool toBeDeleted) { this->toBeDeleted = toBeDeleted; }  // setter pentru flag-ul de autodistrugere

    ~Coin() {
        // destructor
    }
};

// clasa de decoratiuni
class Decoration {
private:
    float x, y;                // coordonatele decoratiunii
    bool toBeDeleted = false;  // flag pentru autodistrugere
public:
    sf::Sprite sprite;  // sprite-ul decoratiunii
    // constructor
    Decoration(float x, float y, const std::string& textureName) : x(x), y(y), sprite(TextureManager::getInstance().find(textureName)) {
        sprite.setPosition(Vector2f(x, y));
    }

    // metoda pentru desenarea decoratiunii
    void draw(RenderWindow& window) {  // and update() tot in aceelasi
        if (!toBeDeleted) {
            // set its position
            sprite.setPosition(Vector2f(x, y));  // seteaza pozitia sprite-ului
            // set the width to 20 and maintain the aspect ratio for the height
            float scaleFactor = 20.0f / sprite.getTexture().getSize().x;
            sprite.setScale(Vector2f(scaleFactor, scaleFactor));  // seteaza scalarea sprite-ului

            // draw the sprite
            window.draw(sprite);
        }

        this->x += playerVx;  // se misca in functie de player

        if (x < -600) {
            toBeDeleted = true;  // autodistruge decoratiunea daca iese din ecran
            cout << "DEBUG: Decoration (" << x << ", " << y << ") is set for deletion (e departe)" << endl;
        }
    }

    // metoda pentru setarea pozitiei
    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
        sprite.setPosition(Vector2f(x, y));
    }

    // metoda pentru autodistrugere
    void setForDelete() { toBeDeleted = true; }

    // metoda pentru verificarea autodistrugerii
    bool isToBeDeleted() const { return toBeDeleted; }

    // metoda pentru setarea scalei
    void setScale(float scaleX, float scaleY) { sprite.setScale(Vector2f(scaleX, scaleY)); }

    // metoda pentru setarea rotatiei
    void setRotation(float angle) { sprite.setRotation(sf::degrees(angle)); }

    // metoda pentru setarea texturii
    void setTexture(const std::string& textureName) { sprite.setTexture(TextureManager::getInstance().find(textureName)); }

    ~Decoration() {
        // destructor
    }
};

// clasa de exp
class ExpOrb {
private:
    float x, y;                                 // coordonatele orb-ului
    int animation;                              // animatia orb-ului (de la 1 la 100)
    float vx, vy;                               // viteza pe axa x si y
    bool toBeDeleted = false;                   // flag pentru autodistrugere
    Angle towardsPlayerAngle = sf::radians(0);  // unghiul catre player
public:
    // constructor
    ExpOrb(float x, float y) : x(x), y(y) {
        animation = rand() % 100 + 1;  // genereaza o animatie aleatoare intre 1 si 100
        vx = rand_uniform(-5, 5);      // genereaza o viteza aleatoare pe axa x
        vy = rand_uniform(-5, 5);      // genereaza o viteza aleatoare pe axa y
        toBeDeleted = false;           // seteaza flag-ul de autodistrugere la false
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
            float angle = atan2(dy, dx);              // calculeaza unghiul catre player
            towardsPlayerAngle = sf::radians(angle);  // seteaza unghiul catre player folosind sfml::Angle
            vx += cos(angle) / 2;                     // seteaza viteza pe axa x
            vy += sin(angle) / 2;                     // seteaza viteza pe axa y
        }
        // limit viteza orb-ului
        if (vx > 5)
            vx = 5;
        if (vx < -5)
            vx = -5;
        if (vy > 5)
            vy = 5;
        if (vy < -5)
            vy = -5;

        // actualizeaza pozitia orb-ului
        x += vx;
        y += vy;

        // player
        x += playerVx;
        // y += playerVy;

        // daca distanta catre player < 10, creste xp level si autodistrugere
        if (distance(Vector2f(x, y), Vector2f(200, playerY)) < 10) {
            // Creste nivelul de experienta
            xp += rand() % 10 + 1;                                    // adauga un numar aleatoriu de xp (1-10)
            cout << "DEBUG: picked up exp orb, xp = " << xp << endl;  // afiseaza xp-ul
            toBeDeleted = true;                                       // seteaza flag-ul de autodistrugere la true
            SoundManager::getInstance().playSound("pickup_exp");      // reda sunetul de pickup
        }

        // frictiune mai mare decat Coin
        vx *= 0.96;  // aplica frictiunea pe axa x
        vy *= 0.96;  // aplica frictiunea pe axa y
    }

    // metoda pentru desenarea orb-ului
    void draw(RenderWindow& window) {
        // selecteaza textura in functie de animatie
        Texture& texture = TextureManager::getInstance().find("Expset");

        Sprite sprite(texture);
        if (animation <= 50)
            sprite.setTextureRect(IntRect({0, 0}, {32, 32}));
        else
            sprite.setTextureRect(IntRect({32, 0}, {32, 32}));

        sprite.setPosition(Vector2f(x, y));     // seteaza pozitia sprite-ului
        sprite.setScale(Vector2f(0.25, 0.25));  // seteaza scalarea sprite-ului
        window.draw(sprite);                    // deseneaza sprite-ul

        // actualizeaza animatia
        animation = (animation % 100) + 1;
    }

    // getters setters deletion
    bool isToBeDeleted() const { return toBeDeleted; }                          // getter pentru flag-ul de autodistrugere
    void setToBeDeleted(bool toBeDeleted) { this->toBeDeleted = toBeDeleted; }  // setter pentru flag-ul de autodistrugere
};

class SkeletronPart {
public:
    Vector2f position;
    Vector2f velocity;
    float followSpeed;
    float delayFactor;
    Sprite sprite;
    float spriteSize;
    Angle rotation;
    bool rotate;
    Angle desiredAngle;
    std::string normalTexture;
    std::string attackTexture;

    int health = 0;
    int maxHealth = 200;
    bool active = true;
    bool attacking = false;
    float baseX;

    SkeletronPart(float x,
                float y,
                float followSpeed,
                float delayFactor,
                string textureName,
                string attackTextureName,
                float spriteSize,
                bool rotate,
                bool flipped = false,
                float desiredRotation = 0)
            : position(x, y),
            baseX(x),
            velocity(0, 0),
            followSpeed(followSpeed * 4),
            delayFactor(delayFactor),
            sprite(Sprite(TextureManager::getInstance().find(textureName))),
            rotate(rotate) {
        this->spriteSize = spriteSize;
        sprite.setOrigin(Vector2f(spriteSize / 2.f, spriteSize / 2.f));
        rotation = sf::degrees(0);
        desiredAngle = sf::degrees(desiredRotation);

        normalTexture = textureName;
        attackTexture = attackTextureName;

        health = maxHealth;

        if (flipped) {
            sprite.scale(sf::Vector2f(-1.f, 1.f));
        }
    }

    void loadTexture(const std::string& texName) {
        if (texName == "") {
            return;
        }
        sprite.setTexture(TextureManager::getInstance().find(texName));
        sprite.setOrigin(Vector2f(spriteSize / 2.f, spriteSize / 2.f));
        sprite.setPosition(position);
    }

    void follow(Vector2f target, float dt, bool attacking = false) {
        this->attacking = attacking;

        if (attacking) {
            loadTexture(attackTexture);
        } else {
            loadTexture(normalTexture);
        }

        Vector2f direction = target - position;
        float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        float attackBoost = attacking ? 3.0f : 1.0f;

        Vector2f directionNormalised = direction;

        if (dist > 0)
            directionNormalised /= dist;  // Normalize

        if (dist <= 1.1f) {
            position = target;
        }

        if (dist > 1.f) {
            // Calculate speed based on distance (further = faster)
            float speedFactor = dist * (attacking ? 1.f : 0.5f);
            float currentSpeed = (followSpeed * attackBoost) + speedFactor;

            if (currentSpeed > 200) {
                currentSpeed = 200;  // speed limit
            }

            velocity = directionNormalised * currentSpeed;

            position += velocity * dt * (attacking ? delayFactor * 0.5f : delayFactor);

            dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (dist < 2.f) {
                position = target;
            }
        }

        // Update rotation
        if (dist > 1.f && attacking && rotate) {
            rotation = sf::degrees((std::atan2(directionNormalised.y, directionNormalised.x) * 180.f / 3.14159f) - 90.f);
            sprite.setRotation(rotation);
        } else if (rotate && attacking == false) {
            sprite.setRotation(desiredAngle);
        }
    }

    bool isTouchingPlayer() const {
        return distance(position, Vector2f(200, playerY)) < (spriteSize / 2.f);
    }

    bool takeDamage(int amount) {
        if (!active)
            return false;

        health -= amount;
        if (health <= 0) {
            health = 0;
            active = false;
            return true;  // Part destroyed
        }
        return false;
    }

    void applyWorldMovement() {
        // Update base position with world movement
        baseX += playerVx;

        // Apply world movement to current position
        position.x += playerVx;
    }
    void draw(RenderWindow& window) {
        if (!active)
            return;

        sprite.setPosition(position);
        window.draw(sprite);
    }
};

class EnemySkeletron : public Enemy {
public:
    SkeletronPart head;
    SkeletronPart upperArmLeft;
    SkeletronPart lowerArmLeft;
    SkeletronPart palmLeft;
    SkeletronPart upperArmRight;
    SkeletronPart lowerArmRight;
    SkeletronPart palmRight;

    Vector2f position = Vector2f(0, 100);
    Vector2f worldPosition;

    // Arm connection points (relative to head)
    const Vector2f LEFT_ARM_BASE_OFFSET = Vector2f(-80, 25);
    const Vector2f RIGHT_ARM_BASE_OFFSET = Vector2f(80, 25);
    const Vector2f FOREARM_OFFSET = Vector2f(0, 40);
    const Vector2f PALM_OFFSET = Vector2f(32, 37);

    struct AttackState {
        enum State { IDLE, ATTACKING, RETURNING };

        State state = IDLE;
        float timer = 0.f;
        const float ATTACK_DURATION = 70.8f;
        const float RETURN_DURATION = 10.2f;
        const float COOLDOWN_MIN = 300.0f;
        const float COOLDOWN_MAX = 600.0f;
        float currentCooldown = 0.f;
        Vector2f originalPosition;
        Vector2f attackTarget;
        bool hasDamaged = false;
    };

    AttackState leftHandAttack;
    AttackState rightHandAttack;
    AttackState headAttack;

    // Special attack when hands are destroyed
    bool headCanAttack = false;
    Vector2f headNeutralPosition;
    float headNeutralPositionVx = 0.0f;  // Neutral position for head, used for AI movement
    float headNeutralPositionVy = 0.0f;  // Neutral position for head, used for AI movement

    EnemyBrain ai;  // AI for Skeletron
public:
    EnemySkeletron(float x, float y)
        : Enemy(x, y, getX(), getY(), false),
            worldPosition(x, y),
            // float x, float y, float followSpeed,
            // float delayFactor, string textureName,
            // string attackTextureName, float spriteSize,
            // bool rotate, bool flipped = false, float desiredRotation = 0
            head(x, y, 4.0f, 0.04f, "Boss_Head", "", 128.f, false),
            upperArmLeft(x - 50, y + 50, 5.0f, 0.11f, "Boss_Arm_UpperVertical", "", 64.f, true, false, 45.f),
            lowerArmLeft(x - 80, y + 100, 4.5f, 0.085f, "Boss_Arm_Vertical", "", 64.f, true, false, 135.f),
            palmLeft(x - 80, y + 140, 4.2f, 0.075f, "Boss_Hand_Left", "Boss_PalmAttackt", 64.f, true, false),
            upperArmRight(x + 50, y + 50, 5.0f, 0.11f, "Boss_Arm_UpperVertical", "", 64.f, true, true, -45.f),
            lowerArmRight(x + 80, y + 100, 4.5f, 0.085f, "Boss_Arm_Vertical", "", 64.f, true, true, -135.f),
            palmRight(x + 80, y + 140, 4.2f, 0.075f, "Boss_Hand_Left", "Boss_PalmAttackt", 64.f, true, true) {
        maxHealth = 500;
        health = maxHealth;
        speed = 5.0f;
        maxSpeed = 25.0f;

        // Offset
        leftHandAttack.currentCooldown = 100.0f;
        rightHandAttack.currentCooldown = 200.5f;
        headAttack.currentCooldown = 100.0f;

        headNeutralPosition = position;

        // Seed random number generator
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    void takeDamage(int amount, const std::string& partName) {
        bool partDestroyed = false;

        if (partName == "head") {
            partDestroyed = head.takeDamage(amount);
        } else if (partName == "palmLeft") {
            partDestroyed = palmLeft.takeDamage(amount);
            if (partDestroyed) {
                // Disable entire left arm
                upperArmLeft.active = false;
                lowerArmLeft.active = false;
            }
        } else if (partName == "palmRight") {
            partDestroyed = palmRight.takeDamage(amount);
            if (partDestroyed) {
                // Disable entire right arm
                upperArmRight.active = false;
                lowerArmRight.active = false;
            }
        }

        // Enable head attacks when both hands are destroyed
        if (!palmLeft.active && !palmRight.active) {
            std::cout << "head attack";
            headCanAttack = true;
        }
    }

    void update(float dt, Vector2f playerPos) {
        // palmRight.takeDamage(150);
        // palmLeft.takeDamage(150);
        //  headCanAttack = true;
        //  Store head neutral position

        worldPosition.x += playerVx;

        // Apply world movement to all parts
        head.applyWorldMovement();
        upperArmLeft.applyWorldMovement();
        lowerArmLeft.applyWorldMovement();
        palmLeft.applyWorldMovement();
        upperArmRight.applyWorldMovement();
        lowerArmRight.applyWorldMovement();
        palmRight.applyWorldMovement();

        headNeutralPosition.x += playerVx;

        // go towards player but use ai too
        auto angleToPlayer = atan2(playerY - headNeutralPosition.y, 200 - headNeutralPosition.x);  // calculate angle to player (from 0 to 360)
        angleToPlayer *= 180 / M_PI;                                                               // convert to degrees
        angleToPlayer = fmod(angleToPlayer + 360, 360);                                            // normalize to [0, 360)
        auto aiResult = ai.result(playerHealth / playerMaxHealth, headNeutralPosition.x, headNeutralPosition.y, 200, playerY, angleToPlayer);
        // aiResult[0]: move towards (1) or away (0), aiResult[1]: angle in degrees
        bool moveTowardsPlayer = (aiResult[0] > 0);
        float moveAngle = aiResult[1] * M_PI / 180;  // convert to radians
        if (moveTowardsPlayer) {
            // move towards player
            headNeutralPositionVx += cos(moveAngle) * dt;
            headNeutralPositionVy += sin(moveAngle) * dt;
        } else {
            // move away from player
            headNeutralPositionVx -= cos(moveAngle) * dt;
            headNeutralPositionVy -= sin(moveAngle) * dt;
        }
        // if too far from player, move towards player
        float distToPlayer = std::hypot(headNeutralPosition.x - playerPos.x, headNeutralPosition.y - playerPos.y);
        if (distToPlayer > 200.f) {
            // Move towards player if too far
            float angleToPlayer = atan2(playerPos.y - headNeutralPosition.y, playerPos.x - headNeutralPosition.x);
            headNeutralPositionVx += cos(angleToPlayer) * dt * 2.0f;  // Move faster towards player
            headNeutralPositionVy += sin(angleToPlayer) * dt * 2.0f;
        }
        // if under player, move up a little
        if (headNeutralPosition.y < playerY) {
            headNeutralPositionVy -= 1;
        }
        // if in the left of the player (x < 200), move right a little
        if (headNeutralPosition.x < 200) {
            headNeutralPositionVx += 1;
        }
        // add vx and vy to head neutral position
        headNeutralPosition.x += headNeutralPositionVx;
        headNeutralPosition.y += headNeutralPositionVy;
        // friction
        headNeutralPositionVx *= 0.95f;  // Apply friction to x velocity
        headNeutralPositionVy *= 0.95f;  // Apply friction to y velocity

        // Head follows neutral position when not attacking
        if (headAttack.state != AttackState::ATTACKING) {
            head.follow(headNeutralPosition, dt);
        }

        leftHandAttack.originalPosition.x += playerVx;
        leftHandAttack.attackTarget.x += playerVx;
        rightHandAttack.originalPosition.x += playerVx;
        rightHandAttack.attackTarget.x += playerVx;
        headAttack.originalPosition.x += playerVx;
        headAttack.attackTarget.x += playerVx;

        // Update hand attack states if hands are active
        if (palmLeft.active)
            updateAttackState(dt, playerPos, leftHandAttack, palmLeft);
        if (palmRight.active)
            updateAttackState(dt, playerPos, rightHandAttack, palmRight);

        // Update head attack if enabled
        if (headCanAttack) {
            updateAttackState(dt, playerPos, headAttack, head, true);
        }

        // Left arm chain
        if (palmLeft.active) {
            if (leftHandAttack.state == AttackState::ATTACKING || leftHandAttack.state == AttackState::RETURNING) {
                bridgeArm(upperArmLeft, lowerArmLeft, head.position + LEFT_ARM_BASE_OFFSET, palmLeft, dt);
            } else {
                upperArmLeft.follow(head.position + LEFT_ARM_BASE_OFFSET, dt);
                lowerArmLeft.follow(upperArmLeft.position + FOREARM_OFFSET, dt);
                palmLeft.follow(lowerArmLeft.position + PALM_OFFSET, dt);
            }
        }

        // Right arm chain
        if (palmRight.active) {
            if (rightHandAttack.state == AttackState::ATTACKING || rightHandAttack.state == AttackState::RETURNING) {
                bridgeArm(upperArmRight, lowerArmRight, head.position + RIGHT_ARM_BASE_OFFSET, palmRight, dt);
            } else {
                upperArmRight.follow(head.position + RIGHT_ARM_BASE_OFFSET, dt);
                lowerArmRight.follow(upperArmRight.position + Vector2f(-FOREARM_OFFSET.x, FOREARM_OFFSET.y), dt);
                palmRight.follow(lowerArmRight.position + Vector2f(-PALM_OFFSET.x, PALM_OFFSET.y), dt);
            }
        }

        // Check for collisions with player
        checkPlayerCollision();

        // if palms not active, head can attack
        if (!palmLeft.active && !palmRight.active) {
            headCanAttack = true;  // Enable head attacks when both hands are destroyed
        } else {
            headCanAttack = false;  // Disable head attacks when at least one hand is active
        }

        // check for sword hitbox
        // get damaged with playerDamageMultiplier * currentHoldingSwordDamage
        if (swordHitbox1.x > head.position.x - head.spriteSize / 2 &&
            swordHitbox1.x < head.position.x + head.spriteSize / 2 &&
            swordHitbox1.y > head.position.y - head.spriteSize / 2 &&
            swordHitbox1.y < head.position.y + head.spriteSize / 2 &&
            headCanAttack) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "head");
            headNeutralPositionVx += 5 * cos(atan2(head.position.y - playerY, head.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(head.position.y - playerY, head.position.x - 200));
        }

        if (swordHitbox1.x > palmLeft.position.x - palmLeft.spriteSize / 2 &&
            swordHitbox1.x < palmLeft.position.x + palmLeft.spriteSize / 2 &&
            swordHitbox1.y > palmLeft.position.y - palmLeft.spriteSize / 2 &&
            swordHitbox1.y < palmLeft.position.y + palmLeft.spriteSize / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "palmLeft");
            headNeutralPositionVx += 5 * cos(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
        }

        if (swordHitbox1.x > palmRight.position.x - palmRight.spriteSize / 2 &&
            swordHitbox1.x < palmRight.position.x + palmRight.spriteSize / 2 &&
            swordHitbox1.y > palmRight.position.y - palmRight.spriteSize / 2 &&
            swordHitbox1.y < palmRight.position.y + palmRight.spriteSize / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "palmRight");
            headNeutralPositionVx += 5 * cos(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
        }

        if (swordHitbox2.x > head.position.x - head.spriteSize / 2 &&
            swordHitbox2.x < head.position.x + head.spriteSize / 2 &&
            swordHitbox2.y > head.position.y - head.spriteSize / 2 &&
            swordHitbox2.y < head.position.y + head.spriteSize / 2 &&
            headCanAttack) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "head");
            headNeutralPositionVx += 5 * cos(atan2(head.position.y - playerY, head.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(head.position.y - playerY, head.position.x - 200));
        }

        if (swordHitbox2.x > palmLeft.position.x - palmLeft.spriteSize / 2 &&
            swordHitbox2.x < palmLeft.position.x + palmLeft.spriteSize / 2 &&
            swordHitbox2.y > palmLeft.position.y - palmLeft.spriteSize / 2 &&
            swordHitbox2.y < palmLeft.position.y + palmLeft.spriteSize / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "palmLeft");
            headNeutralPositionVx += 5 * cos(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
        }

        if (swordHitbox2.x > palmRight.position.x - palmRight.spriteSize / 2 &&
            swordHitbox2.x < palmRight.position.x + palmRight.spriteSize / 2 &&
            swordHitbox2.y > palmRight.position.y - palmRight.spriteSize / 2 &&
            swordHitbox2.y < palmRight.position.y + palmRight.spriteSize / 2) {
            takeDamage(playerDamageMultiplier * currentHoldingSwordDamage, "palmRight");
            headNeutralPositionVx += 5 * cos(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
            headNeutralPositionVy += 5 * sin(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
        }

        // now the player takes damage if its part.itsTouchingPlayer() is true
        if (palmLeft.active && palmLeft.isTouchingPlayer()) {
            playerTakeDamage(60);  // Damage dealt by left palm
            playerVx += -1 * cos(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
            playerVy += -1 * sin(atan2(palmLeft.position.y - playerY, palmLeft.position.x - 200));
        }

        if (palmRight.active && palmRight.isTouchingPlayer()) {
            playerTakeDamage(60);  // Damage dealt by right palm
            playerVx += -1 * cos(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
            playerVy += -1 * sin(atan2(palmRight.position.y - playerY, palmRight.position.x - 200));
        }

        if (headCanAttack && head.isTouchingPlayer()) {
            playerTakeDamage(111);  // Damage dealt by head
            playerVx += -4 * cos(atan2(head.position.y - playerY, head.position.x - 200));
            playerVy += -4 * sin(atan2(head.position.y - playerY, head.position.x - 200));
        }
    }

    void draw(RenderWindow& window) {
        // Draw bones if parts are active
        // if (palmLeft.active) drawBones(window, true);
        // if (palmRight.active) drawBones(window, false);

        // Draw body parts
        if (palmLeft.active) {
            upperArmLeft.draw(window);
            lowerArmLeft.draw(window);
            palmLeft.draw(window);
        }

        if (palmRight.active) {
            upperArmRight.draw(window);
            lowerArmRight.draw(window);
            palmRight.draw(window);
        }

        head.draw(window);

        // Draw health bars
        // drawHealthBars(window);
    }

private:
    void bridgeArm(SkeletronPart& upper, SkeletronPart& lower, const Vector2f& base, SkeletronPart& palm, float dt) {
        if (!palm.active)
            return;

        Vector2f dir = palm.position - base;
        float len = std::hypot(dir.x, dir.y);
        if (len > 0.1f) {
            dir /= len;
            Vector2f t1 = base + dir * (len * 0.3f);
            Vector2f t2 = base + dir * (len * 0.7f);
            upper.follow(t1, dt, true);
            lower.follow(t2, dt, true);
        }
    }

    void updateAttackState(float dt, Vector2f playerPos, AttackState& state, SkeletronPart& part, bool isHead = false) {
        switch (state.state) {
            case AttackState::IDLE:
                state.currentCooldown -= dt;

                if (state.currentCooldown <= 0) {
                    state.state = AttackState::ATTACKING;
                    state.timer = state.ATTACK_DURATION;
                    state.originalPosition = part.position;
                    state.attackTarget = playerPos;
                }
                break;

            case AttackState::ATTACKING:
                // std::cout<<"attack ";
                state.timer -= dt;
                state.hasDamaged = false;
                part.follow(state.attackTarget, dt, true);

                if (state.timer <= 0) {
                    state.state = AttackState::RETURNING;
                    state.timer = state.RETURN_DURATION;
                }
                break;

            case AttackState::RETURNING:
                state.timer -= dt;

                Vector2f returnPos = isHead ? headNeutralPosition : (state.originalPosition + (isHead ? Vector2f(0, 0) : PALM_OFFSET));

                part.follow(returnPos, dt, false);

                if (state.timer <= 0) {
                    state.state = AttackState::IDLE;
                    float randomCooldown =
                        state.COOLDOWN_MIN + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (state.COOLDOWN_MAX - state.COOLDOWN_MIN)));
                    state.currentCooldown = randomCooldown;
                }
                break;
        }
    }

    void checkPlayerCollision() {
        float collisionRadius = 30.f;  // Size for collision detection

        // Check palm left collision during attack
        if (palmLeft.active && leftHandAttack.state == AttackState::ATTACKING) {
            float dist = std::hypot(palmLeft.position.x - playerX, palmLeft.position.y - playerY);
            if (dist < collisionRadius && !leftHandAttack.hasDamaged) {
                // Damage player
                leftHandAttack.hasDamaged = true;
                // player.takeDamage(20);
            }
        }

        // Check palm right collision
        if (palmRight.active && rightHandAttack.state == AttackState::ATTACKING) {
            float dist = std::hypot(palmRight.position.x - playerX, palmRight.position.y - playerY);
            if (dist < collisionRadius && !rightHandAttack.hasDamaged) {
                rightHandAttack.hasDamaged = true;
                // player.takeDamage(20);
            }
        }

        // Check head collision during attack
        if (headCanAttack && headAttack.state == AttackState::ATTACKING) {
            float dist = std::hypot(head.position.x - playerX, head.position.y - playerY);
            if (dist < collisionRadius && !headAttack.hasDamaged) {
                headAttack.hasDamaged = true;
                // player.takeDamage(30); // Head does more damage
            }
        }
    }

public:
    void killAllParts() {
        // head.active = false;
        upperArmLeft.active = false;
        lowerArmLeft.active = false;
        palmLeft.active = false;
        upperArmRight.active = false;
        lowerArmRight.active = false;
        palmRight.active = false;
        // make them take lottta damage too
        // head.takeDamage(1000);
        upperArmLeft.takeDamage(1000);
        lowerArmLeft.takeDamage(1000);
        palmLeft.takeDamage(1000);
        upperArmRight.takeDamage(1000);
        lowerArmRight.takeDamage(1000);
        palmRight.takeDamage(1000);
        headCanAttack = true;
    }
};

// clasa de noduri pentru skill tree
class skillTreeNode {
private:
    bool active = false;  // flag pentru verificarea daca nodul este alocat

    int currentAlloc = 0;                // cate puncte alocate sunt in nod
    int maxAlloc = 0;                    // maximul de puncte alocate
    int cost = 0;                        // cate skillpointuri costa un punct alocat
    int levelReq = 1;                    // nivelul minim de experienta necesar pentru a aloca un punct
    string description;                  // descriptia nodului care este afisata in dreapta sus
    CircleShape node = CircleShape(30);  // nodul care este desenat pe ecran
    float x = 0, y = 0;                  // positia nodului

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
    skillTreeNode(int maxAlloc, int cost, int levelReq, string description, float x, float y) {
        this->maxAlloc = maxAlloc;
        this->cost = cost;
        this->levelReq = levelReq;
        this->description = description;
        this->node.setPosition(Vector2f(x, y));
        this->x = x;
        this->y = y;
    }

    // functie pentru updatarea si desenarea nodului
    void draw(RenderWindow& window, Font& font) {
        if (node.getGlobalBounds().contains(window.mapPixelToCoords(Mouse::getPosition(window)))) {
            // facem nodul negru daca suntem cu mouse-ul pe el
            node.setFillColor(Color::Black);

            //  scrisul pentru alocare care este desenat in dreapta jos
            Text alloc(font, to_string(currentAlloc) + "/" + to_string(maxAlloc), 18);
            alloc.setFont(font);
            alloc.setPosition(node.getPosition() - Vector2f(-40, -65));

            // background-ul pentru alocare
            RectangleShape allocBack;
            allocBack.setSize(Vector2f(alloc.getGlobalBounds().size.x, alloc.getGlobalBounds().size.y + 15));
            allocBack.setFillColor(Color::Black);
            allocBack.setPosition(alloc.getPosition());

            // scrisul pentru descriptia nodului care este desenat in dreapta sus
            Text desc(font, description, 18);
            desc.setFillColor(Color::White);
            desc.setPosition(node.getPosition() - Vector2f(-40, 40));

            // background-ul pentru descriptie
            RectangleShape descBack;
            descBack.setSize(Vector2f(desc.getGlobalBounds().size.x, desc.getGlobalBounds().size.y + 15));
            descBack.setFillColor(Color::Black);
            descBack.setPosition(desc.getPosition());

            // desenarea fiecaru-ia (ordinea conteaza!!)

            window.draw(node);

            window.draw(descBack);
            window.draw(desc);

            window.draw(allocBack);
            window.draw(alloc);

        } else {
            // cu outline negru
            node.setOutlineColor(Color::Black);
            node.setOutlineThickness(2);
            node.setFillColor(Color::Yellow);  // daca nu mai suntem cu mouse-ul pe nod il facem inapoi alb
        }

        // daca am alocat toate punctele dintr-un nod il facem verde
        if (currentAlloc == maxAlloc) {
            node.setOutlineColor(Color::Black);
            node.setOutlineThickness(2);
            node.setFillColor(Color::Green);
        }

        // daca nu avem destul level pentru noduri atunci sunt facute rosu
        if (level < levelReq) {
            node.setOutlineColor(Color::Black);
            node.setOutlineThickness(2);
            node.setFillColor(Color::Red);
        }

        // ordinea functiilor de mai sus conteaza!!!(in teorie)
    }

    // functie pentru verificarea alocarii unui nod
    void allocate(RenderWindow& window) {
        // verificam daca mouse-ul este pe nod, daca apasam, daca avem destul level, daca avem destule
        // skillpoint-uri si daca mai sunt puncte de alocat
        if (node.getGlobalBounds().contains(window.mapPixelToCoords(Mouse::getPosition(window)))) {
            if (leftClick && level >= levelReq && skillPoints >= cost && currentAlloc < maxAlloc) {
                active = true;  // nodul devine alocat
                skillPoints -= cost;
                currentAlloc += 1;
                cout << "Skill activated" << endl;
                leftClick = false;  // resetam butonul de apasare pentru a nu apasa de prea multe ori
            }
        }
    }

    // setters
    void setActive(bool active) { this->active = active; }

    void setPosition(float x, float y) {
        this->node.setPosition(Vector2f(x, y));
        this->x = node.getPosition().x;
        this->y = node.getPosition().y;
    }

    void setCurrentAlloc(int currentAlloc) { this->currentAlloc = currentAlloc; }

    // getters
    bool getActive() { return active; }

    float getX() { return x; }

    float getY() { return y; }

    CircleShape getNode() { return node; }

    string getDescription() { return description; }

    int getCost() { return cost; }

    int getMaxAlloc() { return maxAlloc; }

    int getLevelReq() { return levelReq; }

    int getCurrentAlloc() { return currentAlloc; }
};

class Arrow {
public:
    float x, y;
    float vx, vy;
    std::string type;  // Stores the base name like "arrow_basic"
    sf::Sprite sprite;
    bool toBeDeleted;
    float rotationDegrees;  // Store rotation in degrees

    Arrow(float startX, float startY, float targetX, float targetY, const std::string& currentArrowType)
        : x(startX), y(startY), toBeDeleted(false), sprite(TextureManager::getInstance().find(currentArrowType)), type(currentArrowType) {
        float dx = targetX - startX;
        float dy = targetY - startY;
        // Normalize direction vector
        float length = std::sqrt(dx * dx + dy * dy);
        float speed = 15.0f;  // Arrow speed, can be adjusted

        if (length != 0) {
            vx = (dx / length) * speed;
            vy = (dy / length) * speed;
        } else {
            // Default movement if start and target are same (e.g., straight up if mouse is on player)
            vx = 0;
            vy = -speed;
        }

        // Calculate rotation in degrees
        rotationDegrees = std::atan2(dy, dx) * 180.0f / M_PI;

        TextureManager& tm = TextureManager::getInstance();
        // The 'type' string (e.g., "arrow_basic") is used directly as the texture name
        if (!tm.isLoaded(type)) {
            tm.justLoad(type);  // Attempt to load if not already loaded
        }

        if (tm.isLoaded(type)) {
            sprite.setTexture(tm.find(type));
            sf::FloatRect bounds = sprite.getLocalBounds();
            if (bounds.size.x <= 0 || bounds.size.y <= 0) {
                std::cerr << "Error: Arrow texture '" << type << "' loaded but is empty or invalid." << std::endl;
                toBeDeleted = true;  // Mark for deletion if texture is bad
            } else {
                sprite.setOrigin(Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));  // Center origin for rotation
                // Adjust scale as needed. This is a placeholder scale.
                // e.g. if your arrow sprite is 32x8 pixels and you want it to appear that size:
                // sprite.setScale(32.f / bounds.width, 8.f / bounds.height);
                // For a generic small arrow:
                sprite.setScale(Vector2f(0.5f, 0.5f));  // Adjust scale to fit your game
            }
        } else {
            std::cerr << "Error: Arrow texture '" << type << "' could not be loaded." << std::endl;
            toBeDeleted = true;  // Mark for deletion if texture fails to load
        }
    }

    void update(vector<EnemyGoblin>& mapGoblins, vector<EnemyBaphomet>& mapBaphomets, vector<EnemyReaper>& mapReapers, 
                EnemySkeletron* mapSkeletron, bool bossSpawned) {
        if (toBeDeleted)
            return;

        x += vx;
        y += vy;

        // Arrows move with the world's horizontal scroll, similar to coins/XP orbs
        x += playerVx;
        // y += playerVy; // Typically arrows don't follow player's Y after firing

        // Check for window boundaries (0,0 to 800,600)
        if (x < 0 || x > 800 || y < 0 || y > 600) {
            toBeDeleted = true;
        }

        // Arrow-enemy collision
        // Note: Assumes Enemy class has public getEnemyWidth() and getEnemyHeight() methods,
        // or that enemyWidth/enemyHeight are public members.
        if (!toBeDeleted) {
            for (auto& goblin : mapGoblins) {
                if (!goblin.isToBeDeleted()) {
                    // Hitbox for goblin: center (goblin.getX(), goblin.getY()), size (goblin.enemyWidth, goblin.enemyHeight)
                    // This needs goblin.enemyWidth and goblin.enemyHeight to be accessible.
                    // Add public getters getEnemyWidth() and getEnemyHeight() to Enemy class if they are protected.
                    float goblinLeft = goblin.getX() - goblin.getEnemyWidth() / 2.0f;
                    float goblinRight = goblin.getX() + goblin.getEnemyWidth() / 2.0f;
                    float goblinTop = goblin.getY() - goblin.getEnemyHeight() / 2.0f;
                    float goblinBottom = goblin.getY() + goblin.getEnemyHeight() / 2.0f;

                    if (x >= goblinLeft && x <= goblinRight && y >= goblinTop && y <= goblinBottom) {
                        // Example: if (type == "arrow_fire") arrowDamage = 20;
                        goblin.takeDamage(playerDamageMultiplier * arrowDamage);
                        toBeDeleted = true;
                        break;
                    }
                }
            }
        }
        if (!toBeDeleted) {  // Check Baphomets if not already hit a goblin
            for (auto& baphomet : mapBaphomets) {
                if (!baphomet.isToBeDeleted()) {
                    float baphometLeft = baphomet.getX() - baphomet.getEnemyWidth() / 2.0f;
                    float baphometRight = baphomet.getX() + baphomet.getEnemyWidth() / 2.0f;
                    float baphometTop = baphomet.getY() - baphomet.getEnemyHeight() / 2.0f;
                    float baphometBottom = baphomet.getY() + baphomet.getEnemyHeight() / 2.0f;

                    if (x >= baphometLeft && x <= baphometRight && y >= baphometTop && y <= baphometBottom) {
                        baphomet.takeDamage(playerDamageMultiplier * arrowDamage);
                        toBeDeleted = true;
                        break;
                    }
                }
            }
        }
        if (!toBeDeleted) {  // Check Reapers if not already hit a goblin or baphomet
            for (auto& reaper : mapReapers) {
                if (!reaper.isToBeDeleted()) {
                    float reaperLeft = reaper.getX() - reaper.getEnemyWidth() / 2.0f;
                    float reaperRight = reaper.getX() + reaper.getEnemyWidth() / 2.0f;
                    float reaperTop = reaper.getY() - reaper.getEnemyHeight() / 2.0f;
                    float reaperBottom = reaper.getY() + reaper.getEnemyHeight() / 2.0f;

                    if (x >= reaperLeft && x <= reaperRight && y >= reaperTop && y <= reaperBottom) {
                        reaper.takeDamage(playerDamageMultiplier * arrowDamage);
                        toBeDeleted = true;
                        break;
                    }
                }
            }
        }
        if (!toBeDeleted) {  // Check Skeletron parts if not already hit another enemy
            if (bossSpawned && mapSkeletron != nullptr) {
                std::vector<SkeletronPart*> parts;
                if (!mapSkeletron->palmLeft.active && !mapSkeletron->palmRight.active) {
                    parts.push_back(&mapSkeletron->head);  // Add head only if both palms are inactive
                    mapSkeletron->killAllParts(); // except head js to be sure
                } else {
                    parts.push_back(&mapSkeletron->palmLeft);
                    parts.push_back(&mapSkeletron->palmRight);
                }

                for (SkeletronPart* part : parts) {
                    if (part->active) {
                        float partLeft = part->position.x - part->spriteSize / 2.0f;
                        float partRight = part->position.x + part->spriteSize / 2.0f;
                        float partTop = part->position.y - part->spriteSize / 2.0f;
                        float partBottom = part->position.y + part->spriteSize / 2.0f;

                        if (x >= partLeft && x <= partRight && y >= partTop && y <= partBottom) {
                            part->takeDamage(playerDamageMultiplier * arrowDamage);
                            toBeDeleted = true;
                            break;
                        }
                    }
                }
            }
        }

        // if arrow very out of bounds, delete it
        if (x < -100 || x > 900 || y < -100 || y > 700) {
            toBeDeleted = true;  // Mark for deletion if arrow is too far out of bounds
        }

        x += playerVx;  // Apply world movement to arrow's x position
    }

    void draw(sf::RenderWindow& window) {
        if (toBeDeleted)
            return;
        sprite.setPosition(Vector2f(x, y));
        sprite.setRotation(sf::degrees(rotationDegrees));

        // set scale to 50x50 (based off texture)
        sprite.setScale(Vector2f(50.f / sprite.getLocalBounds().size.x, 50.f / sprite.getLocalBounds().size.y));

        // and origin to center of sprite
        sprite.setOrigin(Vector2f(sprite.getLocalBounds().size.x / 2.0f, sprite.getLocalBounds().size.y / 2.0f));

        window.draw(sprite);
    }

    bool isToBeDeleted() const { return toBeDeleted; }
};

// -------------------------------------------------------------------- containere --------------------------------------------------------------------

vector<Object> mapObjects;          // container pentru obiectele din harta
vector<Tile> mapTiles;              // container pentru tile-urile din harta
vector<Decoration> mapDecorations;  // container pentru decoratiunile din harta
vector<Coin> mapCoins;              // container pentru monedele din harta
vector<ExpOrb> mapExpOrbs;          // container pentru orb-urile de exp din harta
// inamici & entitati
vector<EnemyGoblin> mapGoblins;      // container pentru inamicii de tip goblin
vector<EnemyBaphomet> mapBaphomets;  // container pentru inamicii de tip Baphomet
vector<EnemyReaper> mapReapers;      // container pentru inamicii de tip Reaper
std::unique_ptr<EnemySkeletron> mapSkeletron; // pt boss (eu sunt un boss sincer ca am scris tot codu asta)
// noduri skilltree
skillTreeNode skills[10];    // container pentru nodurile de skill tree
vector<Arrow> playerArrows;  // container pentru sageti

// ---------------------------------------------------------- functiile folosite de obiecte --------------------------------------------------------------------

void spawnCoinAt(float x, float y) {
    mapCoins.push_back(Coin(x, y));  // adauga o moneda la coordonatele x, y
}

void spawnXPAt(float x, float y) {
    mapExpOrbs.push_back(ExpOrb(x, y));  // adauga un orb de exp la coordonatele x, y
}

void playerTakeDamage(int damage) {
    // daca are scut, scade din scut
    // daca nu are scut, scade din viata
    if (playerArmor > 0) {
        playerArmor -= (int)((float)(damage) / armorPowerMultiplier);
        if (playerArmor < 0) {
            playerHealth += playerArmor;  // scade din viata
            playerArmor = 0;              // seteaza scutul la 0
        }
    } else {
        playerHealth -= damage;  // scade din viata
    }
}

// -------------------------------------------------------------------- controale --------------------------------------------------------------------
void controls(RenderWindow& window) {
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
            dashing = true;         // activeaza dash-ul
            playerVx *= dashSpeed;  // seteaza viteza de dash
            playerVy *= dashSpeed;  // seteaza viteza de dash
        }
    }

    if (keysPressed[static_cast<int>(Keyboard::Key::Escape)]) {  // daca tasta Escape este apasata
        exit(0);                                                 // iese din program
    }

    // // debug: C = spawns coins at 500, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::C)]) {  // daca tasta C este apasata
    //     mapCoins.push_back(Coin(500, 300)); // adauga o moneda la coordonatele 500, 300
    // }

    // // debug: V = spawns exp orbs at 500, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::V)]) {  // daca tasta V este apasata
    //     mapExpOrbs.push_back(ExpOrb(500, 300)); // adauga un exp orb la coordonatele 500, 300
    // }

    // // debug: X = spawns goblin at 500, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::X)] && frameCount % 10 == 0) {  // daca tasta X este apasata si frameCount % 10 == 0
    //     mapGoblins.push_back(EnemyGoblin(500, 300, 100, 10, true)); // adauga un goblin la coordonatele 500, 300
    //     cout << "DEBUG: spawned goblin at 500, 300" << endl; // afiseaza mesaj de spawn
    // }

    // // debug: B = spawns baphomet at 600, 300
    // if (keysPressed[static_cast<int>(Keyboard::Key::B)] && frameCount % 10 == 0) {  // daca tasta B este apasata si frameCount % 10 == 0
    //     mapBaphomets.push_back(EnemyBaphomet(600, 300)); // adauga un baphomet la coordonatele 600, 300
    //     cout << "DEBUG: spawned baphomet at 600, 300" << endl; // afiseaza mesaj de spawn
    // }

    // cout << "dashing data: " << "dashing: " << dashing << " canDash: " << canDash << " dashDuration: " << dashDuration << " dashCooldown: " << dashCooldown
    // << endl;

    if (keysPressed[static_cast<int>(Keyboard::Key::Num1)]) {  // daca tasta 1 este apasata, echipeaza primul weapon din inventar
        if (playerInventory.getFirstWeapon().item.type != ItemType::Null) {
            playerHolding = playerInventory.getFirstWeapon().item.texturePath;                         // echipeaza primul weapon
            cout << "DEBUG: equipped weapon: " << playerInventory.getFirstWeapon().item.name << endl;  // afiseaza numele weapon-ului echipat
        }
    } else if (keysPressed[static_cast<int>(Keyboard::Key::Num2)]) {  // daca tasta 2 este apasata, echipeaza al doilea weapon din inventar
        if (playerInventory.getSecondWeapon().item.type != ItemType::Null) {
            playerHolding = playerInventory.getSecondWeapon().item.texturePath;                         // echipeaza al doilea weapon
            cout << "DEBUG: equipped weapon: " << playerInventory.getSecondWeapon().item.name << endl;  // afiseaza numele weapon-ului echipat
        }
    }

    if (mouseDown) {
        cout << "CLICK";
    }

    // if can fire arrows + click pressed
    if (canFireArrows && mouseDown && leftClick) {
        if (arrowCooldown > 0) {
            cout << "DEBUG: cannot fire arrow, cooldown active: " << arrowCooldown << endl;
        } else {
            // get mouse position
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            // create a new arrow
            playerArrows.push_back(Arrow(200, playerY, mousePos.x, mousePos.y, arrowType));  // playerX, playerY are the player's position
            cout << "DEBUG: fired arrow from (" << playerX << ", " << playerY << ") to (" << mousePos.x << ", " << mousePos.y << ")" << endl;
            canFireArrows = false;  // disable firing arrows until next frame
            arrowCooldown = currentBowCooldown;
            if (arrowCooldown > -2)
                arrowCooldown -= 1;  // reduce cooldown by 1 frame
            leftClick = false;       // reset left click to prevent multiple arrows being fired in one frame
        }
    }

    // debug key (L) that skips levelProgres to 8900
    if (keysPressed[static_cast<int>(Keyboard::Key::L)]) {        // daca tasta L este apasata
        levelProgress = 8900;                                     // seteaza progresul nivelului la 8900
        cout << "DEBUG: Skipped to level progress 8900" << endl;  // afiseaza mesaj de debug
    }
}

// -------------------------------------------------------------------- initializare --------------------------------------------------------------------
void init() {
    frameCount = 0;  // initializeaza frameCount
    // map textures
    TextureManager::getInstance().justLoad("Tileset");
    // TextureManager::getInstance().justLoad("dirt");
    // TextureManager::getInstance().justLoad("stone");
    //  player textures
    TextureManager::getInstance().justLoad("Playerset");
    // weapon textures
    TextureManager::getInstance().justLoad("weapon_basic_sword");
    TextureManager::getInstance().justLoad("weapon_ice_sword");
    TextureManager::getInstance().justLoad("weapon_basic_bow");
    // equipment textures
    TextureManager::getInstance().justLoad("equipment_purple_gloves");
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
    // as face un set si pentru reaper da nu e timp...
    TextureManager::getInstance().justLoad("enemy_reaper_move_1");
    TextureManager::getInstance().justLoad("enemy_reaper_move_1_mirror");
    TextureManager::getInstance().justLoad("enemy_reaper_move_2");
    TextureManager::getInstance().justLoad("enemy_reaper_move_2_mirror");
    TextureManager::getInstance().justLoad("enemy_reaper_attack");
    TextureManager::getInstance().justLoad("enemy_reaper_attack_mirror");

    // justLoad all the files that start with "decoration_"
    for (const auto& entry : std::filesystem::directory_iterator("./res/")) {
        if (entry.path().filename().string().find("decoration_") == 0) {
            // delete the extra .png from the filename
            string filename = entry.path().filename().string();
            filename = filename.substr(0, filename.find_last_of('.'));  // remove the .png extension
            // load the texture
            cout << "DEBUG: loading decoration texture: " << filename << endl;
            TextureManager::getInstance().justLoad(filename);
        }
    }
    // justLoad all the files that start with "arrow_"
    for (const auto& entry : std::filesystem::directory_iterator("./res/")) {
        if (entry.path().filename().string().find("arrow_") == 0) {
            // delete the extra .png from the filename
            string filename = entry.path().filename().string();
            filename = filename.substr(0, filename.find_last_of('.'));  // remove the .png extension
            // load the texture
            cout << "DEBUG: loading arrow texture: " << filename << endl;
            TextureManager::getInstance().justLoad(filename);
        }
    }

    // backgroud music
    bgmManager::getInstance().loadAll();
    bgmManager::getInstance().playRandom();

    if (!uiFont.openFromFile("./res/PixelPurl.ttf")) {
        std::cerr << "Error loading font!\n";
    }

    srand(time(NULL));  // initializeaza generatorul de numere aleatorii
    // adauga obiecte in containerul mapObjects
    // DEBUG ONLY
    // mapObjects.push_back(Object(100, 150, 20, Color::Red));
    // mapObjects.push_back(Object(200, 210, 30, Color::Blue));
    // mapObjects.push_back(Object(300, 350, 20, Color::Green));
    // mapObjects.push_back(Object(400, 480, 50, Color::Yellow));
    // mapObjects.push_back(Object(500, 300, 30, Color::Magenta));
    // mapObjects.push_back(Object(600, 170, 40, Color::Cyan));

    worldItems.push_back(ItemObject(1000, 300, 10, Item("Sword", "Basic sword, 11 damag", "weapon_basic_sword", ItemType::Weapon)));
    worldItems.push_back(ItemObject(9000, 300, 11, Item("Sword", "Ice sword, 25 damag", "weapon_ice_sword", ItemType::Weapon)));
    worldItems.push_back(ItemObject(2000, 300, 10, Item("Bow", "Basic bow, 10 damage", "weapon_basic_bow", ItemType::Weapon)));
    worldItems.push_back(
        ItemObject(4050, 330, 10, Item("Puple Gloves", "Lower dashing cooldown", "equipment_purple_gloves", ItemType::Equipment)));

    // init player pos
    playerX = 200;
    playerY = 300;
    playerVx = 0;
    playerVy = 0;

    // 80x60 tileseturille din background
    for (int i = 0; i < 84 / 4; i++) {
        for (int j = 0; j < 64 / 4; j++) {
            mapTiles.push_back(Tile(i * 40 - 40, j * 40 - 40, 40, 40, Color::Black));

            auto random = rand_uniform(0, 100);
            if (random < 20) {
                mapTiles.back().setType("grass4");
            } else if (random < 40) {
                mapTiles.back().setType("grass3");
            } else if (random < 80) {
                mapTiles.back().setType("grass2");
            } else {
                mapTiles.back().setType("grass1");
            }
        }
    }

    // mapSkeletron = new EnemySkeletron(300, 300); // ! debug
    // mapSkeletron->killAllParts();
}

// -------------------------------------------------------------------- update --------------------------------------------------------------------

void updateArrows(RenderWindow& window);
void updateProgress(RenderWindow& window);
void updateEnemySpawns(RenderWindow& window);
void updateDecorationSpawns(RenderWindow& window);
void updateEquipmentEffects(RenderWindow& window);

void update(RenderWindow& window) {  // ! MAIN UPDATE --------------------------
    // limiteaza pozitia jucatorului in fereastra
    if (playerY < 0)
        playerY = 0;
    if (playerY > 600)
        playerY = 600;

    // limiteaza videocitatea jucatorului
    if (!dashing) {
        if (playerVx > SPEED_LIMIT)
            playerVx = SPEED_LIMIT;
        if (playerVx < -SPEED_LIMIT)
            playerVx = -SPEED_LIMIT;
        if (playerVy > SPEED_LIMIT)
            playerVy = SPEED_LIMIT;
        if (playerVy < -SPEED_LIMIT)
            playerVy = -SPEED_LIMIT;
    } else {
        if (playerVx > dashSpeed)
            playerVx = dashSpeed;
        if (playerVx < -dashSpeed)
            playerVx = -dashSpeed;
        if (playerVy > dashSpeed)
            playerVy = dashSpeed;
        if (playerVy < -dashSpeed)
            playerVy = -dashSpeed;
    }

    if (abs(playerVx) < 0.1f)
        playerVx = 0;  // opreste miscarea pe axa x daca viteza este foarte mica
    if (abs(playerVy) < 0.1f)
        playerVy = 0;  // opreste miscarea pe axa y daca viteza este foarte mica
    // pt ca la dash sa nu se miste jucatorul in diagonala random
    if (abs(playerVx) == 0 && abs(playerVy) == 0) {
        notMoving = true;  // jucatorul nu se misca
    } else {
        notMoving = false;  // jucatorul se misca
    }

    // dash mechanics
    if (dashing) {
        playerSpeed += 0.1f;  // creste viteza in dash
        dashDuration -= 1;    // scade durata dash-ului
        if (dashDuration <= 0) {
            dashing = false;    // opreste dash-ul
            canDash = false;    // cooldown-ul incepe
            dashDuration = 30;  // resetare durata dash-ului
        }
    } else {
        if (!canDash) {
            dashCooldown -= 1;  // scade cooldown-ul
            if (dashCooldown <= 0) {
                canDash = true;         // dash-ul este disponibil din nou
                dashCooldown = 60 * 3;  // resetare cooldown
            }
        }
    }

    // adjust based of speedMultipliers[speedSkillLevel]
    playerVx *= speedMultipliers[speedSkillLevel];
    playerVy *= speedMultipliers[speedSkillLevel];

    // actualizeaza pozitia jucatorului in functie de viteza si input-ul tastaturii
    playerX += playerVx;
    playerY += playerVy;

    // frictiune movement player
    playerVx *= 0.95f;  // reduce viteza pe axa x
    playerVy *= 0.95f;  // reduce viteza pe axa y

    // actualizeaza pozitia obiectelor
    for (Object& obj : mapObjects) {
        obj.x += playerVx;
    }

    for (auto& obj : worldItems) {
        if (obj.pickedUp) {
            continue;
        }
        obj.x += playerVx;
    }

    nearbyItemIndex = -1;
    float closestDistance = -1;

    for (auto& worldItem : worldItems) {
        if (worldItem.pickedUp)
            continue;

        sf::Vector2f itemPos(worldItem.x, worldItem.y);
        float dist = distance(Vector2f(200, playerY), itemPos);

        if (dist < pickupRadius && (dist < closestDistance || closestDistance < 0)) {
            closestDistance = dist;
            nearbyItemIndex = static_cast<int>(&worldItem - &worldItems[0]);
        }
    }
    // Handle pickup input

    // asta trebuia la controls() dar ma rog...
    if (keysPressed[static_cast<int>(Keyboard::Key::E)]) {
        if (pressedE == false && nearbyItemIndex != -1 && worldItems[nearbyItemIndex].item.type != ItemType::Null) {
            if (worldItems[nearbyItemIndex].pickedUp == false && inventoryWindow.onAddItem(worldItems[nearbyItemIndex])) {
                worldItems[nearbyItemIndex].pickedUp = true;
                nearbyItemIndex = -1;
            }
        }
        pressedE = true;
    } else {
        pressedE = false;
    }

    if (keysPressed[static_cast<int>(Keyboard::Key::I)]) {
        if (openedInventoryThisPress == false) {
            inventoryVisible = !inventoryVisible;
            inventoryWindow.isVisible = inventoryVisible;
            openedInventoryThisPress = true;
        }
    } else {
        openedInventoryThisPress = false;
    }

    inventoryWindow.updateAlways(window);
    inventoryWindow.update(window, playerInventory);

    // move tiles but their x % 10 so they repeat
    for (Tile& tile : mapTiles) {
        tile.setX(tile.getX() + playerVx);
        if (tile.getX() > 800)
            tile.setX(tile.getX() - 840.0f);
        if (tile.getX() < -40)
            tile.setX(tile.getX() + 840.0f);
    }

    // xp mechanics
    if (xp >= levelXP) {
        level += 1;  // creste nivelul jucatorului
        xp = 0;      // resetare xp
        skillPoints++;
        levelXP += levelXP / 9;

        // creste armura maxima
        playerMaxArmor += rand() % 10 + 2;  // adauga un numar aleatoriu de armura maxima (1-5)

        // creste viata maxima
        playerMaxHealth += 5;

        // creste viata curenta cu 50%
        playerHealth += playerMaxHealth / 2;  // creste viata curenta
        if (playerHealth > playerMaxHealth)
            playerHealth = playerMaxHealth;  // limiteaza viata curenta la maxima

        // creste armura curenta cu 50%
        playerArmor += playerMaxArmor / 2;  // creste armura curenta
        if (playerArmor > playerMaxArmor)
            playerArmor = playerMaxArmor;  // limiteaza armura curenta la maxima

        playerDamageMultiplier += rand_uniform(0.1f, 0.3f);  // creste damage multiplier-ul
    }

    // player health mechanics
    if (playerHealth <= 0) {
        playerHealth = 0;                         // limiteaza viata curenta la 0
        playerArmor = 0;                          // limiteaza armura curenta la 0
        cout << "DEBUG: player is dead" << endl;  // afiseaza mesaj de moarte
        // play dead.wav
        SoundManager::getInstance().playSound("dead");  // reda sunetul de moarte
        // messagebox with you died
        MessageBoxA(NULL, "You died!", "Game Over", MB_OK | MB_ICONERROR);  // afiseaza mesaj de moarte

        // restart the exe
        system("start /B /WAIT /MIN cmd /C start \"\" \"./bin/bloodwavez.exe\"");  // restarteaza jocul

        exit(0);  // iese din program
    } else if (playerHealth > playerMaxHealth) {
        playerHealth = playerMaxHealth;  // limiteaza viata curenta la maxima
    }
    if (playerArmor > playerMaxArmor) {
        playerArmor = playerMaxArmor;  // limiteaza armura curenta la maxima
    }
    if (playerArmor < 0) {
        playerArmor = 0;  // limiteaza armura curenta la 0
    }

    // cout<<mouseX<<" "<<mouseY<<endl;

    // asta trebuia la controls() dar ma rog
    // allows switching between the player screen and the skill tree
    if (keysPressed[static_cast<int>(Keyboard::Key::P)]) {
        if (window.getView().getCenter() == playerView.getCenter() && skillTreeDown) {
            window.setView(skillTree);
            skillTreeDown = false;
        } else if (window.getView().getCenter() == skillTree.getCenter() && skillTreeDown) {
            window.setView(playerView);
            skillTreeDown = false;
        }
    }

    if (keysReleased[static_cast<int>(Keyboard::Key::P)])
        skillTreeDown = true;

    // update arrows
    arrowCooldown -= 1;  // reduce cooldown by 1 frame
    updateArrows(window);

    // update bgm
    bgmManager::getInstance().update();

    // update progress
    updateProgress(window);

    // update enemy spawns
    updateEnemySpawns(window);

    // update decoration spawns
    updateDecorationSpawns(window);

    // regen every 60 fps
    if (frameCount % 60 == 0 && playerHealth < playerMaxHealth) {
        playerHealth += regenPer60Frames;
        if (playerHealth > playerMaxHealth) {
            playerHealth = playerMaxHealth;  // Cap health at max health
        }
    }

    // if boss head is destroyed, bossDefeated = true;
    if (mapSkeletron != nullptr && !mapSkeletron->head.active && (!bossDefeated)) {
        bossDefeated = true;
        std::cout << "DEBUG: Boss defeated!" << std::endl;
        mapSkeletron.reset();
        mapSkeletron = nullptr;

        // Reward player with coins and XP
        for (int i = 0; i < 100; ++i) {
            spawnCoinAt(400 + rand_uniform(-50, 50), 300 + rand_uniform(-50, 50));
        }
        for (int i = 0; i < 150; ++i) {
            spawnXPAt(400 + rand_uniform(-50, 50), 300 + rand_uniform(-50, 50));
        }

        // spawn the trophy item
        worldItems.push_back(ItemObject(400, 300, 10, Item("Endgame Trophy", "Makes you immortal", "trophy_endgame", ItemType::Equipment)));

        // message box cu you win
        MessageBoxA(NULL, "You defeated the boss!", "Victory", MB_OK | MB_ICONINFORMATION);  // afiseaza mesaj de victorie
    }

    // update equipment effects
    updateEquipmentEffects(window);
}

void updateArrows(RenderWindow& window) {
    for (auto& arrow : playerArrows) {
        // update each arrow's position and check for collisions
        arrow.update(mapGoblins, mapBaphomets, mapReapers, mapSkeletron.get(), bossSpawned);
    }

    // Remove arrows that are marked for deletion (hit edge or enemy)
    playerArrows.erase(std::remove_if(playerArrows.begin(), playerArrows.end(), [](const Arrow& a) { return a.isToBeDeleted(); }), playerArrows.end());
}

void updateProgress(RenderWindow& window) {
    // if player moving to the right, world progress increases
    if (playerVx < 0) {                                              // se duce in dr
        levelProgress += 1;                                          // increment progress based on player's horizontal movement
        cout << "DEBUG: Level progress: " << levelProgress << endl;  // debug output for level progress

        if (levelProgress >= 9500 && !bossSpawned) {  // spawn boss after 10,000 progress
            bossSpawned = true;
            mapSkeletron = std::make_unique<EnemySkeletron>(500, 300);
            std::cout << "DEBUG: Boss spawned at progress: " << levelProgress << std::endl;
            
            bgmManager::getInstance().stop();
            bgmManager::getInstance().playFile("./res/boss.mp3");
        }
        if (bossDefeated) {
            bgmManager::getInstance().stop();
            bgmManager::getInstance().playRandom();
        }
    }

    // if levelProgress > 9000, change all tiles types to hell1, hell2, hell3, hell4
    if (levelProgress > 9000 && !inHell) {
        cout << "DEBUG: Entering hell mode, changing tile types." << endl;
        inHell = true;
        for (Tile& tile : mapTiles) {
            auto random = rand_uniform(0, 100);
            if (random < 25) {
                tile.setType("hell1");
            } else if (random < 50) {
                tile.setType("hell2");
            } else if (random < 75) {
                tile.setType("hell3");
            } else {
                tile.setType("hell4");
            }
        }
    }
}

void updateEnemySpawns(RenderWindow& window) {
    // if level between 500-1000, spawn goblins every 300 frames
    if (levelProgress > 500 && levelProgress < 1000) {
        if (frameCount % 300 == 0) {
            mapGoblins.push_back(EnemyGoblin(800, rand_uniform(50, 550), 100, 5, true));
            cout << "DEBUG: Spawned goblin cuz progress = " << levelProgress << std::endl;
        }
    }
    // if level between 1000-2000, spawn baphomets every 500 frames
    else if (levelProgress >= 1000 && levelProgress < 2000) {
        if (frameCount % 500 == 0) {
            mapBaphomets.push_back(EnemyBaphomet(800, rand_uniform(50, 550)));
            cout << "DEBUG: Spawned baphomet cuz progress = " << levelProgress << std::endl;
        }
        // and spawn goblins every 200 frames
        if (frameCount % 200 == 0) {
            mapGoblins.push_back(EnemyGoblin(800, rand_uniform(50, 550), 100, 10, true));
            cout << "DEBUG: Spawned goblin cuz progress = " << levelProgress << std::endl;
        }
    } else if (levelProgress >= 2000 && levelProgress < 5000) {
        // if level between 2000-3000, spawn goblins every 150 frames
        if (frameCount % 150 == 0) {
            mapGoblins.push_back(EnemyGoblin(800, rand_uniform(50, 550), 100, 15, true));
            cout << "DEBUG: Spawned goblin cuz progress = " << levelProgress << std::endl;
        }
        // and spawn baphomets every 400 frames
        if (frameCount % 400 == 0) {
            mapBaphomets.push_back(EnemyBaphomet(800, rand_uniform(50, 550)));
            cout << "DEBUG: Spawned baphomet cuz progress = " << levelProgress << std::endl;
        }
        // reapers everry 600 frames
        if (frameCount % 600 == 0) {
            mapReapers.push_back(EnemyReaper(800, rand_uniform(50, 550)));
            cout << "DEBUG: Spawned reaper cuz progress = " << levelProgress << std::endl;
        }
    }
    // bla bla end level
    if (levelProgress >= 9000 && levelProgress < 10000 && (!bossDefeated)) {
        // if levelProgress > 9000, spawn reapers every 100 frames
        if (frameCount % 300 == 0) {
            mapReapers.push_back(EnemyReaper(800, rand_uniform(50, 550)));
            cout << "DEBUG: Spawned reaper cuz progress = " << levelProgress << std::endl;
        }
    }
}

void updateDecorationSpawns(RenderWindow& window) {
    // from level 0 to 1000, spawn decoration_rock every 100 frames
    if (levelProgress >= 0 && levelProgress < 1000) {
        // every 100 frames, spawn decoration_small_rock
        if (frameCount % 100 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_small_rock"));
            cout << "DEBUG: Spawned decoration_rock cuz progress = " << levelProgress << std::endl;
        }
        // every 300 frames, spawn decoration_bush
        if (frameCount % 300 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush"));
            cout << "DEBUG: Spawned decoration_bush cuz progress = " << levelProgress << std::endl;
        }
        // every 360 frames, spawn decoration_bush_2
        if (frameCount % 360 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush_2"));
            cout << "DEBUG: Spawned decoration_bush_2 cuz progress = " << levelProgress << std::endl;
        }
    } else if (levelProgress >= 1000 && levelProgress < 2000) {
        // from level 1000 to 2000, spawn decoration_small_rock every 150 frames
        if (frameCount % 150 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_small_rock"));
            cout << "DEBUG: Spawned decoration_small_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_rock every 200 frames
        if (frameCount % 200 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_rock"));
            cout << "DEBUG: Spawned decoration_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush every 400 frames
        if (frameCount % 400 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush"));
            cout << "DEBUG: Spawned decoration_bush cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush_2 every 500 frames
        if (frameCount % 500 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush_2"));
            cout << "DEBUG: Spawned decoration_bush_2 cuz progress = " << levelProgress << std::endl;
        }
    } else if (levelProgress >= 2000 && levelProgress < 5000) {
        // from level 2000 to 3000, spawn decoration_small_rock every 200 frames
        if (frameCount % 200 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_small_rock"));
            cout << "DEBUG: Spawned decoration_small_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_rock every 300 frames
        if (frameCount % 300 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_rock"));
            cout << "DEBUG: Spawned decoration_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush every 500 frames
        if (frameCount % 500 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush"));
            cout << "DEBUG: Spawned decoration_bush cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush_2 every 600 frames
        if (frameCount % 600 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush_2"));
            cout << "DEBUG: Spawned decoration_bush_2 cuz progress = " << levelProgress << std::endl;
        }
        // spawn blue flower every 450 frames
        if (frameCount % 450 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_blue_flower"));
            cout << "DEBUG: Spawned decoration_blue_flower cuz progress = " << levelProgress << std::endl;
        }
    } else if (levelProgress >= 5000 && levelProgress < 9000) {
        // from level 5000 to 10000, spawn decoration_small_rock every 300 frames
        if (frameCount % 500 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_small_rock"));
            cout << "DEBUG: Spawned decoration_small_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_rock every 400 frames
        if (frameCount % 620 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_rock"));
            cout << "DEBUG: Spawned decoration_rock cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush every 600 frames
        if (frameCount % 720 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush"));
            cout << "DEBUG: Spawned decoration_bush cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush_2 every 700 frames
        if (frameCount % 830 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush_2"));
            cout << "DEBUG: Spawned decoration_bush_2 cuz progress = " << levelProgress << std::endl;
        }
        // scary items
        // spawn decoration_bone every 130
        if (frameCount % 130 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bone"));
            cout << "DEBUG: Spawned decoration_bone cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bone_2 every 268
        if (frameCount % 268 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bone_2"));
            cout << "DEBUG: Spawned decoration_bone_2 cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_long_bone every 350
        if (frameCount % 350 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_long_bone"));
            cout << "DEBUG: Spawned decoration_long_bone cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_long_bone_2 every 550
        if (frameCount % 550 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_long_bone_2"));
            cout << "DEBUG: Spawned decoration_long_bone_2 cuz progress = " << levelProgress << std::endl;
        }
    } else if (levelProgress >= 9000) {
        // only scary items
        // spawn decoration_bone every 100
        if (frameCount % 100 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bone"));
            cout << "DEBUG: Spawned decoration_bone cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bone_2 every 210
        if (frameCount % 200 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bone_2"));
            cout << "DEBUG: Spawned decoration_bone_2 cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_long_bone every 280
        if (frameCount % 280 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_long_bone"));
            cout << "DEBUG: Spawned decoration_long_bone cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_long_bone_2 every 370
        if (frameCount % 370 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_long_bone_2"));
            cout << "DEBUG: Spawned decoration_long_bone_2 cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_bush_red every 80
        if (frameCount % 80 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_bush_red"));
            cout << "DEBUG: Spawned decoration_bush_red cuz progress = " << levelProgress << std::endl;
        }
        // spawn decoration_rock every 666
        if (frameCount % 666 == 0 && playerVx < -0.1f) {
            mapDecorations.push_back(Decoration(rand_uniform(800, 800), rand_uniform(50, 550), "decoration_rock"));
            cout << "DEBUG: Spawned decoration_rock cuz progress = " << levelProgress << std::endl;
        }
    }
}

void updateEquipmentEffects(RenderWindow& window) {
    // update equipment effects based on equipped items
    if (std::any_of(playerInventory.getEquipment().begin(), playerInventory.getEquipment().end(), 
                    [](const ItemObject& item) { return item.item.texturePath == "equipment_purple_gloves"; })) {
        // reduce dash cooldown by 50%
        dashCooldown -= 1;  // reduce cooldown by 1 frame
        // cout << "DEBUG: Dash cooldown reduced to " << dashCooldown << endl;
    }
    // if it has trophy_endgame, make player immortal (set health to max health)
    if (std::any_of(playerInventory.getEquipment().begin(), playerInventory.getEquipment().end(), 
                    [](const ItemObject& item) { return item.item.texturePath == "trophy_endgame"; })) {
        playerHealth = playerMaxHealth;  // set health to max health
        playerArmor = playerMaxArmor;    // set armor to max armor
        cout << "DEBUG: Player is now immortal!" << endl;
    }
}

// -------------------------------------------------------------------- desenare --------------------------------------------------------------------
void drawPlayerAt(RenderWindow& window, float x, float y, float speed = 0, float scale = 0.45f) {
    // deseneaza jucatorul in functie de viteza si pozitie
    Texture& texture = TextureManager::getInstance().find("Playerset");
    ;
    Sprite sprite(texture);

    // individual texture size 90/128

    if (notMoving) {
        sprite.setTextureRect(IntRect({0, 128 * 2}, {90, 128}));
    } else if (playerVx < 0 && moveAnimationCounter % 40 < 20) {
        sprite.setTextureRect(IntRect({90, 128}, {90, 128}));
        moveAnimationCounter++;
    } else if (playerVx < 0 && moveAnimationCounter % 40 >= 20) {
        sprite.setTextureRect(IntRect({90 * 2, 0}, {90, 128}));
        moveAnimationCounter++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 < 20) {
        sprite.setTextureRect(IntRect({90 * 2, 128}, {90, 128}));
        moveAnimationCounter++;
    } else if (playerVx > 0 && moveAnimationCounter % 40 >= 20) {
        sprite.setTextureRect(IntRect({0, 128}, {90, 128}));
        moveAnimationCounter++;
    } else {
        sprite.setTextureRect(IntRect({0, 128 * 2}, {90, 128}));
    }

    if (dashing && playerVx < 0) {
        sprite.setTextureRect(IntRect({0, 0}, {90, 128}));
    } else if (dashing && playerVx > 0) {
        sprite.setTextureRect(IntRect({90, 0}, {90, 128}));
    }

    sprite.setOrigin(Vector2f(45, 64));       // seteaza originea sprite-ului la mijlocul lui
    sprite.setScale(Vector2f(scale, scale));  // seteaza scalarea sprite-ului
    sprite.setPosition(Vector2f(x, y));       // seteaza pozitia sprite-ului (folosim vector2f)
    window.draw(sprite);                      // deseneaza sprite-ul
}

void drawItemInfo(sf::RenderWindow& window) {
    // deseneaza informatiile despre itemul din apropiere

    if (nearbyItemIndex == -1)
        return;  // nu este niciun item in apropiere
    if (worldItems[nearbyItemIndex].item.type == ItemType::Null)
        return;  // nu este niciun item valid

    const std::string& itemName = worldItems[nearbyItemIndex].item.name;         // numele itemului
    const std::string& itemDesc = worldItems[nearbyItemIndex].item.description;  // descrierea itemului

    // creeaza textul pentru nume si descriere
    sf::Text nameText(uiFont, itemName, 24);
    nameText.setFillColor(sf::Color::White);
    nameText.setStyle(sf::Text::Bold);

    // creeaza textul pentru descriere
    sf::Text descText(uiFont, itemDesc, 18);
    descText.setFillColor(sf::Color(200, 200, 200));

    // seteaza pozitia textului
    sf::FloatRect nameBounds = nameText.getLocalBounds();
    sf::FloatRect descBounds = descText.getLocalBounds();

    sf::Vector2f basePosition(worldItems[nearbyItemIndex].x + 20, worldItems[nearbyItemIndex].y - 40);

    // ajustam pozitia in functie de dimensiunea textului
    nameText.setPosition(basePosition);
    sf::RectangleShape underline(sf::Vector2f(nameBounds.size.x + 10, 2));
    underline.setFillColor(sf::Color::White);
    underline.setPosition(sf::Vector2f(basePosition.x, basePosition.y + nameBounds.size.y + 10));
    descText.setPosition(sf::Vector2f(basePosition.x, basePosition.y + nameBounds.size.y + 10));

    // creeaza un background pentru text
    float backgroundWidth = std::max(nameBounds.size.x, descBounds.size.x) + 20;
    float backgroundHeight = nameBounds.size.y + descBounds.size.y + 40;

    sf::RectangleShape background(sf::Vector2f(backgroundWidth, backgroundHeight));
    background.setFillColor(sf::Color(0, 0, 0, 180));
    background.setPosition(sf::Vector2f(basePosition.x - 10, basePosition.y - 10));

    window.draw(background);
    window.draw(nameText);
    window.draw(underline);
    window.draw(descText);
}

// aceasta functie e importanta pt a stii ce item are jucatorul echipat gen daca poate trage
void drawPlayerWeapon(RenderWindow& window) {
    int handX = 214 - ((playerVx > 0) ? 30 : 0);  // pozitia mainii pe axa x

    if (playerHolding == "weapon_basic_sword") {
        Texture& texture = TextureManager::getInstance().find("weapon_basic_sword");
        Sprite sprite(texture);
        sprite.setOrigin(Vector2f(texture.getSize().x / 2, texture.getSize().y / 3 * 2));  // seteaza originea sprite-ului la mijlocul X si 1/3 din Y (manerul)
        sprite.setScale(Vector2f(0.11f, 0.11f));                                           // seteaza scalarea sprite-ului
        sprite.setPosition(Vector2f(handX, playerY));                                      // seteaza pozitia sprite-ului (folosim vector2f)

        // Calculeaza unghiul de rotatie catre mouse
        float angle = atan2(mouseY - playerY, mouseX - (handX)) * 180 / 3.14159f + 90;  // +90 pentru a alinia sprite-ul
        sprite.setRotation(sf::degrees(angle));                                         // seteaza rotatia sprite-uluis

        window.draw(sprite);  // deseneaza sprite-ul

        // // draw hitbox-ul 1 (DEBUG) = tip-ul sabiei
        // CircleShape circle(5); // creeaza un cerc cu raza 20
        // circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        // circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        // circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 30, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 30)); // seteaza
        // pozitia cercului (folosim vector2f) window.draw(circle); // deseneaza cercul
        // // draw hitbox-ul 2 (DEBUG) = tip-ul sabiei
        // circle.setFillColor(Color::Red); // seteaza culoarea cercului ca transparenta
        // circle.setOutlineColor(Color::Transparent); // seteaza culoarea conturului cercului ca rosu
        // circle.setPosition(Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f/2.0f) * 37, playerY - sin(angle / 180 * 3.14f + 3.14f/2.0f) * 37)); // seteaza
        // pozitia cercului (folosim vector2f) window.draw(circle); // deseneaza cercul

        swordHitbox1 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f / 2.0f) * 30,
                                playerY - sin(angle / 180 * 3.14f + 3.14f / 2.0f) * 30);  // seteaza pozitia hitbox-ului sabiei (folosim vector2f)
        swordHitbox2 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f / 2.0f) * 37,
                                playerY - sin(angle / 180 * 3.14f + 3.14f / 2.0f) * 37);  // seteaza pozitia hitbox-ului sabiei (folosim vector2f)

        canFireArrows = false;
        currentHoldingSwordDamage = 11;
    }

    if (playerHolding == "weapon_ice_sword") {
        Texture& texture = TextureManager::getInstance().find("weapon_ice_sword");
        Sprite sprite(texture);
        sprite.setOrigin(Vector2f(texture.getSize().x / 2, texture.getSize().y / 3 * 2));  // seteaza originea sprite-ului la mijlocul X si 1/3 din Y (manerul)
        sprite.setScale(Vector2f(0.11f, 0.11f));                                           // seteaza scalarea sprite-ului
        sprite.setPosition(Vector2f(handX, playerY));                                      // seteaza pozitia sprite-ului (folosim vector2f)

        // Calculeaza unghiul de rotatie catre mouse
        float angle = atan2(mouseY - playerY, mouseX - (handX)) * 180 / 3.14159f + 90;  // +90 pentru a alinia sprite-ul
        sprite.setRotation(sf::degrees(angle));                                         // seteaza rotatia sprite-ului

        window.draw(sprite);  // deseneaza sprite-ul

        swordHitbox1 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f / 2.0f) * 30,
                                playerY - sin(angle / 180 * 3.14f + 3.14f / 2.0f) * 30);  // seteaza pozitia hitbox-ului sabiei (folosim vector2f)
        swordHitbox2 = Vector2f(handX - cos(angle / 180 * 3.14f + 3.14f / 2.0f) * 37,
                                playerY - sin(angle / 180 * 3.14f + 3.14f / 2.0f) * 37);  // seteaza pozitia hitbox-ului sabiei (folosim vector2f)

        canFireArrows = false;
        currentHoldingSwordDamage = 25;  // Damage specific pentru ice sword
    }


    if (playerHolding == "weapon_basic_bow") {
        Texture& texture = TextureManager::getInstance().find("weapon_basic_bow");
        Sprite sprite(texture);
        sprite.setOrigin(Vector2f(texture.getSize().x / 2, texture.getSize().y / 3 * 2));  // seteaza originea sprite-ului la mijloc
        sprite.setScale(Vector2f(0.11f, 0.11f));                                           // seteaza scalarea sprite-ului
        sprite.setPosition(Vector2f(handX, playerY));                                      // seteaza pozitia sprite-ului (folosim vector2f)

        // Calculeaza unghiul de rotatie catre mouse
        float angle = atan2(mouseY - playerY, mouseX - (handX)) * 180 / 3.14159f + 90;  // +90 pentru a alinia sprite-ul
        sprite.setRotation(sf::degrees(angle));                                         // seteaza rotatia sprite-ului

        window.draw(sprite);  // deseneaza sprite-ul

        canFireArrows = true;       // pt ca are arc
        arrowType = "arrow_basic";  // seteaza tipul de sageata la sageata de bazass
        currentBowCooldown = 60;
    }
}
void drawCoins(RenderWindow& window) {
    for (Coin& coin : mapCoins) {
        coin.update();      // actualizeaza moneda
        coin.draw(window);  // deseneaza moneda

        // deja exista o limitare in obiect dar asta nu strica
        if (coin.getX() < 0)
            coin.setX(0);  // limiteaza moneda pe axa x
        if (coin.getX() > 800)
            coin.setX(800);  // limiteaza moneda pe axa x
        if (coin.getY() < 0)
            coin.setY(0);  // limiteaza moneda pe axa y
        if (coin.getY() > 600)
            coin.setY(600);  // limiteaza moneda pe axa y

        // delete moneda daca este autodistrusa
        if (coin.isToBeDeleted()) {
            auto it = remove_if(mapCoins.begin(), mapCoins.end(), [](Coin& c) { return c.isToBeDeleted(); });
            mapCoins.erase(it, mapCoins.end());  // sterge moneda din vector
        }
    }
}

void drawExpOrbs(RenderWindow& window) {
    for (ExpOrb& orb : mapExpOrbs) {
        orb.update();      // actualizeaza orb-ul
        orb.draw(window);  // deseneaza orb-ul

        // delete orb daca este autodistrus
        if (orb.isToBeDeleted()) {
            auto it = remove_if(mapExpOrbs.begin(), mapExpOrbs.end(), [](ExpOrb& o) { return o.isToBeDeleted(); });
            mapExpOrbs.erase(it, mapExpOrbs.end());  // sterge orb-ul din vector
        }
    }
}

void drawBars(RenderWindow& window) {
    // draw health bar
    RectangleShape healthBar(Vector2f(200, 10));                                                   // creeaza un dreptunghi cu dimensiunile specificate
    healthBar.setFillColor(Color::Green);                                                          // seteaza culoarea
    healthBar.setPosition(Vector2f(10, 10));                                                       // seteaza pozitia (folosim vector2f)
    healthBar.setSize(Vector2f(200 * (float)((float)playerHealth / (float)playerMaxHealth), 10));  // seteaza dimensiunile
    window.draw(healthBar);                                                                        // deseneaza dreptunghiul

    // draw armor bar
    RectangleShape armorBar(Vector2f(200, 20));                                                 // creeaza un dreptunghi cu dimensiunile specificate
    armorBar.setFillColor(Color::Blue);                                                         // seteaza culoarea
    armorBar.setPosition(Vector2f(10, 30));                                                     // seteaza pozitia (folosim vector2f)
    armorBar.setSize(Vector2f(200 * (float)((float)playerArmor / (float)playerMaxArmor), 10));  // seteaza dimensiunile
    window.draw(armorBar);                                                                      // deseneaza dreptunghiul

    // draw xp bar
    RectangleShape xpBar(Vector2f(200, 30));                                 // creeaza un dreptunghi cu dimensiunile specificate
    xpBar.setFillColor(Color::Yellow);                                       // seteaza culoarea
    xpBar.setPosition(Vector2f(10, 50));                                     // seteaza pozitia (folosim vector2f)
    xpBar.setSize(Vector2f(200 * (float)((float)xp / (float)levelXP), 10));  // seteaza dimensiunile
    window.draw(xpBar);                                                      // deseneaza dreptunghiul

    // draw the 2px strok for each bar above
    RectangleShape healthBarStroke(Vector2f(200, 10));  // creeaza un dreptunghi cu dimensiunile specificate
    healthBarStroke.setFillColor(Color::Transparent);   // seteaza culoarea
    RectangleShape armorBarStroke(Vector2f(200, 10));   // creeaza un dreptunghi cu dimensiunile specificate
    armorBarStroke.setFillColor(Color::Transparent);    // seteaza culoarea
    RectangleShape xpBarStroke(Vector2f(200, 10));      // creeaza un dreptunghi cu dimensiunile specificate
    xpBarStroke.setFillColor(Color::Transparent);       // seteaza culoarea

    healthBarStroke.setOutlineThickness(2);         // seteaza grosimea conturului
    healthBarStroke.setOutlineColor(Color::Black);  // seteaza culoarea conturului
    armorBarStroke.setOutlineThickness(2);          // seteaza grosimea conturului
    armorBarStroke.setOutlineColor(Color::Black);   // seteaza culoarea conturului
    xpBarStroke.setOutlineThickness(2);             // seteaza grosimea conturului
    xpBarStroke.setOutlineColor(Color::Black);      // seteaza culoarea conturului

    healthBarStroke.setPosition(Vector2f(10, 10));  // seteaza pozitia (folosim vector2f)
    armorBarStroke.setPosition(Vector2f(10, 30));   // seteaza pozitia (folosim vector2f)
    xpBarStroke.setPosition(Vector2f(10, 50));      // seteaza pozitia (folosim vector2f)

    window.draw(healthBarStroke);  // deseneaza dreptunghiul
    window.draw(armorBarStroke);   // deseneaza dreptunghiul
    window.draw(xpBarStroke);      // deseneaza dreptunghiul

    // get icon set
    // each icon size is 32x32
    Texture& texture = TextureManager::getInstance().find("Miscset");

    Sprite sprite(texture);

    // change for smaller or bigger icons
    float scale = 0.6f;

    sprite.setTextureRect(IntRect({0, 0}, {32, 32}));
    sprite.setPosition(Vector2f(217, 11 - 5));  // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale));    // seteaza scalarea sprite-ului
    window.draw(sprite);                        // deseneaza sprite-ul

    sprite.setTextureRect(IntRect({32, 0}, {32, 32}));
    sprite.setPosition(Vector2f(217, 29 - 5));  // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale));    // seteaza scalarea sprite-ului
    window.draw(sprite);                        // deseneaza sprite-ul

    sprite.setTextureRect(IntRect({0, 32}, {32, 32}));
    sprite.setPosition(Vector2f(217, 48));    // seteaza pozitia (folosim vector2f)
    sprite.setScale(Vector2f(scale, scale));  // seteaza scalarea sprite-ului
    window.draw(sprite);                      // deseneaza sprite-ul
}

void drawText(RenderWindow& window) {
    // show lvl and money under the bars
    Font font;
    if (!font.openFromFile("./res/PixelPurl.ttf")) {
        cout << "Failed to load font: PixelPurl.ttf" << endl;
        return;
    }

    Text levelText(font, "LVL " + to_string(level) + " | " + to_string(balance) + "$", 27);  // creeaza un text cu font-ul specificat
    levelText.setFont(font);                                                                 // seteaza font-ul text-ului
    levelText.setFillColor(Color::White);
    levelText.setPosition(Vector2f(10, 60));

    // Create shadow text
    Text shadowText = levelText;                                       // Copy the original text properties
    shadowText.setFillColor(Color::Black);                             // Set shadow color
    shadowText.setPosition(levelText.getPosition() + Vector2f(2, 2));  // Offset the shadow

    window.draw(shadowText);  // Draw the shadow first
    window.draw(levelText);   // Draw the original text on top

    // show health/max health and armor/max and xp/next xp to the right of the bars
    Text healthText(font, to_string(playerHealth) + "/" + to_string(playerMaxHealth), 18);
    healthText.setFont(font);
    healthText.setFillColor(Color::White);
    healthText.setPosition(Vector2f(240, 8 - 5));  // Position to the right of the health bar icon

    Text armorText(font, to_string(playerArmor) + "/" + to_string(playerMaxArmor), 18);
    armorText.setFont(font);
    armorText.setFillColor(Color::White);
    armorText.setPosition(Vector2f(240, 28 - 5));  // Position to the right of the armor bar icon

    Text xpText(font, to_string(xp) + "/" + to_string(levelXP), 18);
    xpText.setFont(font);
    xpText.setFillColor(Color::White);
    xpText.setPosition(Vector2f(240, 48 - 5));  // Position to the right of the xp bar icon

    // Create shadow texts
    Text healthShadow = healthText;
    healthShadow.setFillColor(Color::Black);
    healthShadow.setPosition(healthText.getPosition() + Vector2f(1, 1));  // Offset shadow

    Text armorShadow = armorText;
    armorShadow.setFillColor(Color::Black);
    armorShadow.setPosition(armorText.getPosition() + Vector2f(1, 1));  // Offset shadow

    Text xpShadow = xpText;
    xpShadow.setFillColor(Color::Black);
    xpShadow.setPosition(xpText.getPosition() + Vector2f(1, 1));  // Offset shadow

    // Draw shadows first
    window.draw(healthShadow);
    window.draw(armorShadow);
    window.draw(xpShadow);

    // Draw original texts on top
    window.draw(healthText);
    window.draw(armorText);
    window.draw(xpText);
}

void drawArrows(RenderWindow& window) {
    for (auto& arrow : playerArrows) {
        arrow.draw(window);
    }
}

void drawDecorations(RenderWindow& window) {
    // draw all decorations
    for (Decoration& decoration : mapDecorations) {
        decoration.draw(window);  // deseneaza decoratiunea
    }

    // Remove decorations marked for deletion
    mapDecorations.erase(std::remove_if(mapDecorations.begin(), mapDecorations.end(), [](const Decoration& decoration) { return decoration.isToBeDeleted(); }),
                        mapDecorations.end());
}

// draw all enemies
void drawEnemies(RenderWindow& window) {
    // GOBLINS
    for (EnemyGoblin& enemy : mapGoblins) {
        enemy.update(window);  // actualizeaza inamicul
        enemy.draw(window);    // deseneaza inamicul

        // delete inamic daca este autodistrus
        if (enemy.isToBeDeleted()) {
            auto it = remove_if(mapGoblins.begin(), mapGoblins.end(), [](EnemyGoblin& e) { return e.isToBeDeleted(); });
            mapGoblins.erase(it, mapGoblins.end());  // sterge inamicul din vector
        }
    }

    // BAPHOMETS
    for (EnemyBaphomet& enemy : mapBaphomets) {
        enemy.update(window);  // actualizeaza inamicul
        enemy.draw(window);    // deseneaza inamicul

        // delete inamic daca este autodistrus
        if (enemy.isToBeDeleted()) {
            auto it = remove_if(mapBaphomets.begin(), mapBaphomets.end(), [](EnemyBaphomet& e) { return e.isToBeDeleted(); });
            mapBaphomets.erase(it, mapBaphomets.end());  // sterge inamicul din vector
        }
    }

    // REAPERS
    for (EnemyReaper& enemy : mapReapers) {
        enemy.update(window);  // actualizeaza inamicul
        enemy.draw(window);    // deseneaza inamicul

        // delete inamic daca este autodistrus
        if (enemy.isToBeDeleted()) {
            auto it = remove_if(mapReapers.begin(), mapReapers.end(), [](EnemyReaper& e) { return e.isToBeDeleted(); });
            mapReapers.erase(it, mapReapers.end());  // sterge inamicul din vector
        }
    }

    // SKELETRON
    if (bossSpawned && (!bossDefeated)) {
        if (mapSkeletron != nullptr) {
            mapSkeletron->update(1.f, sf::Vector2f(200 + playerVx, playerY));
            mapSkeletron->draw(window);
        }

        if (mapSkeletron->isToBeDeleted()) {
            mapSkeletron.reset();
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

    // creare background
    Sprite skillTree(texture);
    skillTree.setPosition(Vector2f(-400, 700));
    skillTree.setScale(Vector2f(8, 5));

    window.draw(skillTree);

    // generarea nodurilor
    int nrSkills = 10;

    int x = -350, y = 950, offx = 200, offy = 160;  // x,y = pozitia nodului, offx,offy = offset-ul nodului
    int levelReq = 1, levelScale = 5;

    // initializare o singura data
    if (skills[0].getCost() == 0) {
        skills[0] = skillTreeNode(5, 1, levelReq, "Money +", x, y);

        skills[1] = skillTreeNode(5, 2, levelReq * levelScale, "Viteza +", x + offx, y - offy);
        skills[2] = skillTreeNode(5, 2, levelReq * levelScale, "Regen +", x + offx, y);
        skills[3] = skillTreeNode(5, 2, levelReq * levelScale, "Defense +", x + offx, y + offy);

        skills[4] = skillTreeNode(5, 3, levelReq * levelScale * 2, "Neimplementat", x + 2 * offx, y - offy);
        skills[5] = skillTreeNode(5, 3, levelReq * levelScale * 2, "Neimplementat", x + 2 * offx, y);
        skills[6] = skillTreeNode(5, 3, levelReq * levelScale * 2, "Neimplementat", x + 2 * offx, y + offy);

        skills[7] = skillTreeNode(5, 4, levelReq * levelScale * 3, "Neimplementat", x + 3 * offx, y - offy);
        skills[8] = skillTreeNode(5, 4, levelReq * levelScale * 3, "Neimplementat", x + 3 * offx, y);
        skills[9] = skillTreeNode(5, 4, levelReq * levelScale * 3, "Neimplementat", x + 3 * offx, y + offy);
    }

    Font font;
    if (!font.openFromFile("./res/PixelPurl.ttf")) {
        cout << "Failed to load font: PixelPurl.ttf" << endl;
        return;
    }

    // textul de sus afisand cate skillpoint-uri are jucatorul
    Text nrSkp(font, "You have " + to_string(skillPoints) + " skill points.");
    nrSkp.setFillColor(Color::Black);
    nrSkp.setPosition(Vector2f(-100, 700));
    nrSkp.setCharacterSize(40);

    window.draw(nrSkp);

    // foru-l principal pentru updatarea skilltree-ului
    for (int i = 0; i < nrSkills; i++) {
        window.draw(skills[i].getNode());
        skills[i].draw(window, font);
        skills[i].allocate(window);
    }

    // implementare functionlitate skillpoints
    moneyMultiplier = (float)((float)(skills[0].getCurrentAlloc()) / 5.0f + 1.0f);

    speedSkillLevel = skills[1].getCurrentAlloc();
    regenPer60Frames = (float)(skills[2].getCurrentAlloc());
    armorPowerMultiplier = (float)((float)(skills[3].getCurrentAlloc()) / 8.0f + 1.0f);
}

void drawWorldItems(RenderWindow& window) {
    for (auto& worldItem : worldItems) {
        if (worldItem.pickedUp)
            continue;            // skip if item is picked up
        worldItem.draw(window);  // deseneaza obiectul din lume
    }
}

void draw(RenderWindow& window) {
    // deseneaza fiecare tile din vectorul mapTiles
    for (Tile& tile : mapTiles) {
        tile.draw(window);
    }

    // decoratiuni
    drawDecorations(window);  // desenare decoratiuni

    // weapon
    drawPlayerWeapon(window);  // desenare sabie la pozitia (210, playerY)
    // desenare jucator: un cerc verde
    drawPlayerAt(window, 200, playerY);  // desenare jucator la pozitia (200, playerY) cu viteza 0

    // deseneaza fiecare obiect din vectorul mapObjects
    for (Object& obj : mapObjects) {
        obj.draw(window);
    }

    drawWorldItems(window);  // desenare obiecte din lume

    drawEnemies(window);  // desenare inamici

    // player arrows
    drawArrows(window);  // desenare sageti

    drawExpOrbs(window);  // desenare orb-uri de exp
    drawCoins(window);    // desenare monede

    // those needs 2 be on top
    drawBars(window);  // desenare bare de viata, armura si xp
    drawText(window);  // desenare text cu nivelul si banii

    drawSkillTree(window);

    inventoryWindow.draw(window);
    drawItemInfo(window);
}

// -------------------------------------------------------------------- main --------------------------------------------------------------------
int main() {
    AllocConsole();                   // aloca o consola pentru aplicatie PT DEBUG
    freopen("CONOUT$", "w", stdout);  // redirectioneaza stdout catre consola alocata
    freopen("CONOUT$", "w", stderr);  // redirectioneaza stderr catre consola alocata
    if (RELEASE)
        ShowWindow(GetConsoleWindow(), SW_HIDE);  // ascunde fereastra consolei cand va fi RELEASEs

    // creeaza fereastra jocului cu dimensiuni 800x600 si titlul "project"
    RenderWindow window(VideoMode({800, 600}, 32), "project");
    window.setFramerateLimit(FPS_LIMIT);
    init();

    skillTree.setCenter(Vector2f(0, 1000));
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
                if (keyEv) {
                    keysPressed[static_cast<int>(keyEv->code)] = true;
                    keysReleased[static_cast<int>(keyEv->code)] = false;
                }
            }
            if (event.is<Event::KeyReleased>()) {
                auto keyEv = event.getIf<Event::KeyReleased>();
                if (keyEv) {
                    keysPressed[static_cast<int>(keyEv->code)] = false;
                    keysReleased[static_cast<int>(keyEv->code)] = true;
                }
            }
            if (event.is<Event::MouseButtonPressed>()) {
                auto mouseEv = event.getIf<Event::MouseButtonPressed>();
                mouseDown = true;  // setam mouseDown la true cand apasam un buton
                if (mouseEv) {
                    if (mouseEv->button == Mouse::Button::Left)
                        leftClick = true;
                    if (mouseEv->button == Mouse::Button::Right)
                        rightClick = true;
                }
            }
            if (event.is<Event::MouseButtonReleased>()) {
                auto mouseEv = event.getIf<Event::MouseButtonReleased>();
                mouseDown = false;  // setam mouseDown la false cand eliberam un buton
                if (mouseEv) {
                    if (mouseEv->button == Mouse::Button::Left)
                        leftClick = false;
                    if (mouseEv->button == Mouse::Button::Right)
                        rightClick = false;
                }
            }
            if (event.is<Event::MouseMoved>()) {
                auto mouseEv = event.getIf<Event::MouseMoved>();
                if (mouseEv) {
                    mouseX = mouseEv->position.x;
                    mouseY = mouseEv->position.y;
                }
            }
        }
        window.clear();    // sterge continutul ferestrei
        controls(window);  // gestioneaza input-ul de la tastatura si mouse
        update(window);    // actualizeaza starea jocului
        draw(window);      // deseneaza totul in fereastra
        window.display();  // afiseaza continutul desenat pe ecran

        frameCount++;  // creste numarul de frame-uri
    }
    return 0;  // intoarce 0 la terminarea executiei
}
