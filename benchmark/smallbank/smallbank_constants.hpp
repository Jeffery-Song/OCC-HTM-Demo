#pragma once

#include <cstdint>

class SmallBankConstants {
  public:
    // ----------------------------------------------------------------
    // STORED PROCEDURE EXECUTION FREQUENCIES (0-100)
    // ----------------------------------------------------------------
    static uint32_t FREQUENCY_AMALGAMATE       ;
    static uint32_t FREQUENCY_BALANCE          ;
    static uint32_t FREQUENCY_DEPOSIT_CHECKING ;
    static uint32_t FREQUENCY_SEND_PAYMENT     ;
    static uint32_t FREQUENCY_TRANSACT_SAVINGS ;
    static uint32_t FREQUENCY_WRITE_CHECK      ;

    // ----------------------------------------------------------------
    // ACCOUNT INFORMATION
    // ----------------------------------------------------------------
    // Default number of customers in bank
    static uint32_t NUM_ACCOUNTS           ;
    static bool     ENABLE_HOTSPOT         ;
    static uint32_t HOTSPOT_PROB           ;
    static uint32_t HOTSPOT_FIXED_SIZE     ; // fixed number of tuples

    // ----------------------------------------------------------------
    // ADDITIONAL CONFIGURATION SETTINGS
    // ----------------------------------------------------------------
    // Initial balance amount
    // We'll just make it really big so that they never run out of money
    static uint32_t MIN_BALANCE            ;
    static uint32_t MAX_BALANCE            ;

    // configs
    static uint32_t CONCURRENT_LOADER      ;
    static uint32_t BULK_LOAD_SIZE         ;
    static uint32_t NUM_CLIENTS            ;
    static uint64_t RUN_TIME_SEC           ; // seconds
};

uint32_t SmallBankConstants::FREQUENCY_AMALGAMATE       = 15;
uint32_t SmallBankConstants::FREQUENCY_BALANCE          = 15;
uint32_t SmallBankConstants::FREQUENCY_DEPOSIT_CHECKING = 15;
uint32_t SmallBankConstants::FREQUENCY_SEND_PAYMENT     = 25;
uint32_t SmallBankConstants::FREQUENCY_TRANSACT_SAVINGS = 15;
uint32_t SmallBankConstants::FREQUENCY_WRITE_CHECK      = 15;
uint32_t SmallBankConstants::NUM_ACCOUNTS               = 200000;
bool     SmallBankConstants::ENABLE_HOTSPOT             = false;
uint32_t SmallBankConstants::HOTSPOT_PROB               = 90; // 0-100
uint32_t SmallBankConstants::HOTSPOT_FIXED_SIZE         = 200; // fixed number of tuples
uint32_t SmallBankConstants::MIN_BALANCE                = 1000000;
uint32_t SmallBankConstants::MAX_BALANCE                = 5000000;
uint32_t SmallBankConstants::CONCURRENT_LOADER          = 1;
uint32_t SmallBankConstants::BULK_LOAD_SIZE             = 1;
uint32_t SmallBankConstants::NUM_CLIENTS                = 48;
uint64_t SmallBankConstants::RUN_TIME_SEC               = 20;
