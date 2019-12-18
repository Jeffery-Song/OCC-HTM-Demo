#include "smallbank/smallbank_loader.hpp"
#include "smallbank/smallbank_client.hpp"

#include <vector>
#include <thread>

void client_func(SBDb* db) {

}

int main() {
    SBDb db;
    SmallBankLoader sb_loader(&db);
    sb_loader.load();

    auto start_time = std::chrono::high_resolution_clock::now();

    auto thd_func = [&db, &start_time]() {
        SmallBankClient client(&db);
        client.run_for_time(start_time, SmallBankConstants::RUN_TIME_SEC);
    };

    std::vector<std::thread> thds(SmallBankConstants::NUM_CLIENTS);
    for (int i = 0; i < SmallBankConstants::NUM_CLIENTS; i++) {
        thds[i] = std::thread(thd_func);
    }

    for (auto & thd : thds) {
        thd.join();
    }
}