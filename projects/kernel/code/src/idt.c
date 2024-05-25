#include "idt.h"
#include "memory/definitions.h"
#include "memory/standard.h"
#include "util/log.h"
#include "util/text/string.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void isr48();
extern void isr49();
extern void isr50();
extern void isr51();
extern void isr52();
extern void isr53();
extern void isr54();
extern void isr55();
extern void isr56();
extern void isr57();
extern void isr58();
extern void isr59();
extern void isr60();
extern void isr61();
extern void isr62();
extern void isr63();
extern void isr64();
extern void isr65();
extern void isr66();
extern void isr67();
extern void isr68();
extern void isr69();
extern void isr70();
extern void isr71();
extern void isr72();
extern void isr73();
extern void isr74();
extern void isr75();
extern void isr76();
extern void isr77();
extern void isr78();
extern void isr79();
extern void isr80();
extern void isr81();
extern void isr82();
extern void isr83();
extern void isr84();
extern void isr85();
extern void isr86();
extern void isr87();
extern void isr88();
extern void isr89();
extern void isr90();
extern void isr91();
extern void isr92();
extern void isr93();
extern void isr94();
extern void isr95();
extern void isr96();
extern void isr97();
extern void isr98();
extern void isr99();
extern void isr100();
extern void isr101();
extern void isr102();
extern void isr103();
extern void isr104();
extern void isr105();
extern void isr106();
extern void isr107();
extern void isr108();
extern void isr109();
extern void isr110();
extern void isr111();
extern void isr112();
extern void isr113();
extern void isr114();
extern void isr115();
extern void isr116();
extern void isr117();
extern void isr118();
extern void isr119();
extern void isr120();
extern void isr121();
extern void isr122();
extern void isr123();
extern void isr124();
extern void isr125();
extern void isr126();
extern void isr127();
extern void isr128();
extern void isr129();
extern void isr130();
extern void isr131();
extern void isr132();
extern void isr133();
extern void isr134();
extern void isr135();
extern void isr136();
extern void isr137();
extern void isr138();
extern void isr139();
extern void isr140();
extern void isr141();
extern void isr142();
extern void isr143();
extern void isr144();
extern void isr145();
extern void isr146();
extern void isr147();
extern void isr148();
extern void isr149();
extern void isr150();
extern void isr151();
extern void isr152();
extern void isr153();
extern void isr154();
extern void isr155();
extern void isr156();
extern void isr157();
extern void isr158();
extern void isr159();
extern void isr160();
extern void isr161();
extern void isr162();
extern void isr163();
extern void isr164();
extern void isr165();
extern void isr166();
extern void isr167();
extern void isr168();
extern void isr169();
extern void isr170();
extern void isr171();
extern void isr172();
extern void isr173();
extern void isr174();
extern void isr175();
extern void isr176();
extern void isr177();
extern void isr178();
extern void isr179();
extern void isr180();
extern void isr181();
extern void isr182();
extern void isr183();
extern void isr184();
extern void isr185();
extern void isr186();
extern void isr187();
extern void isr188();
extern void isr189();
extern void isr190();
extern void isr191();
extern void isr192();
extern void isr193();
extern void isr194();
extern void isr195();
extern void isr196();
extern void isr197();
extern void isr198();
extern void isr199();
extern void isr200();
extern void isr201();
extern void isr202();
extern void isr203();
extern void isr204();
extern void isr205();
extern void isr206();
extern void isr207();
extern void isr208();
extern void isr209();
extern void isr210();
extern void isr211();
extern void isr212();
extern void isr213();
extern void isr214();
extern void isr215();
extern void isr216();
extern void isr217();
extern void isr218();
extern void isr219();
extern void isr220();
extern void isr221();
extern void isr222();
extern void isr223();
extern void isr224();
extern void isr225();
extern void isr226();
extern void isr227();
extern void isr228();
extern void isr229();
extern void isr230();
extern void isr231();
extern void isr232();
extern void isr233();
extern void isr234();
extern void isr235();
extern void isr236();
extern void isr237();
extern void isr238();
extern void isr239();
extern void isr240();
extern void isr241();
extern void isr242();
extern void isr243();
extern void isr244();
extern void isr245();
extern void isr246();
extern void isr247();
extern void isr248();
extern void isr249();
extern void isr250();
extern void isr251();
extern void isr252();
extern void isr253();
extern void isr254();
extern void isr255();

