#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty){
    if (what != ItemType::PatientSick || qty <= 0) return 0;

    mut.lock();
    int amount = std::min(stocks[ItemType::PatientSick], qty);
    money += getCostPerUnit(ItemType::PatientSick) * amount;
    currentBeds -= amount;
    stocks[ItemType::PatientSick] -= amount;
    mut.unlock();
    return amount;
}

void Hospital::freeHealedPatient() {
    int patientsToFree = patientsRecovering[0];//these patients did their 5 days of resting
    //each patient need to stay one less day
    for(int i = 0; i < patientsRecovering.size() - 1; ++i){
        patientsRecovering[i] = patientsRecovering[i + 1];
    }
    patientsRecovering[4] = 0;
    //stocks[ItemType::PatientHealed] -= patientsToFree;
    nbFree += patientsToFree;
    currentBeds -= patientsToFree;
}

void Hospital::transferPatientsFromClinic() {
    for(const auto clinic : this->clinics){
        if(maxBeds == currentBeds) return;
        int healedPatients = clinic->request(ItemType::PatientHealed, 1);

        currentBeds += healedPatients;
        patientsRecovering[4] += healedPatients;//newly healed patients who need to stay 5 days
        mut.lock();
        stocks[ItemType::PatientHealed] += healedPatients;
        mut.unlock();
        money -= getEmployeeSalary(EmployeeType::Nurse) * healedPatients;
        nbHospitalised += healedPatients;
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    if (it != ItemType::PatientSick || qty <= 0 || maxBeds == currentBeds) return 0;
    mut.lock();
    int amount = std::min(std::min(maxBeds - currentBeds, qty), money / bill);

    money -= bill * amount;
    currentBeds += amount;
    stocks[ItemType::PatientSick] += amount;
    mut.unlock();
    nbHospitalised += amount;
    return amount;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    while (!PcoThread::thisThread()->stopRequested()) {

        transferPatientsFromClinic();
        freeHealedPatient();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller*> clinics){
    this->clinics = clinics;

    for (Seller* clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface* windowInterface){
    interface = windowInterface;
}
