#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

IWindowInterface* Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), nbTreated(0), resourcesNeeded(resourcesNeeded)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Factory created");

    for(const auto& item : resourcesNeeded) {
        stocks[item] = 0;
    }
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] == 0) {
            return false;
        }
    }
    return true;
}

int Clinic::request(ItemType what, int qty){
    if(what != ItemType::PatientHealed || qty <= 0) return 0;

    m_stocks.lock();
    int patients = std::min(qty, stocks[ItemType::PatientHealed]);
    stocks[ItemType::PatientHealed] -= patients;
    money += getCostPerUnit(ItemType::PatientHealed) * patients;
    m_stocks.unlock();

    return patients;
}

void Clinic::treatPatient() {
    if(stocks[ItemType::PatientSick] <= 0) return;

    ++nbTreated;
    m_stocks.lock();
    for(const auto& item : resourcesNeeded){
        if(stocks[item] <= 0){
            std::cerr << "Error " << stocks[item] << std::endl;
            m_stocks.unlock();
            return;
        }
        --stocks[item];
    }

    --stocks[ItemType::PatientSick];
    ++stocks[ItemType::PatientHealed];
    money -= getEmployeeSalary(EmployeeType::Doctor);
    m_stocks.unlock();
    //Temps simulant un traitement 
    interface->simulateWork();
    interface->consoleAppendText(uniqueId, "Clinic have healed a new patient");
}

void Clinic::orderResources() {
    //Acquire any necessary item
    for(const auto& item : resourcesNeeded){
        m_stocks.lock();
        if(stocks[item] > 0){
             m_stocks.unlock();
             continue;
        }
        if(money < getCostPerUnit(item)){
            m_stocks.unlock();
            return;
        }
        for(const auto sup : suppliers){
            int amount = sup->request(item, 1);
            stocks[item] += amount;
            money -= getCostPerUnit(item) * amount;
        }
        m_stocks.unlock();
    }

    for(const auto hospital : hospitals){
        m_stocks.lock();
        if(stocks[ItemType::PatientSick] > 0 || money < getCostPerUnit(ItemType::PatientSick)){
            m_stocks.unlock();
            return;
        }
        int amount = hospital->request(ItemType::PatientSick, 1);
        stocks[ItemType::PatientSick] += amount;
        money -= getCostPerUnit(ItemType::PatientSick) * amount;
        m_stocks.unlock();
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Clinic routine");

    while (!PcoThread::thisThread()->stopRequested()) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
       
        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller* hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller* supplier : suppliers) {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
}

int Clinic::getTreatmentCost() {
    return 0;
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::PatientSick];
}

int Clinic::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed];
}

int Clinic::send(ItemType it, int qty, int bill){
    return 0;
}

int Clinic::getAmountPaidToWorkers() {
    return nbTreated * getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
}

void Clinic::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::map<ItemType, int> Clinic::getItemsForSale() {
    return stocks;
}


Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel}) {}
