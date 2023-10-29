/* externs.h: declarations for global variables and initialized data

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#ifndef MSDOS
extern int errno;
#endif

extern char *copyright[5];

/* horrible hack: needed because compact_monster() can be called from deep
   within creatures() via place_monster() and summon_monster() */
extern int hack_monptr;

extern int16 log_index;		/* Index to log file. -CJS- */
extern vtype died_from;
extern vtype savefile;			/* The save file. -CJS- */

/* These are options, set with set_options command -CJS- */
extern int rogue_like_commands;
extern int find_cut;			/* Cut corners on a run */
extern int find_examine;		/* Check corners on a run */
extern int find_prself;			/* Print yourself on a run (slower) */
extern int find_bound;			/* Stop run when the map shifts */
extern int prompt_carry_flag;		/* Prompt to pick something up */
extern int show_weight_flag;		/* Display weights in inventory */
extern int highlight_seams;		/* Highlight magma and quartz */
extern int find_ignore_doors;		/* Run through open doors */

/* global flags */
extern int new_level_flag;	  /* Next level when true  */
extern int search_flag;	      /* Player is searching   */
extern int teleport_flag;	/* Handle teleport traps  */
extern int eof_flag;		/* Used to handle eof/HANGUP */
extern int player_light;      /* Player carrying light */
extern int find_flag;	/* Used in MORIA	      */
extern int free_turn_flag;	/* Used in MORIA	      */
extern int weapon_heavy;	/* Flag if the weapon too heavy -CJS- */
extern int pack_heavy;		/* Flag if the pack too heavy -CJS- */
extern char doing_inven;	/* Track inventory commands */
extern int screen_change;	/* Screen changes (used in inven_commands) */

extern int character_generated;	 /* don't save score until char gen finished */
extern int character_saved;	 /* prevents save on kill after save_char() */
extern int highscore_fd;	/* High score file descriptor */
extern int command_count;	/* Repetition of commands. -CJS- */
extern int default_dir;		/* Use last direction in repeated commands */
extern int16 noscore;		/* Don't score this game. -CJS- */
extern int32u randes_seed;    /* For encoding colors */
extern int32u town_seed;	    /* Seed for town genera*/
extern int16 dun_level;	/* Cur dungeon level   */
extern int16 missile_ctr;	/* Counter for missiles */
extern int msg_flag;	/* Set with first msg  */
extern vtype old_msg[MAX_SAVE_MSG];	/* Last messages -CJS- */
extern int16 last_msg;			/* Where in the array is the last */
extern int death;	/* True if died	      */
extern int32 turn;	/* Cur trun of game    */
extern int wizard;	/* Wizard flag	      */
extern int to_be_wizard;
extern int16 panic_save; /* this is true if playing from a panic save */

extern int wait_for_more;

extern char days[7][29];
extern int closing_flag;	/* Used for closing   */

extern int16 cur_height, cur_width;	/* Cur dungeon size    */
/*  Following are calculated from max dungeon sizes		*/
extern int16 max_panel_rows, max_panel_cols;
extern int panel_row, panel_col;
extern int panel_row_min, panel_row_max;
extern int panel_col_min, panel_col_max;
extern int panel_col_prt, panel_row_prt;

/*  Following are all floor definitions				*/
#ifdef MAC
extern cave_type (*cave)[MAX_WIDTH];
#else
extern cave_type cave[MAX_HEIGHT][MAX_WIDTH];
#endif

/* Following are player variables				*/
extern player_type py;
#ifdef MACGAME
extern char *(*player_title)[MAX_PLAYER_LEVEL];
extern race_type *race;
extern background_type *background;
#else
extern char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];
extern race_type race[MAX_RACES];
extern background_type background[MAX_BACKGROUND];
#endif
extern int32u player_exp[MAX_PLAYER_LEVEL];
extern int16u player_hp[MAX_PLAYER_LEVEL];
extern int16 char_row;
extern int16 char_col;

extern char *dsp_race[MAX_RACES];	/* Short strings for races. -CJS- */
extern int8u rgold_adj[MAX_RACES][MAX_RACES];

