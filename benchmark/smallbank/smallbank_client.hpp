#pragma once

#include <cstdint>
#include <string>
#include <chrono>

#include "transaction.hpp"
#include "smallbank/smallbank_utils.hpp"
#include "smallbank/smallbank_constants.hpp"
#include "utils/random.hpp"


class SmallBankClient {
  private:
    SBDb * db = nullptr;
    RandUIntUniform op_rand;
    RandUInt * id_rand = nullptr;

    // measurements
    uint64_t commited_txns = 0;
    uint64_t launched_txns = 0;

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

    void run_amalgamate(uint32_t id1, uint32_t id2) {
        // first generate parameters
        uint64_t tries = 0;
        while (true) {
            launched_txns++;
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            Account acc2 = txn.Read(id2);
            // todo: check overflow
            acc2.checks += acc1.saving + acc1.checks;
            acc1.saving = 0;
            acc1.checks = 0;
            txn.Update(id1, acc1);
            txn.Update(id2, acc2);
            if (txn.Commit()) break;
            tries++;
        }
    }
    uint32_t run_balance(uint32_t id1) {
        uint64_t tries = 0;
        Account acc1;
        while (true) {
            launched_txns++;
            Transaction<Account> txn(db);
            acc1 = txn.Read(id1);
            if (txn.Commit()) break;
            tries++;
        }
        return acc1.checks + acc1.saving;
    }
    void run_deposit_checking(uint32_t id1, uint32_t amount) {
        if (amount < 0) ASSERT_MSG(false, "deposit checking amount must be positive"); 
        uint64_t tries = 0;
        while (true) {
            launched_txns++;
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            acc1.checks += amount;
            txn.Update(id1, acc1);
            if (txn.Commit()) break;
            tries++;
        }
    }
    void run_transact_saving(uint32_t id1, uint32_t amount) {
        uint64_t tries = 0;
        while (true) {
            launched_txns++;
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            if (acc1.saving + amount < 0) {
                fprintf(stderr, "transact saving cannot have negative result"); 
                return;
            }
            acc1.saving += amount;
            txn.Update(id1, acc1);
            if (txn.Commit()) break;
            tries++;
        }
    }
    void run_write_check(uint32_t id1, uint32_t amount) {
        uint64_t tries = 0;
        while (true) {
            launched_txns++;
            Transaction<Account> txn(db);
            Account acc1 = txn.Read(id1);
            acc1.checks -= amount;
            if (acc1.saving + acc1.checks < 0) acc1.checks -= 1;
            txn.Update(id1, acc1);
            if (txn.Commit()) break;
            tries++;
        }
    }
    void run_send_payment(uint32_t id1, uint32_t id2, uint32_t amount) {
        // first generate parameters
        uint64_t tries = 0;
        while (true) {
            launched_txns++;
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
            if (txn.Commit()) break;
            tries++;
        }
    }
    void run_one() {
        Proc op = select_op();
        uint32_t id1 = id_rand->next(), id2;
        switch (op) {
            case Amalgamate: {
                do {id2 = id_rand->next();} while (id1 == id2);
                run_amalgamate(id1, id2); break;
            }
            case Balance: {
                run_balance(id1); break;
            }
            case DepositChecking: {
                run_deposit_checking(id1, 130); break;
            }
            case TransactSaving: {
                run_transact_saving(id1, 2020); break;
            }
            case WriteCheck: {
                run_write_check(id1, 500); break;
            }
            case SendPayment: {
                do {id2 = id_rand->next();} while (id1 == id2);
                run_send_payment(id1, id2, 500); break;
            }
        }
        commited_txns ++;
    }
  public:
    SmallBankClient(SBDb * db) : db(db), op_rand(0, 99) {
        if (SmallBankConstants::ENABLE_HOTSPOT) {
            id_rand = new RandUIntHot(0, SmallBankConstants::NUM_ACCOUNTS-1, SmallBankConstants::HOTSPOT_FIXED_SIZE, SmallBankConstants::HOTSPOT_PROB);
        } else {
            id_rand = new RandUIntUniform(0, SmallBankConstants::NUM_ACCOUNTS-1);
        }
    }
    ~SmallBankClient() {
        delete id_rand;
    }
    void run_for_time(const std::chrono::time_point<std::chrono::high_resolution_clock> & start_time, const uint64_t second_limit) {
        uint64_t old_sec = 0;
        while(true) {
            uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count();
            if (seconds > old_sec) {
                fprintf(stderr, "now at sec %lu, lt=%lu, cmtt=%lu\n", seconds, launched_txns, commited_txns);
                old_sec = seconds;
            }
            if (seconds > second_limit) break;
            run_one();
        }
    }

};