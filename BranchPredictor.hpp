#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>

struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> table;
    float a=0;
    float b=0;
    float acc=0;

    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc) {
        std::bitset<2>& counter = table[pc & ((1 << 14) - 1)];
        bool prediction = counter.test(1);
        return prediction;

    }
    void update(uint32_t pc, bool taken) {
        std::bitset<2> counter = table[pc & ((1 << 14) - 1)];
        if (taken == 1) {
            if(counter.to_ulong() == 0){counter = std::bitset<2UL>("01");}
            else if(counter.to_ulong() == 1){counter = std::bitset<2UL>("10");}
            else if(counter.to_ulong() == 2){counter = std::bitset<2UL>("11");}
            else if(counter.to_ulong() == 3){counter = std::bitset<2UL>("11");}
        } else {
            if(counter.to_ulong() == 0){counter = std::bitset<2UL>("00");}
            else if(counter.to_ulong() == 1){counter = std::bitset<2UL>("00");}
            else if(counter.to_ulong() == 2){counter = std::bitset<2UL>("01");}
            else if(counter.to_ulong() == 3){counter = std::bitset<2UL>("10");}
        }
        table[pc & ((1 << 14) - 1)] = counter;
        bool prediction = counter.test(1);
        if (prediction == taken){a++;}b++;
        acc = a/b;
    }
};

struct BHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    float a = 0;
    float b = 0;
    float acc = 0 ;

    BHRBranchPredictor(int value) : bhrTable(1 << 2, value), bhr(value) {}

    bool predict(uint32_t pc) {
        int index = bhr.to_ulong();
        std::bitset<2> prediction = bhrTable[index];
        return prediction[1];
    }

    void update(uint32_t pc, bool taken) {
        int index = bhr.to_ulong();
        std::bitset<2>& prediction = bhrTable[index];
        if (taken == prediction[1]){a++;}b++;
        if (taken == true) {
            if(prediction.to_ulong() == 0){prediction = std::bitset<2UL>("01");}
            else if(prediction.to_ulong() == 1){prediction = std::bitset<2UL>("10");}
            else if(prediction.to_ulong() == 2){prediction = std::bitset<2UL>("11");}
            else if(prediction.to_ulong() == 3){prediction = std::bitset<2UL>("11");}
        } else {
            if(prediction.to_ulong() == 0){prediction = std::bitset<2UL>("00");}
            else if(prediction.to_ulong() == 1){prediction = std::bitset<2UL>("00");}
            else if(prediction.to_ulong() == 2){prediction = std::bitset<2UL>("01");}
            else if(prediction.to_ulong() == 3){prediction = std::bitset<2UL>("10");}
        }
        bhrTable[index] = prediction;

        //change the bhr
        bhr >>= 1;
        bhr[1] = taken;
        acc = a/b;
    }
};



struct SaturatingBHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    float a = 0;
    float b = 0;
    float acc=0;

    SaturatingBHRBranchPredictor(int value, int size) : bhrTable(1 << 2, value), bhr(value), table(1 << 14, value), combination(size, value) {
        assert(size <= (1 << 16));
    }

    bool predict(uint32_t pc) {
        std::bitset<2> bhrIndex = (bhr.to_ulong() & 0b11);
        std::bitset<14> tableIndex = (pc & ((1 << 14) - 1));
        std::bitset<2>& counter = table[pc & ((1 << 14) - 1)];
        bool prediction = counter.test(1);

        if(not prediction){
            int index = bhr.to_ulong();
            std::bitset<2> prediction = bhrTable[index];
        }
        return prediction;
    }

    void update(uint32_t pc, bool taken) {
        std::bitset<14> tableIndex = (pc & 0x3FFF);
        std::bitset<2> bhrIndex = (bhr.to_ulong() & 0b11);

        std::bitset<2> counter = table[pc & ((1 << 14) - 1)];
        bool prediction = counter.test(1);
        if(not prediction){
            int index = bhr.to_ulong();
            std::bitset<2> prediction = bhrTable[index];
        }
        if (prediction == taken){a++;}b++;
        if (taken == 1) {
            if(counter.to_ulong() == 0){counter = std::bitset<2UL>("01");}
            else if(counter.to_ulong() == 1){counter = std::bitset<2UL>("10");}
            else if(counter.to_ulong() == 2){counter = std::bitset<2UL>("11");}
            else if(counter.to_ulong() == 3){counter = std::bitset<2UL>("11");}
        } else {
            if(counter.to_ulong() == 0){counter = std::bitset<2UL>("00");}
            else if(counter.to_ulong() == 1){counter = std::bitset<2UL>("00");}
            else if(counter.to_ulong() == 2){counter = std::bitset<2UL>("01");}
            else if(counter.to_ulong() == 3){counter = std::bitset<2UL>("10");}
        }
        table[pc & ((1 << 14) - 1)] = counter;

        int index = bhr.to_ulong();
        std::bitset<2>& bhrguess = bhrTable[index];
        if (taken == true) {
            if(bhrguess.to_ulong() == 0){bhrguess = std::bitset<2UL>("01");}
            else if(bhrguess.to_ulong() == 1){bhrguess = std::bitset<2UL>("10");}
            else if(bhrguess.to_ulong() == 2){bhrguess = std::bitset<2UL>("11");}
            else if(bhrguess.to_ulong() == 3){bhrguess = std::bitset<2UL>("11");}
        } else {
            if(bhrguess.to_ulong() == 0){bhrguess = std::bitset<2UL>("00");}
            else if(bhrguess.to_ulong() == 1){bhrguess = std::bitset<2UL>("00");}
            else if(bhrguess.to_ulong() == 2){bhrguess = std::bitset<2UL>("01");}
            else if(bhrguess.to_ulong() == 3){bhrguess = std::bitset<2UL>("10");}
        }
        bhrTable[index] = bhrguess;
        acc = a/b;
    }
};


#endif