extern void asm_lidt(void *);

idt_ptr idtp;
static InterruptDescriptor idt[256];

typedef enum { INTERRUPT_GATE, TRAP_GATE, GATE_NUM } GateType;

uint8_t gateFlags[GATE_NUM] = {0x8E, 0x8F};

void idt_set_gate(uint8_t num, uint64_t base, GateType gateType) {
    idt[num].offset_1 = (base & 0xFFFF);
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;

    // Referring to the GDT segment here
    idt[num].selector = 0x08;

    idt[num].type_attributes = gateFlags[gateType];

    idt[num].ist = 0;
    idt[num].zero = 0;
}

void setupIDT() {
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof(InterruptDescriptor) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(InterruptDescriptor) * 256);

    idt_set_gate(0, (uint64_t)isr0, INTERRUPT_GATE);
    idt_set_gate(1, (uint64_t)isr1, INTERRUPT_GATE);
    idt_set_gate(2, (uint64_t)isr2, INTERRUPT_GATE);
    idt_set_gate(3, (uint64_t)isr3, INTERRUPT_GATE);
    idt_set_gate(4, (uint64_t)isr4, INTERRUPT_GATE);
    idt_set_gate(5, (uint64_t)isr5, INTERRUPT_GATE);
    idt_set_gate(6, (uint64_t)isr6, INTERRUPT_GATE);
    idt_set_gate(7, (uint64_t)isr7, INTERRUPT_GATE);
    idt_set_gate(8, (uint64_t)isr8, INTERRUPT_GATE);
    idt_set_gate(9, (uint64_t)isr9, INTERRUPT_GATE);
    idt_set_gate(10, (uint64_t)isr10, INTERRUPT_GATE);
    idt_set_gate(11, (uint64_t)isr11, INTERRUPT_GATE);
    idt_set_gate(12, (uint64_t)isr12, INTERRUPT_GATE);
    idt_set_gate(13, (uint64_t)isr13, INTERRUPT_GATE);
    idt_set_gate(14, (uint64_t)isr14, INTERRUPT_GATE);
    idt_set_gate(15, (uint64_t)isr15, INTERRUPT_GATE);
    idt_set_gate(16, (uint64_t)isr16, INTERRUPT_GATE);
    idt_set_gate(17, (uint64_t)isr17, INTERRUPT_GATE);
    idt_set_gate(18, (uint64_t)isr18, INTERRUPT_GATE);
    idt_set_gate(19, (uint64_t)isr19, INTERRUPT_GATE);
    idt_set_gate(20, (uint64_t)isr20, INTERRUPT_GATE);
    idt_set_gate(21, (uint64_t)isr21, INTERRUPT_GATE);
    idt_set_gate(22, (uint64_t)isr22, INTERRUPT_GATE);
    idt_set_gate(23, (uint64_t)isr23, INTERRUPT_GATE);
    idt_set_gate(24, (uint64_t)isr24, INTERRUPT_GATE);
    idt_set_gate(25, (uint64_t)isr25, INTERRUPT_GATE);
    idt_set_gate(26, (uint64_t)isr26, INTERRUPT_GATE);
    idt_set_gate(27, (uint64_t)isr27, INTERRUPT_GATE);
    idt_set_gate(28, (uint64_t)isr28, INTERRUPT_GATE);
    idt_set_gate(29, (uint64_t)isr29, INTERRUPT_GATE);
    idt_set_gate(30, (uint64_t)isr30, INTERRUPT_GATE);
    idt_set_gate(31, (uint64_t)isr31, INTERRUPT_GATE);
    idt_set_gate(32, (uint64_t)isr32, INTERRUPT_GATE);
    idt_set_gate(33, (uint64_t)isr33, INTERRUPT_GATE);
    idt_set_gate(34, (uint64_t)isr34, INTERRUPT_GATE);
    idt_set_gate(35, (uint64_t)isr35, INTERRUPT_GATE);
    idt_set_gate(36, (uint64_t)isr36, INTERRUPT_GATE);
    idt_set_gate(37, (uint64_t)isr37, INTERRUPT_GATE);
    idt_set_gate(38, (uint64_t)isr38, INTERRUPT_GATE);
    idt_set_gate(39, (uint64_t)isr39, INTERRUPT_GATE);
    idt_set_gate(40, (uint64_t)isr40, INTERRUPT_GATE);
    idt_set_gate(41, (uint64_t)isr41, INTERRUPT_GATE);
    idt_set_gate(42, (uint64_t)isr42, INTERRUPT_GATE);
    idt_set_gate(43, (uint64_t)isr43, INTERRUPT_GATE);
    idt_set_gate(44, (uint64_t)isr44, INTERRUPT_GATE);
    idt_set_gate(45, (uint64_t)isr45, INTERRUPT_GATE);
    idt_set_gate(46, (uint64_t)isr46, INTERRUPT_GATE);
    idt_set_gate(47, (uint64_t)isr47, INTERRUPT_GATE);
    idt_set_gate(48, (uint64_t)isr48, INTERRUPT_GATE);
    idt_set_gate(49, (uint64_t)isr49, INTERRUPT_GATE);
    idt_set_gate(50, (uint64_t)isr50, INTERRUPT_GATE);
    idt_set_gate(51, (uint64_t)isr51, INTERRUPT_GATE);
    idt_set_gate(52, (uint64_t)isr52, INTERRUPT_GATE);
    idt_set_gate(53, (uint64_t)isr53, INTERRUPT_GATE);
    idt_set_gate(54, (uint64_t)isr54, INTERRUPT_GATE);
    idt_set_gate(55, (uint64_t)isr55, INTERRUPT_GATE);
    idt_set_gate(56, (uint64_t)isr56, INTERRUPT_GATE);
    idt_set_gate(57, (uint64_t)isr57, INTERRUPT_GATE);
    idt_set_gate(58, (uint64_t)isr58, INTERRUPT_GATE);
    idt_set_gate(59, (uint64_t)isr59, INTERRUPT_GATE);
    idt_set_gate(60, (uint64_t)isr60, INTERRUPT_GATE);
    idt_set_gate(61, (uint64_t)isr61, INTERRUPT_GATE);
    idt_set_gate(62, (uint64_t)isr62, INTERRUPT_GATE);
    idt_set_gate(63, (uint64_t)isr63, INTERRUPT_GATE);
    idt_set_gate(64, (uint64_t)isr64, INTERRUPT_GATE);
    idt_set_gate(65, (uint64_t)isr65, INTERRUPT_GATE);
    idt_set_gate(66, (uint64_t)isr66, INTERRUPT_GATE);
    idt_set_gate(67, (uint64_t)isr67, INTERRUPT_GATE);
    idt_set_gate(68, (uint64_t)isr68, INTERRUPT_GATE);
    idt_set_gate(69, (uint64_t)isr69, INTERRUPT_GATE);
    idt_set_gate(70, (uint64_t)isr70, INTERRUPT_GATE);
    idt_set_gate(71, (uint64_t)isr71, INTERRUPT_GATE);
    idt_set_gate(72, (uint64_t)isr72, INTERRUPT_GATE);
    idt_set_gate(73, (uint64_t)isr73, INTERRUPT_GATE);
    idt_set_gate(74, (uint64_t)isr74, INTERRUPT_GATE);
    idt_set_gate(75, (uint64_t)isr75, INTERRUPT_GATE);
    idt_set_gate(76, (uint64_t)isr76, INTERRUPT_GATE);
    idt_set_gate(77, (uint64_t)isr77, INTERRUPT_GATE);
    idt_set_gate(78, (uint64_t)isr78, INTERRUPT_GATE);
    idt_set_gate(79, (uint64_t)isr79, INTERRUPT_GATE);
    idt_set_gate(80, (uint64_t)isr80, INTERRUPT_GATE);
    idt_set_gate(81, (uint64_t)isr81, INTERRUPT_GATE);
    idt_set_gate(82, (uint64_t)isr82, INTERRUPT_GATE);
    idt_set_gate(83, (uint64_t)isr83, INTERRUPT_GATE);
    idt_set_gate(84, (uint64_t)isr84, INTERRUPT_GATE);
    idt_set_gate(85, (uint64_t)isr85, INTERRUPT_GATE);
    idt_set_gate(86, (uint64_t)isr86, INTERRUPT_GATE);
    idt_set_gate(87, (uint64_t)isr87, INTERRUPT_GATE);
    idt_set_gate(88, (uint64_t)isr88, INTERRUPT_GATE);
    idt_set_gate(89, (uint64_t)isr89, INTERRUPT_GATE);
    idt_set_gate(90, (uint64_t)isr90, INTERRUPT_GATE);
    idt_set_gate(91, (uint64_t)isr91, INTERRUPT_GATE);
    idt_set_gate(92, (uint64_t)isr92, INTERRUPT_GATE);
    idt_set_gate(93, (uint64_t)isr93, INTERRUPT_GATE);
    idt_set_gate(94, (uint64_t)isr94, INTERRUPT_GATE);
    idt_set_gate(95, (uint64_t)isr95, INTERRUPT_GATE);
    idt_set_gate(96, (uint64_t)isr96, INTERRUPT_GATE);
    idt_set_gate(97, (uint64_t)isr97, INTERRUPT_GATE);
    idt_set_gate(98, (uint64_t)isr98, INTERRUPT_GATE);
    idt_set_gate(99, (uint64_t)isr99, INTERRUPT_GATE);
    idt_set_gate(100, (uint64_t)isr100, INTERRUPT_GATE);
    idt_set_gate(101, (uint64_t)isr101, INTERRUPT_GATE);
    idt_set_gate(102, (uint64_t)isr102, INTERRUPT_GATE);
    idt_set_gate(103, (uint64_t)isr103, INTERRUPT_GATE);
    idt_set_gate(104, (uint64_t)isr104, INTERRUPT_GATE);
    idt_set_gate(105, (uint64_t)isr105, INTERRUPT_GATE);
    idt_set_gate(106, (uint64_t)isr106, INTERRUPT_GATE);
    idt_set_gate(107, (uint64_t)isr107, INTERRUPT_GATE);
    idt_set_gate(108, (uint64_t)isr108, INTERRUPT_GATE);
    idt_set_gate(109, (uint64_t)isr109, INTERRUPT_GATE);
    idt_set_gate(110, (uint64_t)isr110, INTERRUPT_GATE);
    idt_set_gate(111, (uint64_t)isr111, INTERRUPT_GATE);
    idt_set_gate(112, (uint64_t)isr112, INTERRUPT_GATE);
    idt_set_gate(113, (uint64_t)isr113, INTERRUPT_GATE);
    idt_set_gate(114, (uint64_t)isr114, INTERRUPT_GATE);
    idt_set_gate(115, (uint64_t)isr115, INTERRUPT_GATE);
    idt_set_gate(116, (uint64_t)isr116, INTERRUPT_GATE);
    idt_set_gate(117, (uint64_t)isr117, INTERRUPT_GATE);
    idt_set_gate(118, (uint64_t)isr118, INTERRUPT_GATE);
    idt_set_gate(119, (uint64_t)isr119, INTERRUPT_GATE);
    idt_set_gate(120, (uint64_t)isr120, INTERRUPT_GATE);
    idt_set_gate(121, (uint64_t)isr121, INTERRUPT_GATE);
    idt_set_gate(122, (uint64_t)isr122, INTERRUPT_GATE);
    idt_set_gate(123, (uint64_t)isr123, INTERRUPT_GATE);
    idt_set_gate(124, (uint64_t)isr124, INTERRUPT_GATE);
    idt_set_gate(125, (uint64_t)isr125, INTERRUPT_GATE);
    idt_set_gate(126, (uint64_t)isr126, INTERRUPT_GATE);
    idt_set_gate(127, (uint64_t)isr127, INTERRUPT_GATE);
    idt_set_gate(128, (uint64_t)isr128, INTERRUPT_GATE);
    idt_set_gate(129, (uint64_t)isr129, INTERRUPT_GATE);
    idt_set_gate(130, (uint64_t)isr130, INTERRUPT_GATE);
    idt_set_gate(131, (uint64_t)isr131, INTERRUPT_GATE);
    idt_set_gate(132, (uint64_t)isr132, INTERRUPT_GATE);
    idt_set_gate(133, (uint64_t)isr133, INTERRUPT_GATE);
    idt_set_gate(134, (uint64_t)isr134, INTERRUPT_GATE);
    idt_set_gate(135, (uint64_t)isr135, INTERRUPT_GATE);
    idt_set_gate(136, (uint64_t)isr136, INTERRUPT_GATE);
    idt_set_gate(137, (uint64_t)isr137, INTERRUPT_GATE);
    idt_set_gate(138, (uint64_t)isr138, INTERRUPT_GATE);
    idt_set_gate(139, (uint64_t)isr139, INTERRUPT_GATE);
    idt_set_gate(140, (uint64_t)isr140, INTERRUPT_GATE);
    idt_set_gate(141, (uint64_t)isr141, INTERRUPT_GATE);
    idt_set_gate(142, (uint64_t)isr142, INTERRUPT_GATE);
    idt_set_gate(143, (uint64_t)isr143, INTERRUPT_GATE);
    idt_set_gate(144, (uint64_t)isr144, INTERRUPT_GATE);
    idt_set_gate(145, (uint64_t)isr145, INTERRUPT_GATE);
    idt_set_gate(146, (uint64_t)isr146, INTERRUPT_GATE);
    idt_set_gate(147, (uint64_t)isr147, INTERRUPT_GATE);
    idt_set_gate(148, (uint64_t)isr148, INTERRUPT_GATE);
    idt_set_gate(149, (uint64_t)isr149, INTERRUPT_GATE);
    idt_set_gate(150, (uint64_t)isr150, INTERRUPT_GATE);
    idt_set_gate(151, (uint64_t)isr151, INTERRUPT_GATE);
    idt_set_gate(152, (uint64_t)isr152, INTERRUPT_GATE);
    idt_set_gate(153, (uint64_t)isr153, INTERRUPT_GATE);
    idt_set_gate(154, (uint64_t)isr154, INTERRUPT_GATE);
    idt_set_gate(155, (uint64_t)isr155, INTERRUPT_GATE);
    idt_set_gate(156, (uint64_t)isr156, INTERRUPT_GATE);
    idt_set_gate(157, (uint64_t)isr157, INTERRUPT_GATE);
    idt_set_gate(158, (uint64_t)isr158, INTERRUPT_GATE);
    idt_set_gate(159, (uint64_t)isr159, INTERRUPT_GATE);
    idt_set_gate(160, (uint64_t)isr160, INTERRUPT_GATE);
    idt_set_gate(161, (uint64_t)isr161, INTERRUPT_GATE);
    idt_set_gate(162, (uint64_t)isr162, INTERRUPT_GATE);
    idt_set_gate(163, (uint64_t)isr163, INTERRUPT_GATE);
    idt_set_gate(164, (uint64_t)isr164, INTERRUPT_GATE);
    idt_set_gate(165, (uint64_t)isr165, INTERRUPT_GATE);
    idt_set_gate(166, (uint64_t)isr166, INTERRUPT_GATE);
    idt_set_gate(167, (uint64_t)isr167, INTERRUPT_GATE);
    idt_set_gate(168, (uint64_t)isr168, INTERRUPT_GATE);
    idt_set_gate(169, (uint64_t)isr169, INTERRUPT_GATE);
    idt_set_gate(170, (uint64_t)isr170, INTERRUPT_GATE);
    idt_set_gate(171, (uint64_t)isr171, INTERRUPT_GATE);
    idt_set_gate(172, (uint64_t)isr172, INTERRUPT_GATE);
    idt_set_gate(173, (uint64_t)isr173, INTERRUPT_GATE);
    idt_set_gate(174, (uint64_t)isr174, INTERRUPT_GATE);
    idt_set_gate(175, (uint64_t)isr175, INTERRUPT_GATE);
    idt_set_gate(176, (uint64_t)isr176, INTERRUPT_GATE);
    idt_set_gate(177, (uint64_t)isr177, INTERRUPT_GATE);
    idt_set_gate(178, (uint64_t)isr178, INTERRUPT_GATE);
    idt_set_gate(179, (uint64_t)isr179, INTERRUPT_GATE);
    idt_set_gate(180, (uint64_t)isr180, INTERRUPT_GATE);
    idt_set_gate(181, (uint64_t)isr181, INTERRUPT_GATE);
    idt_set_gate(182, (uint64_t)isr182, INTERRUPT_GATE);
    idt_set_gate(183, (uint64_t)isr183, INTERRUPT_GATE);
    idt_set_gate(184, (uint64_t)isr184, INTERRUPT_GATE);
    idt_set_gate(185, (uint64_t)isr185, INTERRUPT_GATE);
    idt_set_gate(186, (uint64_t)isr186, INTERRUPT_GATE);
    idt_set_gate(187, (uint64_t)isr187, INTERRUPT_GATE);
    idt_set_gate(188, (uint64_t)isr188, INTERRUPT_GATE);
    idt_set_gate(189, (uint64_t)isr189, INTERRUPT_GATE);
    idt_set_gate(190, (uint64_t)isr190, INTERRUPT_GATE);
    idt_set_gate(191, (uint64_t)isr191, INTERRUPT_GATE);
    idt_set_gate(192, (uint64_t)isr192, INTERRUPT_GATE);
    idt_set_gate(193, (uint64_t)isr193, INTERRUPT_GATE);
    idt_set_gate(194, (uint64_t)isr194, INTERRUPT_GATE);
    idt_set_gate(195, (uint64_t)isr195, INTERRUPT_GATE);
    idt_set_gate(196, (uint64_t)isr196, INTERRUPT_GATE);
    idt_set_gate(197, (uint64_t)isr197, INTERRUPT_GATE);
    idt_set_gate(198, (uint64_t)isr198, INTERRUPT_GATE);
    idt_set_gate(199, (uint64_t)isr199, INTERRUPT_GATE);
    idt_set_gate(200, (uint64_t)isr200, INTERRUPT_GATE);
    idt_set_gate(201, (uint64_t)isr201, INTERRUPT_GATE);
    idt_set_gate(202, (uint64_t)isr202, INTERRUPT_GATE);
    idt_set_gate(203, (uint64_t)isr203, INTERRUPT_GATE);
    idt_set_gate(204, (uint64_t)isr204, INTERRUPT_GATE);
    idt_set_gate(205, (uint64_t)isr205, INTERRUPT_GATE);
    idt_set_gate(206, (uint64_t)isr206, INTERRUPT_GATE);
    idt_set_gate(207, (uint64_t)isr207, INTERRUPT_GATE);
    idt_set_gate(208, (uint64_t)isr208, INTERRUPT_GATE);
    idt_set_gate(209, (uint64_t)isr209, INTERRUPT_GATE);
    idt_set_gate(210, (uint64_t)isr210, INTERRUPT_GATE);
    idt_set_gate(211, (uint64_t)isr211, INTERRUPT_GATE);
    idt_set_gate(212, (uint64_t)isr212, INTERRUPT_GATE);
    idt_set_gate(213, (uint64_t)isr213, INTERRUPT_GATE);
    idt_set_gate(214, (uint64_t)isr214, INTERRUPT_GATE);
    idt_set_gate(215, (uint64_t)isr215, INTERRUPT_GATE);
    idt_set_gate(216, (uint64_t)isr216, INTERRUPT_GATE);
    idt_set_gate(217, (uint64_t)isr217, INTERRUPT_GATE);
    idt_set_gate(218, (uint64_t)isr218, INTERRUPT_GATE);
    idt_set_gate(219, (uint64_t)isr219, INTERRUPT_GATE);
    idt_set_gate(220, (uint64_t)isr220, INTERRUPT_GATE);
    idt_set_gate(221, (uint64_t)isr221, INTERRUPT_GATE);
    idt_set_gate(222, (uint64_t)isr222, INTERRUPT_GATE);
    idt_set_gate(223, (uint64_t)isr223, INTERRUPT_GATE);
    idt_set_gate(224, (uint64_t)isr224, INTERRUPT_GATE);
    idt_set_gate(225, (uint64_t)isr225, INTERRUPT_GATE);
    idt_set_gate(226, (uint64_t)isr226, INTERRUPT_GATE);
    idt_set_gate(227, (uint64_t)isr227, INTERRUPT_GATE);
    idt_set_gate(228, (uint64_t)isr228, INTERRUPT_GATE);
    idt_set_gate(229, (uint64_t)isr229, INTERRUPT_GATE);
    idt_set_gate(230, (uint64_t)isr230, INTERRUPT_GATE);
    idt_set_gate(231, (uint64_t)isr231, INTERRUPT_GATE);
    idt_set_gate(232, (uint64_t)isr232, INTERRUPT_GATE);
    idt_set_gate(233, (uint64_t)isr233, INTERRUPT_GATE);
    idt_set_gate(234, (uint64_t)isr234, INTERRUPT_GATE);
    idt_set_gate(235, (uint64_t)isr235, INTERRUPT_GATE);
    idt_set_gate(236, (uint64_t)isr236, INTERRUPT_GATE);
    idt_set_gate(237, (uint64_t)isr237, INTERRUPT_GATE);
    idt_set_gate(238, (uint64_t)isr238, INTERRUPT_GATE);
    idt_set_gate(239, (uint64_t)isr239, INTERRUPT_GATE);
    idt_set_gate(240, (uint64_t)isr240, INTERRUPT_GATE);
    idt_set_gate(241, (uint64_t)isr241, INTERRUPT_GATE);
    idt_set_gate(242, (uint64_t)isr242, INTERRUPT_GATE);
    idt_set_gate(243, (uint64_t)isr243, INTERRUPT_GATE);
    idt_set_gate(244, (uint64_t)isr244, INTERRUPT_GATE);
    idt_set_gate(245, (uint64_t)isr245, INTERRUPT_GATE);
    idt_set_gate(246, (uint64_t)isr246, INTERRUPT_GATE);
    idt_set_gate(247, (uint64_t)isr247, INTERRUPT_GATE);
    idt_set_gate(248, (uint64_t)isr248, INTERRUPT_GATE);
    idt_set_gate(249, (uint64_t)isr249, INTERRUPT_GATE);
    idt_set_gate(250, (uint64_t)isr250, INTERRUPT_GATE);
    idt_set_gate(251, (uint64_t)isr251, INTERRUPT_GATE);
    idt_set_gate(252, (uint64_t)isr252, INTERRUPT_GATE);
    idt_set_gate(253, (uint64_t)isr253, INTERRUPT_GATE);
    idt_set_gate(254, (uint64_t)isr254, INTERRUPT_GATE);
    idt_set_gate(255, (uint64_t)isr255, INTERRUPT_GATE);

    asm_lidt(&idtp);
}

