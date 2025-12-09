#include <stdint.h>
#include <stdlib.h>

typedef struct PredStat {
    unsigned long long total;
    unsigned long long mispred;
} PredStat;

typedef struct Bimodal {
    unsigned int size;     // 256, 1024, 4096, 16384
    unsigned int mask;     // size-1
    unsigned char *ctr;    // array of 2-bit counters (saved in 8-bit)
    struct PredStat pstat;
} Bimodal;

typedef struct GShare {
    unsigned int size;
    unsigned int mask;
    unsigned char *ctr;
    unsigned int ghr;      // global history register
    unsigned int history_bits;
    struct PredStat pstat;
} GShare;

typedef struct Stat { 
    long int insns; 
    struct PredStat nt;
    struct PredStat btfnt;
    struct Bimodal bimodal[4]; // 256, 1K, 4K, 16K
    struct GShare gshare[4];   // 256, 1K, 4K, 16K
} Stat;

static void pred_nt(PredStat *stat, int actual_taken) {
    stat->total++;
    int pred_taken = 0;
    if (pred_taken != actual_taken) {
        stat->mispred++;
    }
}

static void pred_btfnt(PredStat *stat, int actual_taken, int32_t bimm) {
    stat->total++;
    int pred_taken = (bimm < 0); 
    if (pred_taken != actual_taken) {
        stat->mispred++;
    }
}

static void bimodal_init(Bimodal *bimodal, unsigned int size) {
    bimodal->size = size;
    bimodal->mask = size - 1;
    bimodal->ctr = (unsigned char*)calloc(size, 1);
    // start i "svagt taget" (3) eller "svagt ikke-taget" (1) – valgfrit
    for (unsigned i = 0; i < size; i++) {
        bimodal->ctr[i] = 3;
    }
    bimodal->pstat.total = bimodal->pstat.mispred = 0;
}

static void bimodal_update(struct Bimodal *bimodal, uint32_t pc, int actual_taken) {
    unsigned index = (pc >> 2) & bimodal->mask;  // ignorerer de nederste 2 bit (word-align)
    unsigned char state = bimodal->ctr[index];

    int pred_taken = (state >= 2); // high bit = 1 → predict taken

    bimodal->pstat.total++;
    if (pred_taken != actual_taken) {
        bimodal->pstat.mispred++;
    }
    // opdate state
    if (actual_taken) {
        if (state < 3) state++;
    } else {
        if (state > 0) state--;
    }
    bimodal->ctr[index] = state;
}

static void gshare_init(struct GShare *gshare, unsigned int size) {
    gshare->size = size;
    gshare->mask = size - 1;
    gshare->ctr  = (unsigned char*)calloc(size, 1);
    for (unsigned i = 0; i < size; i++) {
        gshare->ctr[i] = 3; // fx weak taken
    }
    gshare->history_bits = 0;
    while ((1u << gshare->history_bits) < size) {
        gshare->history_bits++;
    }
    gshare->ghr = 0;
    gshare->pstat.total = gshare->pstat.mispred = 0;
}

static void gshare_update(struct GShare *gshare, uint32_t pc, int actual_taken) {
    unsigned index_bits = (pc >> 2) & gshare->mask;
    unsigned index = (index_bits ^ (gshare->ghr & gshare->mask));

    unsigned char state = gshare->ctr[index];
    int pred_taken = (state >= 2);

    gshare->pstat.total++;
    if (pred_taken != actual_taken) {
        gshare->pstat.mispred++;
    }
    // opdater saturerende counter
    if (actual_taken) {
        if (state < 3) state++;
    } else {
        if (state > 0) state--;
    }
    gshare->ctr[index] = state;

    // opdater global history
    gshare->ghr = ((gshare->ghr << 1) | (actual_taken ? 1 : 0)) & gshare->mask;
}