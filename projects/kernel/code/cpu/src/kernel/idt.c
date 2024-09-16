#include "cpu/idt.h"
#include "log/log.h" // for LOG, LOG_CHOOSER_IMPL_1, LOG_CHOOSER...
#include "memory/manipulation/manipulation.h"
#include "text/string.h" // for STRING, string

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

U8 gateFlags[GATE_NUM] = {0x8E, 0x8F};

static void idt_set_gate(U8 num, U64 base, GateType gateType) {
    idt[num].offset_1 = (base & 0xFFFF);
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;

    // Referring to the GDT segment here
    idt[num].selector = 0x08;

    idt[num].type_attributes = gateFlags[gateType];

    idt[num].ist = 0;
    idt[num].zero = 0;
}

void initIDT() {
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof(InterruptDescriptor) * 256) - 1;
    idtp.base = (U64)&idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(InterruptDescriptor) * 256);

    idt_set_gate(0, (U64)isr0, INTERRUPT_GATE);
    idt_set_gate(1, (U64)isr1, INTERRUPT_GATE);
    idt_set_gate(2, (U64)isr2, INTERRUPT_GATE);
    idt_set_gate(3, (U64)isr3, INTERRUPT_GATE);
    idt_set_gate(4, (U64)isr4, INTERRUPT_GATE);
    idt_set_gate(5, (U64)isr5, INTERRUPT_GATE);
    idt_set_gate(6, (U64)isr6, INTERRUPT_GATE);
    idt_set_gate(7, (U64)isr7, INTERRUPT_GATE);
    idt_set_gate(8, (U64)isr8, INTERRUPT_GATE);
    idt_set_gate(9, (U64)isr9, INTERRUPT_GATE);
    idt_set_gate(10, (U64)isr10, INTERRUPT_GATE);
    idt_set_gate(11, (U64)isr11, INTERRUPT_GATE);
    idt_set_gate(12, (U64)isr12, INTERRUPT_GATE);
    idt_set_gate(13, (U64)isr13, INTERRUPT_GATE);
    idt_set_gate(14, (U64)isr14, INTERRUPT_GATE);
    idt_set_gate(15, (U64)isr15, INTERRUPT_GATE);
    idt_set_gate(16, (U64)isr16, INTERRUPT_GATE);
    idt_set_gate(17, (U64)isr17, INTERRUPT_GATE);
    idt_set_gate(18, (U64)isr18, INTERRUPT_GATE);
    idt_set_gate(19, (U64)isr19, INTERRUPT_GATE);
    idt_set_gate(20, (U64)isr20, INTERRUPT_GATE);
    idt_set_gate(21, (U64)isr21, INTERRUPT_GATE);
    idt_set_gate(22, (U64)isr22, INTERRUPT_GATE);
    idt_set_gate(23, (U64)isr23, INTERRUPT_GATE);
    idt_set_gate(24, (U64)isr24, INTERRUPT_GATE);
    idt_set_gate(25, (U64)isr25, INTERRUPT_GATE);
    idt_set_gate(26, (U64)isr26, INTERRUPT_GATE);
    idt_set_gate(27, (U64)isr27, INTERRUPT_GATE);
    idt_set_gate(28, (U64)isr28, INTERRUPT_GATE);
    idt_set_gate(29, (U64)isr29, INTERRUPT_GATE);
    idt_set_gate(30, (U64)isr30, INTERRUPT_GATE);
    idt_set_gate(31, (U64)isr31, INTERRUPT_GATE);
    idt_set_gate(32, (U64)isr32, INTERRUPT_GATE);
    idt_set_gate(33, (U64)isr33, INTERRUPT_GATE);
    idt_set_gate(34, (U64)isr34, INTERRUPT_GATE);
    idt_set_gate(35, (U64)isr35, INTERRUPT_GATE);
    idt_set_gate(36, (U64)isr36, INTERRUPT_GATE);
    idt_set_gate(37, (U64)isr37, INTERRUPT_GATE);
    idt_set_gate(38, (U64)isr38, INTERRUPT_GATE);
    idt_set_gate(39, (U64)isr39, INTERRUPT_GATE);
    idt_set_gate(40, (U64)isr40, INTERRUPT_GATE);
    idt_set_gate(41, (U64)isr41, INTERRUPT_GATE);
    idt_set_gate(42, (U64)isr42, INTERRUPT_GATE);
    idt_set_gate(43, (U64)isr43, INTERRUPT_GATE);
    idt_set_gate(44, (U64)isr44, INTERRUPT_GATE);
    idt_set_gate(45, (U64)isr45, INTERRUPT_GATE);
    idt_set_gate(46, (U64)isr46, INTERRUPT_GATE);
    idt_set_gate(47, (U64)isr47, INTERRUPT_GATE);
    idt_set_gate(48, (U64)isr48, INTERRUPT_GATE);
    idt_set_gate(49, (U64)isr49, INTERRUPT_GATE);
    idt_set_gate(50, (U64)isr50, INTERRUPT_GATE);
    idt_set_gate(51, (U64)isr51, INTERRUPT_GATE);
    idt_set_gate(52, (U64)isr52, INTERRUPT_GATE);
    idt_set_gate(53, (U64)isr53, INTERRUPT_GATE);
    idt_set_gate(54, (U64)isr54, INTERRUPT_GATE);
    idt_set_gate(55, (U64)isr55, INTERRUPT_GATE);
    idt_set_gate(56, (U64)isr56, INTERRUPT_GATE);
    idt_set_gate(57, (U64)isr57, INTERRUPT_GATE);
    idt_set_gate(58, (U64)isr58, INTERRUPT_GATE);
    idt_set_gate(59, (U64)isr59, INTERRUPT_GATE);
    idt_set_gate(60, (U64)isr60, INTERRUPT_GATE);
    idt_set_gate(61, (U64)isr61, INTERRUPT_GATE);
    idt_set_gate(62, (U64)isr62, INTERRUPT_GATE);
    idt_set_gate(63, (U64)isr63, INTERRUPT_GATE);
    idt_set_gate(64, (U64)isr64, INTERRUPT_GATE);
    idt_set_gate(65, (U64)isr65, INTERRUPT_GATE);
    idt_set_gate(66, (U64)isr66, INTERRUPT_GATE);
    idt_set_gate(67, (U64)isr67, INTERRUPT_GATE);
    idt_set_gate(68, (U64)isr68, INTERRUPT_GATE);
    idt_set_gate(69, (U64)isr69, INTERRUPT_GATE);
    idt_set_gate(70, (U64)isr70, INTERRUPT_GATE);
    idt_set_gate(71, (U64)isr71, INTERRUPT_GATE);
    idt_set_gate(72, (U64)isr72, INTERRUPT_GATE);
    idt_set_gate(73, (U64)isr73, INTERRUPT_GATE);
    idt_set_gate(74, (U64)isr74, INTERRUPT_GATE);
    idt_set_gate(75, (U64)isr75, INTERRUPT_GATE);
    idt_set_gate(76, (U64)isr76, INTERRUPT_GATE);
    idt_set_gate(77, (U64)isr77, INTERRUPT_GATE);
    idt_set_gate(78, (U64)isr78, INTERRUPT_GATE);
    idt_set_gate(79, (U64)isr79, INTERRUPT_GATE);
    idt_set_gate(80, (U64)isr80, INTERRUPT_GATE);
    idt_set_gate(81, (U64)isr81, INTERRUPT_GATE);
    idt_set_gate(82, (U64)isr82, INTERRUPT_GATE);
    idt_set_gate(83, (U64)isr83, INTERRUPT_GATE);
    idt_set_gate(84, (U64)isr84, INTERRUPT_GATE);
    idt_set_gate(85, (U64)isr85, INTERRUPT_GATE);
    idt_set_gate(86, (U64)isr86, INTERRUPT_GATE);
    idt_set_gate(87, (U64)isr87, INTERRUPT_GATE);
    idt_set_gate(88, (U64)isr88, INTERRUPT_GATE);
    idt_set_gate(89, (U64)isr89, INTERRUPT_GATE);
    idt_set_gate(90, (U64)isr90, INTERRUPT_GATE);
    idt_set_gate(91, (U64)isr91, INTERRUPT_GATE);
    idt_set_gate(92, (U64)isr92, INTERRUPT_GATE);
    idt_set_gate(93, (U64)isr93, INTERRUPT_GATE);
    idt_set_gate(94, (U64)isr94, INTERRUPT_GATE);
    idt_set_gate(95, (U64)isr95, INTERRUPT_GATE);
    idt_set_gate(96, (U64)isr96, INTERRUPT_GATE);
    idt_set_gate(97, (U64)isr97, INTERRUPT_GATE);
    idt_set_gate(98, (U64)isr98, INTERRUPT_GATE);
    idt_set_gate(99, (U64)isr99, INTERRUPT_GATE);
    idt_set_gate(100, (U64)isr100, INTERRUPT_GATE);
    idt_set_gate(101, (U64)isr101, INTERRUPT_GATE);
    idt_set_gate(102, (U64)isr102, INTERRUPT_GATE);
    idt_set_gate(103, (U64)isr103, INTERRUPT_GATE);
    idt_set_gate(104, (U64)isr104, INTERRUPT_GATE);
    idt_set_gate(105, (U64)isr105, INTERRUPT_GATE);
    idt_set_gate(106, (U64)isr106, INTERRUPT_GATE);
    idt_set_gate(107, (U64)isr107, INTERRUPT_GATE);
    idt_set_gate(108, (U64)isr108, INTERRUPT_GATE);
    idt_set_gate(109, (U64)isr109, INTERRUPT_GATE);
    idt_set_gate(110, (U64)isr110, INTERRUPT_GATE);
    idt_set_gate(111, (U64)isr111, INTERRUPT_GATE);
    idt_set_gate(112, (U64)isr112, INTERRUPT_GATE);
    idt_set_gate(113, (U64)isr113, INTERRUPT_GATE);
    idt_set_gate(114, (U64)isr114, INTERRUPT_GATE);
    idt_set_gate(115, (U64)isr115, INTERRUPT_GATE);
    idt_set_gate(116, (U64)isr116, INTERRUPT_GATE);
    idt_set_gate(117, (U64)isr117, INTERRUPT_GATE);
    idt_set_gate(118, (U64)isr118, INTERRUPT_GATE);
    idt_set_gate(119, (U64)isr119, INTERRUPT_GATE);
    idt_set_gate(120, (U64)isr120, INTERRUPT_GATE);
    idt_set_gate(121, (U64)isr121, INTERRUPT_GATE);
    idt_set_gate(122, (U64)isr122, INTERRUPT_GATE);
    idt_set_gate(123, (U64)isr123, INTERRUPT_GATE);
    idt_set_gate(124, (U64)isr124, INTERRUPT_GATE);
    idt_set_gate(125, (U64)isr125, INTERRUPT_GATE);
    idt_set_gate(126, (U64)isr126, INTERRUPT_GATE);
    idt_set_gate(127, (U64)isr127, INTERRUPT_GATE);
    idt_set_gate(128, (U64)isr128, INTERRUPT_GATE);
    idt_set_gate(129, (U64)isr129, INTERRUPT_GATE);
    idt_set_gate(130, (U64)isr130, INTERRUPT_GATE);
    idt_set_gate(131, (U64)isr131, INTERRUPT_GATE);
    idt_set_gate(132, (U64)isr132, INTERRUPT_GATE);
    idt_set_gate(133, (U64)isr133, INTERRUPT_GATE);
    idt_set_gate(134, (U64)isr134, INTERRUPT_GATE);
    idt_set_gate(135, (U64)isr135, INTERRUPT_GATE);
    idt_set_gate(136, (U64)isr136, INTERRUPT_GATE);
    idt_set_gate(137, (U64)isr137, INTERRUPT_GATE);
    idt_set_gate(138, (U64)isr138, INTERRUPT_GATE);
    idt_set_gate(139, (U64)isr139, INTERRUPT_GATE);
    idt_set_gate(140, (U64)isr140, INTERRUPT_GATE);
    idt_set_gate(141, (U64)isr141, INTERRUPT_GATE);
    idt_set_gate(142, (U64)isr142, INTERRUPT_GATE);
    idt_set_gate(143, (U64)isr143, INTERRUPT_GATE);
    idt_set_gate(144, (U64)isr144, INTERRUPT_GATE);
    idt_set_gate(145, (U64)isr145, INTERRUPT_GATE);
    idt_set_gate(146, (U64)isr146, INTERRUPT_GATE);
    idt_set_gate(147, (U64)isr147, INTERRUPT_GATE);
    idt_set_gate(148, (U64)isr148, INTERRUPT_GATE);
    idt_set_gate(149, (U64)isr149, INTERRUPT_GATE);
    idt_set_gate(150, (U64)isr150, INTERRUPT_GATE);
    idt_set_gate(151, (U64)isr151, INTERRUPT_GATE);
    idt_set_gate(152, (U64)isr152, INTERRUPT_GATE);
    idt_set_gate(153, (U64)isr153, INTERRUPT_GATE);
    idt_set_gate(154, (U64)isr154, INTERRUPT_GATE);
    idt_set_gate(155, (U64)isr155, INTERRUPT_GATE);
    idt_set_gate(156, (U64)isr156, INTERRUPT_GATE);
    idt_set_gate(157, (U64)isr157, INTERRUPT_GATE);
    idt_set_gate(158, (U64)isr158, INTERRUPT_GATE);
    idt_set_gate(159, (U64)isr159, INTERRUPT_GATE);
    idt_set_gate(160, (U64)isr160, INTERRUPT_GATE);
    idt_set_gate(161, (U64)isr161, INTERRUPT_GATE);
    idt_set_gate(162, (U64)isr162, INTERRUPT_GATE);
    idt_set_gate(163, (U64)isr163, INTERRUPT_GATE);
    idt_set_gate(164, (U64)isr164, INTERRUPT_GATE);
    idt_set_gate(165, (U64)isr165, INTERRUPT_GATE);
    idt_set_gate(166, (U64)isr166, INTERRUPT_GATE);
    idt_set_gate(167, (U64)isr167, INTERRUPT_GATE);
    idt_set_gate(168, (U64)isr168, INTERRUPT_GATE);
    idt_set_gate(169, (U64)isr169, INTERRUPT_GATE);
    idt_set_gate(170, (U64)isr170, INTERRUPT_GATE);
    idt_set_gate(171, (U64)isr171, INTERRUPT_GATE);
    idt_set_gate(172, (U64)isr172, INTERRUPT_GATE);
    idt_set_gate(173, (U64)isr173, INTERRUPT_GATE);
    idt_set_gate(174, (U64)isr174, INTERRUPT_GATE);
    idt_set_gate(175, (U64)isr175, INTERRUPT_GATE);
    idt_set_gate(176, (U64)isr176, INTERRUPT_GATE);
    idt_set_gate(177, (U64)isr177, INTERRUPT_GATE);
    idt_set_gate(178, (U64)isr178, INTERRUPT_GATE);
    idt_set_gate(179, (U64)isr179, INTERRUPT_GATE);
    idt_set_gate(180, (U64)isr180, INTERRUPT_GATE);
    idt_set_gate(181, (U64)isr181, INTERRUPT_GATE);
    idt_set_gate(182, (U64)isr182, INTERRUPT_GATE);
    idt_set_gate(183, (U64)isr183, INTERRUPT_GATE);
    idt_set_gate(184, (U64)isr184, INTERRUPT_GATE);
    idt_set_gate(185, (U64)isr185, INTERRUPT_GATE);
    idt_set_gate(186, (U64)isr186, INTERRUPT_GATE);
    idt_set_gate(187, (U64)isr187, INTERRUPT_GATE);
    idt_set_gate(188, (U64)isr188, INTERRUPT_GATE);
    idt_set_gate(189, (U64)isr189, INTERRUPT_GATE);
    idt_set_gate(190, (U64)isr190, INTERRUPT_GATE);
    idt_set_gate(191, (U64)isr191, INTERRUPT_GATE);
    idt_set_gate(192, (U64)isr192, INTERRUPT_GATE);
    idt_set_gate(193, (U64)isr193, INTERRUPT_GATE);
    idt_set_gate(194, (U64)isr194, INTERRUPT_GATE);
    idt_set_gate(195, (U64)isr195, INTERRUPT_GATE);
    idt_set_gate(196, (U64)isr196, INTERRUPT_GATE);
    idt_set_gate(197, (U64)isr197, INTERRUPT_GATE);
    idt_set_gate(198, (U64)isr198, INTERRUPT_GATE);
    idt_set_gate(199, (U64)isr199, INTERRUPT_GATE);
    idt_set_gate(200, (U64)isr200, INTERRUPT_GATE);
    idt_set_gate(201, (U64)isr201, INTERRUPT_GATE);
    idt_set_gate(202, (U64)isr202, INTERRUPT_GATE);
    idt_set_gate(203, (U64)isr203, INTERRUPT_GATE);
    idt_set_gate(204, (U64)isr204, INTERRUPT_GATE);
    idt_set_gate(205, (U64)isr205, INTERRUPT_GATE);
    idt_set_gate(206, (U64)isr206, INTERRUPT_GATE);
    idt_set_gate(207, (U64)isr207, INTERRUPT_GATE);
    idt_set_gate(208, (U64)isr208, INTERRUPT_GATE);
    idt_set_gate(209, (U64)isr209, INTERRUPT_GATE);
    idt_set_gate(210, (U64)isr210, INTERRUPT_GATE);
    idt_set_gate(211, (U64)isr211, INTERRUPT_GATE);
    idt_set_gate(212, (U64)isr212, INTERRUPT_GATE);
    idt_set_gate(213, (U64)isr213, INTERRUPT_GATE);
    idt_set_gate(214, (U64)isr214, INTERRUPT_GATE);
    idt_set_gate(215, (U64)isr215, INTERRUPT_GATE);
    idt_set_gate(216, (U64)isr216, INTERRUPT_GATE);
    idt_set_gate(217, (U64)isr217, INTERRUPT_GATE);
    idt_set_gate(218, (U64)isr218, INTERRUPT_GATE);
    idt_set_gate(219, (U64)isr219, INTERRUPT_GATE);
    idt_set_gate(220, (U64)isr220, INTERRUPT_GATE);
    idt_set_gate(221, (U64)isr221, INTERRUPT_GATE);
    idt_set_gate(222, (U64)isr222, INTERRUPT_GATE);
    idt_set_gate(223, (U64)isr223, INTERRUPT_GATE);
    idt_set_gate(224, (U64)isr224, INTERRUPT_GATE);
    idt_set_gate(225, (U64)isr225, INTERRUPT_GATE);
    idt_set_gate(226, (U64)isr226, INTERRUPT_GATE);
    idt_set_gate(227, (U64)isr227, INTERRUPT_GATE);
    idt_set_gate(228, (U64)isr228, INTERRUPT_GATE);
    idt_set_gate(229, (U64)isr229, INTERRUPT_GATE);
    idt_set_gate(230, (U64)isr230, INTERRUPT_GATE);
    idt_set_gate(231, (U64)isr231, INTERRUPT_GATE);
    idt_set_gate(232, (U64)isr232, INTERRUPT_GATE);
    idt_set_gate(233, (U64)isr233, INTERRUPT_GATE);
    idt_set_gate(234, (U64)isr234, INTERRUPT_GATE);
    idt_set_gate(235, (U64)isr235, INTERRUPT_GATE);
    idt_set_gate(236, (U64)isr236, INTERRUPT_GATE);
    idt_set_gate(237, (U64)isr237, INTERRUPT_GATE);
    idt_set_gate(238, (U64)isr238, INTERRUPT_GATE);
    idt_set_gate(239, (U64)isr239, INTERRUPT_GATE);
    idt_set_gate(240, (U64)isr240, INTERRUPT_GATE);
    idt_set_gate(241, (U64)isr241, INTERRUPT_GATE);
    idt_set_gate(242, (U64)isr242, INTERRUPT_GATE);
    idt_set_gate(243, (U64)isr243, INTERRUPT_GATE);
    idt_set_gate(244, (U64)isr244, INTERRUPT_GATE);
    idt_set_gate(245, (U64)isr245, INTERRUPT_GATE);
    idt_set_gate(246, (U64)isr246, INTERRUPT_GATE);
    idt_set_gate(247, (U64)isr247, INTERRUPT_GATE);
    idt_set_gate(248, (U64)isr248, INTERRUPT_GATE);
    idt_set_gate(249, (U64)isr249, INTERRUPT_GATE);
    idt_set_gate(250, (U64)isr250, INTERRUPT_GATE);
    idt_set_gate(251, (U64)isr251, INTERRUPT_GATE);
    idt_set_gate(252, (U64)isr252, INTERRUPT_GATE);
    idt_set_gate(253, (U64)isr253, INTERRUPT_GATE);
    idt_set_gate(254, (U64)isr254, INTERRUPT_GATE);
    idt_set_gate(255, (U64)isr255, INTERRUPT_GATE);

    asm_lidt(&idtp);
}

