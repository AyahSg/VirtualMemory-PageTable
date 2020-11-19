//
// Created by student on 09/11/2020.
//

#include <stdio.h>
#include "os.h"

#define LEVELS 5
#define SYMBOL 9
#define PARTITION 0x1FF

void destroyMapping(uint64_t pt, uint64_t vpn);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    if (ppn == NO_MAPPING) {
        //call function to destroy mapping
        destroyMapping(pt, vpn);
        return;
    }
    uint64_t physicalAddress = pt << 12;
    uint64_t *pageTable = phys_to_virt(physicalAddress); //points to the root of the pt
    int symbolNum;
    uint64_t pte;
    int foundInvalidNode = 0;

    for (int level = 0; level < LEVELS; level++) {
        symbolNum = (vpn >> (((LEVELS - 1) - level) * SYMBOL)) & PARTITION; //extract the relevant vpn offset
        pte = pageTable[symbolNum];
        if (level == LEVELS - 1) {
            //special handling in case we finished walking the table
            pageTable[symbolNum] = (ppn << 12) + 1; //map vpn to given ppn, leftmost 12 bits are zero except for the valid
            break;
        }
        if (pte & 1 && !foundInvalidNode) {
            pageTable = phys_to_virt(pte);
        } else {
            foundInvalidNode = 1;
            //we need to insert this node, as well as a new path
            pte = (alloc_page_frame() << 12) + 1;
            pageTable[symbolNum] = pte;
            pageTable = phys_to_virt(pte); //now go on to next level
        }

    }
}

void destroyMapping(uint64_t pt, uint64_t vpn) {
    uint64_t physicalAddress = pt << 12;
    uint64_t *pageTable;
    int symbolNum;
    pageTable = phys_to_virt(physicalAddress);
    uint64_t pte;
    symbolNum = (vpn >> (((LEVELS - 1)) * SYMBOL)) & PARTITION; //extract the relevant vpn offset
    pte = pageTable[symbolNum];
    if (pte & 1) {
        pageTable[symbolNum] -= 1;
        return;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t physicalAddress = pt << 12;
    uint64_t *pageTable;
    int symbolNum;
    pageTable = phys_to_virt(physicalAddress);
    uint64_t pte;

    for (int level = 0; level < LEVELS; level++) {
        symbolNum = (vpn >> (((LEVELS - 1) - level) * SYMBOL)) & PARTITION; //extract the relevant vpn offset
        pte = pageTable[symbolNum];
        if (!(pte&1)){
            return NO_MAPPING;
        }
        pageTable=phys_to_virt(pte);
    }
    return pte >> 12;
}
