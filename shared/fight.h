#ifndef SHARED_FIGHT_H
#define SHARED_FIGHT_H
/* The fight with the kami: the numbers, and what the panel says about it.
 *
 * The arithmetic itself stays in each version. On the C64 a shared routine that
 * hands its results back through pointers cost 371 bytes more than the two
 * lines it replaces (measured); the numbers and the wording are what must not
 * drift. A text is cut at the figures, because each version prints them its
 * own way (PC: printf, C64: by hand). */
#define PLAYER_HP 24
#define PLAYER_ATTACK 8
#define PLAYER_DEFENSE 2
#define KAMI_HP 24
#define KAMI_ATTACK 6
#define KAMI_DEFENSE 2

#define FIGHT_NO_HERB_TEXT "Kein Heilkraut benutzt. Waehle neu."
/* "Dein Hieb verursacht <hit> Schaden." and what follows */
#define FIGHT_HIT_TEXT "Dein Hieb verursacht "
#define FIGHT_THEN_TEXT " Schaden.\nDer Kami verursacht "
#define FIGHT_WON_TEXT " Schaden.\nDer Kami sinkt in sich zusammen."
/* "Das Kraut lindert deine Wunden. Der Kami verursacht <taken> Schaden." */
#define FIGHT_HERB_TEXT "Das Kraut lindert deine Wunden.\nDer Kami verursacht "
#define FIGHT_END_TEXT " Schaden."
#endif