typedef enum : uint64_t {
    FAULT_DIVIDE_ERROR = 0,
    FAULT_DEBUG = 1,
    FAULT_NMI = 2,
    FAULT_BREAKPOINT = 3,
    FAULT_OVERFLOW = 4,
    FAULT_BOUND_RANGE_EXCEED = 5,
    FAULT_INVALID_OPCODE = 6,
    FAULT_DEVICE_NOT_AVAILABLE = 7,
    FAULT_DOUBLE_FAULT = 8,
    FAULT_9_RESERVED = 9,
    FAULT_INVALID_TSS = 10,
    FAULT_SEGMENT_NOT_PRESENT = 11,
    FAULT_STACK_FAULT = 12,
    FAULT_GENERAL_PROTECTION = 13,
    FAULT_PAGE_FAULT = 14,
    FAULT_15_RESERVED = 15,
    FAULT_FPU_ERROR = 16,
    FAULT_ALIGNMENT_CHECK = 17,
    FAULT_MACHINE_CHECK = 18,
    FAULT_SIMD_FLOATING_POINT = 19,
    FAULT_VIRTUALIZATION = 20,
    FAULT_CONTROL_PROTECTION = 21,
    FAULT_22_RESERVED = 22,
    FAULT_23_RESERVED = 23,
    FAULT_24_RESERVED = 24,
    FAULT_25_RESERVED = 25,
    FAULT_26_RESERVED = 26,
    FAULT_27_RESERVED = 27,
    FAULT_28_RESERVED = 28,
    FAULT_29_RESERVED = 29,
    FAULT_30_RESERVED = 30,
    FAULT_31_RESERVED = 31,
    // User defined faults start here
    FAULT_USER = 32,
    FAULT_SYSCALL = 33,

    // Keep this to know how many we have defined
    FAULT_NUMS
} Fault;

