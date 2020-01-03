#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <chrono>
#include <iostream>
#include "utils/timer.hpp"
const uint64_t slot_num = 10000;
const uint64_t slot_interval = 100;
class Monitor {
  public:
    struct Item {
        uint64_t count = 0;
        uint64_t occ_aborts = 0;
        uint64_t htm_aborts = 0;
        uint64_t htm_use_lock = 0;
        void add(const Item & a) {
            count += a.count;
            occ_aborts += a.occ_aborts;
            htm_aborts += a.htm_aborts;
            htm_use_lock += a.htm_use_lock;
        }
    };
  private:
    friend class MonitorManager;
    // we need: (for each operation)
    //   latency distribution (of each op)
    //   occ abort times
    //   htm abort times
    // we don't need:
    //   latency-runtime 
  public:
    class OpItem {
      public:
        std::vector<Item> slots; // latency -> cnt
        Item overflow_slot;
        // Item total_slot;
        OpItem() : slots(slot_num) {}
        void place(
                uint64_t latency,
                uint64_t occ_aborts,
                uint64_t htm_aborts,
                uint64_t htm_use_lock) {
            if (latency / slot_interval >= slot_num) {
                overflow_slot.count++;
                overflow_slot.occ_aborts += occ_aborts;
                overflow_slot.htm_aborts += htm_aborts;
                overflow_slot.htm_use_lock += htm_use_lock;
            } else {
                slots[latency/slot_interval].count++;
                slots[latency/slot_interval].occ_aborts += occ_aborts;
                slots[latency/slot_interval].htm_aborts += htm_aborts;
                slots[latency/slot_interval].htm_use_lock += htm_use_lock;
            }
        }
        void add(const OpItem & a) {
            overflow_slot.add(a.overflow_slot);
            for (int i = 0; i < slots.size(); i++) {
                slots[i].add(a.slots[i]);
            }
        }
        void dump(std::ostream & os) {
            int i = 0;
            for (; i < slots.size(); i++) {
                os << i * slot_interval << ",\t"
                    << slots[i].count << ",\t"
                    << slots[i].occ_aborts << ",\t"
                    << slots[i].htm_aborts << ",\t"
                    << slots[i].htm_use_lock << "\n";
            }
            os << i * slot_interval << ",\t"
                << overflow_slot.count << ",\t"
                << overflow_slot.occ_aborts << ",\t"
                << overflow_slot.htm_aborts << ",\t"
                << overflow_slot.htm_use_lock << "\n";
        }
    };
  private:
    std::vector<OpItem> logs;
    uint64_t total_cnt = 0;

  public:
    void regist(int op_num) {
        logs.resize(op_num);
    }
    void record(int op, 
                uint64_t latency,
                uint64_t occ_aborts,
                uint64_t htm_aborts,
                uint64_t htm_use_lock) {
        logs[op].place(latency, occ_aborts, htm_aborts, htm_use_lock);
        total_cnt++;
    }
};

class MonitorManager {

    std::vector<std::string> op_names;
    uint64_t current_cnt = 0;
    uint64_t total_cnt = 0;
    timepoint_t last_time;
    timepoint_t start_time;


    void gather() {
        uint64_t new_total_cnt = 0;
        for (auto & mn : monitors) {
            new_total_cnt += mn.total_cnt;
        }
        current_cnt = new_total_cnt - total_cnt;
        total_cnt = new_total_cnt;
    }

  public:
    std::vector<Monitor> monitors;

    MonitorManager(int num_thds) : monitors(num_thds) {
        
    }
    void regist(std::vector<std::string> names_in) {
        op_names = names_in;
        for (auto & mn : monitors) {
            mn.regist(names_in.size());
        }
    }
    void start(timepoint_t start) {
        last_time = start;
        start_time = start;
    }
    std::string show_current_state() {
        gather();
        timepoint_t this_time = now();
        uint64_t sec = seconds(start_time, this_time);
        std::stringstream ss;
        ss << total_cnt << " commited txns, " 
           << (double)current_cnt / microseconds(last_time, this_time) * 1000000.0
           << " txn/s\n";
        last_time = this_time;
        current_cnt = 0;
        return ss.str();
    }

    std::string summary() {
        gather();
        std::stringstream ss;
        uint64_t occ_aborts = 0;
        uint64_t htm_aborts = 0;
        uint64_t htm_use_lock = 0;
        for (auto & mn : monitors) {
            for (auto & opitem : mn.logs) {
                for (auto & item : opitem.slots) {
                    occ_aborts += item.occ_aborts;
                    htm_aborts += item.htm_aborts;
                    htm_use_lock += item.htm_use_lock;
                }
                occ_aborts += opitem.overflow_slot.occ_aborts;
                htm_aborts += opitem.overflow_slot.htm_aborts;
                htm_use_lock += opitem.overflow_slot.htm_use_lock;

            }
        }
        
        ss << total_cnt << " commited txns, "
           << occ_aborts << " occ aborts, "
           << htm_aborts << " htm aborts, "
           << htm_use_lock << "htm use locks\n";
        return ss.str();
    }
    void dump(std::ostream & os) {
        Monitor::OpItem aggregate_thd_all_item;
        for (int op = 0; op < op_names.size(); op++) {
            std::string opname = op_names[op];
            os << "# " << opname << std::endl;
            os << "latency(ns), count, occ_aborts, htm_aborts, htm_use_lock\n";
            Monitor::OpItem aggregate_thd_op_item;
            for (auto & mn : monitors) {
                aggregate_thd_op_item.add(mn.logs[op]);
            }
            aggregate_thd_all_item.add(aggregate_thd_op_item);
            aggregate_thd_op_item.dump(os);
        }
        aggregate_thd_all_item.dump(os);
    }
};