#include "transaction.hpp"

#define TestPayloadT uint32_t

ConcurrentHashMap<TestPayloadT>* db;

const int length = 1000;
const unsigned int concurrent_level = 50;

static void* func(void *) {
    for (int i = 1; i < length; i++) {
        uint32_t pre;
        uint32_t post;
        while(true) {
            Transaction<TestPayloadT> txn(db);
            pre = txn.Read(i - 1);
            post = txn.Read(i);
            if (pre > 0) {
                txn.Update(i-1, pre - 1);
                txn.Update(i, post + 1);
                if (txn.Commit()) break;
            } else {
                txn.Commit();
            }
        }
        // fprintf(stderr, "successfully make %d,%d from %u,%u to %u,%u\n", i-1,i, pre,post,pre-1,post+1);
    }
}

int main() {
    db = new ConcurrentHashMap<TestPayloadT>();
    Transaction<TestPayloadT> loader(db);
    loader.Insert(0, concurrent_level);
    for (int i = 1; i < length; i++) {
        loader.Insert(i, 0u);
    }
    assert(loader.Commit());

    Transaction<TestPayloadT> validator1(db);
    ASSERT_EQ(validator1.Read(0), concurrent_level);
    for (int i = 1; i < length; i++) {
        ASSERT_EQ(validator1.Read(i), 0u);
    }
    assert(validator1.Commit());

    pthread_t tids[concurrent_level];
    for (int i = 0; i < concurrent_level; i++) {
        int res = pthread_create(&tids[i], NULL, func, NULL);     // 创建线程
        if (res != 0) {
            printf("create thread err.\n");
            exit(1);
        }
    }
    for (int i = 0; i < concurrent_level; i++)
        pthread_join(tids[i], NULL);

    Transaction<TestPayloadT> validator2(db);
    for (int i = 0; i < length-1; i++) {
        assert(validator2.Read(i) == 0u);
    }
    assert(validator2.Read(length-1) == concurrent_level);
    assert(validator2.Commit());
    fprintf(stderr, "Validation Passed!\n");
    delete db;
}