extern class_type class[MAX_CLASS];
extern int16 class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

/* Warriors don't have spells, so there is no entry for them. */
#ifdef MACGAME
extern spell_type (*magic_spell)[31];
#else
extern spell_type magic_spell[MAX_CLASS-1][31];
#endif
extern char *spell_names[62];
extern int32u spell_learned;	/* Bit field for spells learnt -CJS- */
extern int32u spell_worked;	/* Bit field for spells tried -CJS- */
extern int32u spell_forgotten;	/* Bit field for spells forgotten -JEW- */
extern int8u spell_order[32];	/* remember order that spells are learned in */
extern int16u player_init[MAX_CLASS][5];
extern int16 total_winner;

/* Following are store definitions				*/
#ifdef MACGAME
extern owner_type *owners;
#else
extern owner_type owners[MAX_OWNERS];
#endif
#ifdef MAC
extern store_type *store;
#else
extern store_type store[MAX_STORES];
#endif
extern int16u store_choice[MAX_STORES][STORE_CHOICES];
#ifndef MAC
extern int (*store_buy[MAX_STORES])();
#endif

/* Following are treasure arrays	and variables			*/
#ifdef MACGAME
extern treasure_type *object_list;
#else
extern treasure_type object_list[MAX_OBJECTS];
#endif
extern int8u object_ident[OBJECT_IDENT_SIZE];
extern int16 t_level[MAX_OBJ_LEVEL+1];
extern inven_type t_list[MAX_TALLOC];
extern inven_type inventory[INVEN_ARRAY_SIZE];
extern char *special_names[SN_ARRAY_SIZE];
extern int16 sorted_objects[MAX_DUNGEON_OBJ];
extern int16 inven_ctr;		/* Total different obj's	*/
extern int16 inven_weight;	/* Cur carried weight	*/
extern int16 equip_ctr;	/* Cur equipment ctr	*/
extern int16 tcptr;	/* Cur treasure heap ptr	*/

/* Following are creature arrays and variables			*/
#ifdef MACGAME
extern creature_type *c_list;
#else
extern creature_type c_list[MAX_CREATURES];
#endif
extern monster_type m_list[MAX_MALLOC];
extern int16 m_level[MAX_MONS_LEVEL+1];
extern m_attack_type monster_attacks[N_MONS_ATTS];
#ifdef MAC
extern recall_type *c_recall;
#else
extern recall_type c_recall[MAX_CREATURES];	/* Monster memories. -CJS- */
#endif
extern monster_type blank_monster;	/* Blank monster values	*/
extern int16 mfptr;	/* Cur free monster ptr	*/
extern int16 mon_tot_mult;	/* # of repro's of creature	*/

/* Following are arrays for descriptive pieces			*/
#ifdef MACGAME
extern char **colors;
extern char **mushrooms;
extern char **woods;
extern char **metals;
extern char **rocks;
extern char **amulets;
extern char **syllables;
#else
extern char *colors[MAX_COLORS];
extern char *mushrooms[MAX_MUSH];
extern char *woods[MAX_WOODS];
extern char *metals[MAX_METALS];
extern char *rocks[MAX_ROCKS];
extern char *amulets[MAX_AMULETS];
extern char *syllables[MAX_SYLLABLES];
#endif

extern int8u blows_table[7][6];

extern int16u normal_table[NORMAL_TABLE_SIZE];

/* Initialized data which had to be moved from some other file */
/* Since these get modified, macrsrc.c must be able to access them */
/* Otherwise, game cannot be made restartable */
/* dungeon.c */
extern char last_command;  /* Memory of previous command. */
/* moria1.c */
/* Track if temporary light about player.  */
extern int light_flag;

#ifdef MSDOS
extern int8u	floorsym, wallsym;
extern int	ansi, saveprompt;
extern char	moriatop[], moriasav[];
#endif

#ifdef unix
/* call functions which expand tilde before calling open/fopen */
#define open topen
#define fopen tfopen
#endif