static string faultStrings[FAULT_NUMS] = {
    STRING("Divide Error"),
    STRING("Debug"),
    STRING("Non-Maskable Interrupt"),
    STRING("Breakpoint"),
    STRING("Overflow"),
    STRING("Bound Range Exceeded"),
    STRING("Invalid Opcode"),
    STRING("Device Not Available"),
    STRING("Double Fault"),
    STRING("Reserved fault"),
    STRING("Invalid TSS"),
    STRING("Segment Not Present"),
    STRING("Stack-Segment Fault"),
    STRING("General Protection"),
    STRING("Page Fault"),
    STRING("Reserved fault"),
    STRING("x87 FPU Floating-Point Error"),
    STRING("Alignment Check"),
    STRING("Machine Check"),
    STRING("SIMD Floating-Point Exception"),
    STRING("Virtualization Exception"),
    STRING("Control Protection Exception"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("Reserved fault"),
    STRING("User Defined"),
    STRING("System Call")};

typedef struct {
    // Segment selectors with alignment attributes
    uint16_t gs __attribute__((aligned(8)));
    uint16_t fs __attribute__((aligned(8)));
    uint16_t es __attribute__((aligned(8)));
    uint16_t ds __attribute__((aligned(8)));

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;

    Fault int_no;
    uint64_t err_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    // Never having these because we are not ever planning to switch privileges
    //    uint64_t useresp;
    //    uint64_t ss;
} regs __attribute__((aligned(8)));

void fault_handler(regs *regs) {
    FLUSH_AFTER {
        LOG(STRING("ISR fault handler triggered!\n"));
        LOG(STRING("Interrupt #: "));
        LOG(regs->int_no);
        LOG(STRING(", Interrupt Message: "));
        LOG(faultStrings[regs->int_no], NEWLINE);
        LOG(STRING("Error code: "));
        LOG(regs->err_code, NEWLINE);
        LOG(STRING("Halting...\n"));
    }
    __asm__ __volatile__("cli;hlt;");
}