void triggerFault(Fault fault) {
    switch (fault) {
    case FAULT_DIVIDE_ERROR:
        __asm__ __volatile__("int $0x00" :::);
        break;
    case FAULT_DEBUG:
        __asm__ __volatile__("int $0x01" :::);
        break;
    case FAULT_NMI:
        __asm__ __volatile__("int $0x02" :::);
        break;
    case FAULT_BREAKPOINT:
        __asm__ __volatile__("int $0x03" :::);
        break;
    case FAULT_OVERFLOW:
        __asm__ __volatile__("int $0x04" :::);
        break;
    case FAULT_BOUND_RANGE_EXCEED:
        __asm__ __volatile__("int $0x05" :::);
        break;
    case FAULT_INVALID_OPCODE:
        __asm__ __volatile__("int $0x06" :::);
        break;
    case FAULT_DEVICE_NOT_AVAILABLE:
        __asm__ __volatile__("int $0x07" :::);
        break;
    case FAULT_DOUBLE_FAULT:
        __asm__ __volatile__("int $0x08" :::);
        break;
    case FAULT_9_RESERVED:
        __asm__ __volatile__("int $0x09" :::);
        break;
    case FAULT_INVALID_TSS:
        __asm__ __volatile__("int $0x0A" :::);
        break;
    case FAULT_SEGMENT_NOT_PRESENT:
        __asm__ __volatile__("int $0x0B" :::);
        break;
    case FAULT_STACK_FAULT:
        __asm__ __volatile__("int $0x0C" :::);
        break;
    case FAULT_GENERAL_PROTECTION:
        __asm__ __volatile__("int $0x0D" :::);
        break;
    case FAULT_PAGE_FAULT:
        __asm__ __volatile__("int $0x0E" :::);
        break;
    case FAULT_15_RESERVED:
        __asm__ __volatile__("int $0x0F" :::);
        break;
    case FAULT_FPU_ERROR:
        __asm__ __volatile__("int $0x10" :::);
        break;
    case FAULT_ALIGNMENT_CHECK:
        __asm__ __volatile__("int $0x11" :::);
        break;
    case FAULT_MACHINE_CHECK:
        __asm__ __volatile__("int $0x12" :::);
        break;
    case FAULT_SIMD_FLOATING_POINT:
        __asm__ __volatile__("int $0x13" :::);
        break;
    case FAULT_VIRTUALIZATION:
        __asm__ __volatile__("int $0x14" :::);
        break;
    case FAULT_CONTROL_PROTECTION:
        __asm__ __volatile__("int $0x15" :::);
        break;
    case FAULT_22_RESERVED:
        __asm__ __volatile__("int $0x16" :::);
        break;
    case FAULT_23_RESERVED:
        __asm__ __volatile__("int $0x17" :::);
        break;
    case FAULT_24_RESERVED:
        __asm__ __volatile__("int $0x18" :::);
        break;
    case FAULT_25_RESERVED:
        __asm__ __volatile__("int $0x19" :::);
        break;
    case FAULT_26_RESERVED:
        __asm__ __volatile__("int $0x1A" :::);
        break;
    case FAULT_27_RESERVED:
        __asm__ __volatile__("int $0x1B" :::);
        break;
    case FAULT_28_RESERVED:
        __asm__ __volatile__("int $0x1C" :::);
        break;
    case FAULT_29_RESERVED:
        __asm__ __volatile__("int $0x1D" :::);
        break;
    case FAULT_30_RESERVED:
        __asm__ __volatile__("int $0x1E" :::);
        break;
    case FAULT_31_RESERVED:
        __asm__ __volatile__("int $0x1F" :::);
        break;
    case FAULT_USER:
        __asm__ __volatile__("int $0x20" :::);
        break;
    case FAULT_SYSCALL:
        __asm__ __volatile__("int $0x21" :::);
        break;
    case FAULT_NO_MORE_PHYSICAL_MEMORY:
        __asm__ __volatile__("int $0x22" :::);
        break;
    case FAULT_OVERLAPPING_VIRTUAL_SEGMENTS:
        __asm__ __volatile__("int $0x23" :::);
        break;
    default:
        __asm__ __volatile__("int $0xFF" :::);
        break;
    }

    __builtin_unreachable();
}

