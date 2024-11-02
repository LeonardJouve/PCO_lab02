#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <algorithm>

IWindowInterface* Supplier::interface = nullptr;

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0) 
{
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }

    interface->consoleAppendText(uniqueId, QString("Supplier Created"));
    interface->updateFund(uniqueId, fund);
}


int Supplier::request(ItemType it, int qty) {
    // TODO
    if (qty <= 0 || find(resourcesSupplied.begin(), resourcesSupplied.end(), it) == resourcesSupplied.end()) {
        return 0;
    }

    int amount = std::min(stocks[it], qty);
    money += amount * getCostPerUnit(it);
    stocks[it] -= amount;

    return amount;
}

void Supplier::run() {
    interface->consoleAppendText(uniqueId, "[START] Supplier routine");
    while (!PcoThread::thisThread()->stopRequested() /*TODO*/) {
        ItemType resourceSupplied = getRandomItemFromStock();
        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));
        // TODO 
        if (money < supplierCost) continue;

        // TODO decrement funds
        money -= supplierCost;
        ++stocks[resourceSupplied];

        /* Temps aléatoire borné qui simule l'attente du travail fini*/
        interface->simulateWork();
        //TODO

        ++nbSupplied;

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Supplier routine");
}


std::map<ItemType, int> Supplier::getItemsForSale() {
    return stocks;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Supplier::getAmountPaidToWorkers() {
    return nbSupplied * getEmployeeSalary(EmployeeType::Supplier);
}

void Supplier::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::vector<ItemType> Supplier::getResourcesSupplied() const
{
    return resourcesSupplied;
}

int Supplier::send(ItemType it, int qty, int bill){
    return 0;
}
