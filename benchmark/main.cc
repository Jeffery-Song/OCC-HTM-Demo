#include "smallbank/smallbank_loader.hpp"
#include "smallbank/smallbank_client.hpp"
#include "monitor.hpp"

#include <vector>
#include <thread>
#include <fstream>

int main() {
    SBDb db;
    SmallBankLoader sb_loader(&db);
    sb_loader.load();

    MonitorManager mn_mngr(SmallBankConstants::NUM_CLIENTS);

    mn_mngr.regist(SmallBankClient::ProcNames);

    auto start_time = std::chrono::high_resolution_clock::now();

    auto thd_func = [&db, &start_time, &mn_mngr](int i) {
        SmallBankClient client(&db, &mn_mngr.monitors[i]);
        client.run_for_time(start_time, SmallBankConstants::RUN_TIME_SEC);
    };
    std::ofstream dump_file;
#ifdef USE_RTM_TXN
    dump_file.open("rtm.log");
#else
    dump_file.open("normal.log");
#endif
    auto state_thd_func = [&mn_mngr, &dump_file, start_time, second_limit = SmallBankConstants::RUN_TIME_SEC]() {
        mn_mngr.start(start_time);
        while(true) {
            sleep(5);
            uint64_t sec = seconds(start_time, now());
            if (sec > second_limit) break;
            std::cerr << sec << "s :" << mn_mngr.show_current_state();
        }
        std::cerr << "fin, " << mn_mngr.summary();
        mn_mngr.dump(dump_file);
    };

    std::vector<std::thread> thds(SmallBankConstants::NUM_CLIENTS);
    for (int i = 0; i < SmallBankConstants::NUM_CLIENTS; i++) {
        thds[i] = std::thread(thd_func, i);
    }
    std::thread state_thd = std::thread(state_thd_func);

    for (auto & thd : thds) {
        thd.join();
    }
    state_thd.join();
    dump_file.close();
}