typedef struct {
    // Segment selectors with alignment attributes
    U16 gs __attribute__((aligned(8)));
    U16 fs __attribute__((aligned(8)));
    U16 es __attribute__((aligned(8)));
    U16 ds __attribute__((aligned(8)));

    U64 r15;
    U64 r14;
    U64 r13;
    U64 r12;
    U64 r11;
    U64 r10;
    U64 r9;
    U64 r8;

    U64 rdi;
    U64 rsi;
    U64 rbp;
    U64 rbx;
    U64 rdx;
    U64 rcx;
    U64 rax;

    Fault int_no;
    U64 err_code;

    U64 rip;
    U64 cs;
    U64 eflags;
    // Never having these because we are not ever planning to switch privileges
    //    U64 useresp;
    //    U64 ss;
} regs __attribute__((aligned(8)));

void fault_handler(regs *regs) {
    FLUSH_AFTER {
        LOG(STRING("ISR fault handler triggered!\n"));
        LOG(STRING("Interrupt #: "));
        LOG(regs->int_no);
        LOG(STRING(", Interrupt Message: "));
        LOG(faultToString[regs->int_no], NEWLINE);
        LOG(STRING("Error code: "));
        LOG(regs->err_code, NEWLINE);
        LOG(STRING("Halting...\n"));
    }
    __asm__ __volatile__("cli;hlt;");
}
