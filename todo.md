- Set MTTR for graphics frame buffer to Write Combining (WC)
- Set PAT to be WC for that entry? → If this is all that's necessary should just do this.

  - Update PAT3 to be WC

        Address: 0x277
        the fourth byte I think, the first 3 bits
        We need to set the least significant bit to 1, the other 2 should be 0
        0x1 stands for WC

        Startup:

        - 0: 110 WB
        - 1: 100 WT
        - 2: 111 UC-
        - 3: 000 UC
        - 4: 110 WB
        - 5: 100 WT
        - 6: 111 UC-
        - 7: 000 UC

  - Remap graphics buffers to use PAT3

        PAT-bit: 0
        PCD-bit: 1
        PWT-bit: 1
        i.e. turn on bits 3 and 4 !!!

  - Flush TLB, I think

- Create table printer with columns etc.?
