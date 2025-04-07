#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>

//se pot schimba tipurile in fucntie de itemele pe care le vom avea
enum class ItemType {
    Weapon,
    Armor,
    Equipment,
    Coin
};

struct Item {
    std::string name;
    ItemType type;
    int amount; // for coins or stackable items

    Item(std::string name, ItemType type, int amount = 1)
        : name(name), type(type), amount(amount) {}
};

class Inventory {
private:
    std::shared_ptr<Item> mainWeapon;
    std::vector<std::shared_ptr<Item>> stackables;
    int coinCount = 0;

public:
    //todo implementeaza inventar vizual
    void pickUp(std::shared_ptr<Item> item) {
        switch (item->type) {
            case ItemType::Weapon:
                if (mainWeapon) {
                    std::cout << "Dropped " << mainWeapon->name << "\n";
                }
                mainWeapon = item;
                std::cout << "Equipped weapon: " << item->name << "\n";
                break;

            case ItemType::Armor:
            case ItemType::Equipment:
                stackables.push_back(item);
                std::cout << "Picked equipment: " << item->name << "\n";
                break;

            case ItemType::Coin:
                coinCount += item->amount;
                std::cout << "Picked up coins: " << item->amount << " (Total: " << coinCount << ")\n";
                break;
        }
    }

    /*void showInventory() {
        std::cout << "\n-- Inventory --\n";
        if (mainWeapon) {
            std::cout << "Weapon: " << mainWeapon->name << "\n";
        } else {
            std::cout << "Weapon: None\n";
        }

        std::cout << "Equipment:\n";
        for (auto& eq : stackables) {
            std::cout << "- " << eq->name << "\n";
        }

        std::cout << "Coins: " << coinCount << "\n";
        std::cout << "----------------\n";
    }*/
};
