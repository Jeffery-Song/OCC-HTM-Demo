#pragma once

#include "smallbank/smallbank_constants.hpp"
#include "smallbank/smallbank_utils.hpp"
#include "utils/random.hpp"

#include <vector>
#include <thread>

class SmallBankLoader {
  private:
    uint32_t cnt_loader = SmallBankConstants::CONCURRENT_LOADER;
    uint32_t cnt_account = SmallBankConstants::NUM_ACCOUNTS;
    SBDb *db = nullptr;
    Account gen(uint32_t id, RandUIntUniform & balance_rand) {
        Account acc;
        acc.id = id;
        // todo: generate name
        acc.checks = balance_rand.next();
        acc.saving = balance_rand.next();
        return acc;
    }

    void load_range(uint32_t start_id, uint32_t stop_id) {
        ASSERT_LE(start_id, stop_id);
        RandUIntUniform rand(SmallBankConstants::MIN_BALANCE, SmallBankConstants::MAX_BALANCE);
        uint32_t bulk_size = 0;
        SBTxn txn(db);
        for (int id = start_id; id < stop_id; id++) {
            Account acc = gen(id, rand);
            fprintf(stderr, "loading id=%d\n", id);
            txn.Insert(id, acc);
            bulk_size++;
            if (bulk_size >= SmallBankConstants::BULK_LOAD_SIZE) {
                bulk_size = 0;
                ASSERT(txn.Commit());
            }
        }
        if (bulk_size > 0) {
            ASSERT(txn.Commit());
        }
    }

  public:
    SmallBankLoader(SBDb * db) : db(db) {}
    void load() {
        std::vector<std::thread> thds(cnt_loader);
        uint32_t range_size = cnt_account / cnt_loader;
        uint32_t start_id = 0;
        for (int i = 0; i < cnt_loader; i++) {
            uint32_t stop_id = (i == cnt_loader-1) ? cnt_account : start_id + range_size;
            thds[i] = std::move(std::thread(&SmallBankLoader::load_range, this, start_id, stop_id));
            start_id += range_size;
        }
        for (int i = 0; i < cnt_loader; i++) {
            thds[i].join();
        }
    }
};
