#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

#include "transaction.hpp"
#include "smallbank/smallbank_utils.hpp"
#include "smallbank/smallbank_constants.hpp"
#include "utils/random.hpp"
#include "utils/timer.hpp"
#include "monitor.hpp"


class SmallBankClient {
  private:
    SBDb * db = nullptr;
    RandUIntUniform op_rand;
    RandUInt * id_rand = nullptr;
    Monitor* monitor;

    enum Proc {
        Amalgamate = 0,
        Balance,
        DepositChecking,
        TransactSaving,
        WriteCheck,
        SendPayment,
    };

    Proc select_op() {
        uint32_t rand = op_rand.next();

        if (rand < SmallBankConstants::FREQUENCY_AMALGAMATE) {
            return Amalgamate;
        }
        rand -= SmallBankConstants::FREQUENCY_AMALGAMATE;

        if (rand < SmallBankConstants::FREQUENCY_BALANCE) {
            return Balance;
        }
        rand -= SmallBankConstants::FREQUENCY_BALANCE;

        if (rand < SmallBankConstants::FREQUENCY_DEPOSIT_CHECKING) {
            return DepositChecking;
        }
        rand -= SmallBankConstants::FREQUENCY_DEPOSIT_CHECKING;

        if (rand < SmallBankConstants::FREQUENCY_TRANSACT_SAVINGS) {
            return TransactSaving;
        }
        rand -= SmallBankConstants::FREQUENCY_TRANSACT_SAVINGS;

        if (rand < SmallBankConstants::FREQUENCY_WRITE_CHECK) {
            return WriteCheck;
        }
        rand -= SmallBankConstants::FREQUENCY_WRITE_CHECK;

        if (rand < SmallBankConstants::FREQUENCY_SEND_PAYMENT) {
            return SendPayment;
        }
        ASSERT(false);
    }

    void run_amalgamate(uint32_t id1, uint32_t id2, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        occ_aborts = 0;
        htm_aborts = 0;
        while (true) {
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            Account acc2 = txn.Read(id2);
            // todo: check overflow
            acc2.checks += acc1.saving + acc1.checks;
            acc1.saving = 0;
            acc1.checks = 0;
            txn.Update(id1, acc1);
            txn.Update(id2, acc2);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
    }
    uint32_t run_balance(uint32_t id1, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        Account acc1;
        while (true) {
            Transaction<Account> txn(db);
            acc1 = txn.Read(id1);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
        return acc1.checks + acc1.saving;
    }
    void run_deposit_checking(uint32_t id1, uint32_t amount, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        if (amount < 0) ASSERT_MSG(false, "deposit checking amount must be positive"); 
        while (true) {
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            acc1.checks += amount;
            txn.Update(id1, acc1);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
    }
    void run_transact_saving(uint32_t id1, uint32_t amount, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        while (true) {
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            if (acc1.saving + amount < 0) {
                fprintf(stderr, "transact saving cannot have negative result"); 
                return;
            }
            acc1.saving += amount;
            txn.Update(id1, acc1);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
    }
    void run_write_check(uint32_t id1, uint32_t amount, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        while (true) {
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            acc1.checks -= amount;
            if (acc1.saving + acc1.checks < 0) acc1.checks -= 1;
            txn.Update(id1, acc1);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
    }
    void run_send_payment(uint32_t id1, uint32_t id2, uint32_t amount, 
                        uint64_t & occ_aborts, uint64_t & htm_aborts, 
                        uint64_t & htm_use_lock) {
        while (true) {
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            Account acc2 = txn.Read(id1);
            if (acc1.checks < amount) {
                // fprintf(stderr, "send payment has no enough checking: %u, %u\n", acc1.checks, amount); 
                return;
            }
            acc1.checks -= amount;
            acc2.checks += amount;
            txn.Update(id1, acc1);
            txn.Update(id2, acc2);
            bool success = txn.Commit();
            htm_aborts += txn.rtm_retry_cnt;
            htm_use_lock += txn.rtm_use_lock;
            if (success) break;
            occ_aborts++;
        }
    }
    void run_one() {
        uint64_t occ_aborts = 0, htm_aborts = 0, htm_use_lock = 0;
        Proc op = select_op();
        uint32_t id1 = id_rand->next(), id2;
        timepoint_t start;
        switch (op) {
            case Amalgamate: {
                do {id2 = id_rand->next();} while (id1 == id2);
                start = now();
                run_amalgamate(id1, id2, occ_aborts, htm_aborts, htm_use_lock); break;
            }
            case Balance: {
                start = now();
                run_balance(id1, occ_aborts, htm_aborts, htm_use_lock); break;
            }
            case DepositChecking: {
                start = now();
                run_deposit_checking(id1, 130, occ_aborts, htm_aborts, htm_use_lock); break;
            }
            case TransactSaving: {
                start = now();
                run_transact_saving(id1, 2020, occ_aborts, htm_aborts, htm_use_lock); break;
            }
            case WriteCheck: {
                start = now();
                run_write_check(id1, 500, occ_aborts, htm_aborts, htm_use_lock); break;
            }
            case SendPayment: {
                do {id2 = id_rand->next();} while (id1 == id2);
                start = now();
                run_send_payment(id1, id2, 500, occ_aborts, htm_aborts, htm_use_lock); break;
            }
        }
        monitor->record(op, nanoseconds(start, now()), occ_aborts, htm_aborts, htm_use_lock);
    }
  public:
    SmallBankClient(SBDb * db, Monitor * mn) : db(db), monitor(mn), op_rand(0, 99) {
        if (SmallBankConstants::ENABLE_HOTSPOT) {
            id_rand = new RandUIntHot(0, SmallBankConstants::NUM_ACCOUNTS-1, SmallBankConstants::HOTSPOT_FIXED_SIZE, SmallBankConstants::HOTSPOT_PROB);
        } else {
            id_rand = new RandUIntUniform(0, SmallBankConstants::NUM_ACCOUNTS-1);
        }
    }
    ~SmallBankClient() {
        delete id_rand;
    }
    void run_for_time(const timepoint_t & start_time, const uint64_t second_limit) {
        uint64_t old_sec = 0;
        while(true) {
            uint64_t sec = seconds(start_time, now());
            if (sec > second_limit) break;
            run_one();
        }
    }


    static std::vector<std::string> ProcNames;
};

std::vector<std::string> SmallBankClient::ProcNames = {
    "Amalgamate",
    "Balance",
    "DepositChecking",
    "TransactSaving",
    "WriteCheck",
    "SendPayment",
};