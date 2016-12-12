#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define MAX_BOMBS 1000

struct options {
	int overall, alien, shots, bombs, bombchance;
};

void menu(struct options *settings) {
	char option, buf[30];
	int new;

	clear();
	echo();
	nocbreak();
	nodelay(stdscr, 0);

	move(0, 0);
	addstr("1. Change overall game speed");
	move(1, 0);
	addstr("2. Change alien motion speed");
	move(2, 0);
	addstr("3. Change tank shot speed");
	move(3, 0);
	addstr("4. Change alien bomb speed");
	move(4, 0);
	addstr("5. Change alien bomb dropping frequency");
	move(5, 0);
	addstr("6. Return to the game");
	move(6, 0);
	addstr("7. Exit the game");
	move(8, 0);
	addstr("Enter your option: ");
	refresh();

	while (1) {
		move(8, 19);
		option = getch();
		move(9, 0);
		deleteln();
		move(10, 0);
		deleteln();
		move(11, 0);
		deleteln();

		if (option == '1') {
			sprintf(buf, "Current value: %d\n", settings->overall);

			move(9, 0);
			addstr(buf);
			move(10, 0);
			addstr("Enter new value: ");
			move(10, 17);
			refresh();
			getch();
			getstr(buf);

			new = atoi(buf);

			/* 유효값인지 검사 */
			if (new < 0) {
				move(11, 0);
				addstr("ERROR: Inalid value");
			}
			else {
				settings->overall = new;
			}
		}
		else if (option == '2') {
			sprintf(buf, "Current value: %d\n", settings->alien);

			move(9, 0);
			addstr(buf);
			move(10, 0);
			addstr("Enter new value: ");
			move(10, 17);
			refresh();
			getch();
			getstr(buf);

			new = atoi(buf);

			/* 유효값 검사 */
			if (new <= 0) {
				move(11, 0);
				addstr("ERROR: Inalid value");
			}
			else {
				settings->alien = new;
			}
		}
		else if (option == '3') {
			sprintf(buf, "Current value: %d\n", settings->shots);

			move(9, 0);
			addstr(buf);
			move(10, 0);
			addstr("Enter new value: ");
			move(10, 17);
			refresh();
			getch();
			getstr(buf);

			new = atoi(buf);

			/* 유효값 검사 */
			if (new <= 0) {
				move(11, 0);
				addstr("ERROR: Inalid value");
			}
			else {
				settings->shots = new;
			}
		}
		else if (option == '4') {
			sprintf(buf, "Current value: %d\n", settings->bombs);

			move(9, 0);
			addstr(buf);
			move(10, 0);
			addstr("Enter new value: ");
			move(10, 17);
			refresh();
			getch();
			getstr(buf);

			new = atoi(buf);

			/* 유효값 검사 */
			if (new <= 0) {
				move(11, 0);
				addstr("ERROR: Inalid value");
			}
			else {
				settings->bombs = new;
			}
		}
		else if (option == '5') {
			sprintf(buf, "Current value: %d\n", settings->bombchance);

			move(9, 0);
			addstr(buf);
			move(10, 0);
			addstr("Enter new value: ");
			move(10, 17);
			refresh();
			getch();
			getstr(buf);

			new = atoi(buf);

			/* 유효값 검사 */
			if (new > 100 || new < 0) {
				move(11, 0);
				addstr("ERROR: Inalid value");
			}
			else {
				settings->bombchance = new;
			}
		}
		else if (option == '6') {
			break;
		}
		else if (option == '7') {
			endwin();
			exit(0);
		}
		else {
			move(9, 0);
			addstr("ERROR: Invalid selection");
			move(8, 19);
			addstr(" ");
			refresh();
		}
	}

	clear();
	noecho();
	cbreak();
	nodelay(stdscr, 1);

	move(0, (COLS / 2) - 9);
	addstr("--SPACE INVADERS--");
	move(0, 1);
	addstr("SCORE: ");
	move(0, COLS - 19);
	addstr("m = menu  q = quit");
}
void gameover(int win);

struct player {
	int r, c;
	char ch;
};

struct alien {
	int r, c;
	int pr, pc;
	int alive; /* 1=생존 0=파괴 */
	char direction; /* 'l'=left 'r'=right */
	char ch;
};

struct shoot {
	int r, c;
	int active; /* 1=생존 0=파괴 */
	char ch;
};

struct bomb {
	int r, c;
	int active; /* 1=생존 0=파괴 */
	int loop; /* 폭탄이 떨어질때 적이 깜빡거림을 방지 */
	char ch;
};



/* 메인 함수. 유저 input, 게임화면, 승리/패배 상태를 체크한다 */
int main() {
	struct player tank;
	struct alien aliens[30];
	struct shoot shot[3];
	struct bomb bomb[MAX_BOMBS];
	struct options settings;
	unsigned int input, loops = 0, i = 0, j = 0, currentshots = 0, currentbombs = 0, currentaliens = 30;
	int random = 0, score = 0, win = -1, life = 30;
	char tellscore[30];
	char tellLife[30];

	initscr();
	clear();
	noecho();
	cbreak();
	nodelay(stdscr, 1);
	keypad(stdscr, 1);
	srand(time(NULL));

	/* 초기값 셋팅 */
	settings.overall = 15000;
	settings.alien = 12;
	settings.shots = 3;
	settings.bombs = 10;
	settings.bombchance = 5;

	/* 아군 셋팅 */
	tank.r = LINES - 1;
	tank.c = COLS / 2;
	tank.ch = "A";

	/* 적군 셋팅 */
	for (i = 0; i<10; ++i) {
		aliens[i].r = 1;
		aliens[i].c = i * 3;
		aliens[i].pr = 0;
		aliens[i].pc = 0;
		aliens[i].ch = '@';
		aliens[i].alive = 1;
		aliens[i].direction = 'r';
	}
	for (i = 10; i<20; ++i) {
		aliens[i].r = 2;
		aliens[i].c = (i - 10) * 3;
		aliens[i].pr = 0;
		aliens[i].pc = 0;
		aliens[i].ch = '#';
		aliens[i].alive = 1;
		aliens[i].direction = 'r';
	}
	for (i = 20; i<30; ++i) {
		aliens[i].r = 3;
		aliens[i].c = (i - 20) * 3;
		aliens[i].pr = 0;
		aliens[i].pc = 0;
		aliens[i].ch = '$';
		aliens[i].alive = 1;
		aliens[i].direction = 'r';
	}

	/* 아군 총알발사 셋팅 */
	for (i = 0; i<3; ++i) {
		shot[i].active = 0;
		shot[i].ch = '*';
	}

	/* 적군 총알발사 셋팅 */
	for (i = 0; i<MAX_BOMBS; ++i) {
		bomb[i].active = 0;
		bomb[i].ch = 'o';
		bomb[i].loop = 0;
	}

	/* 화면 정보표시 */
	move(0, (COLS / 2) - 9);
	addstr("--SPACE INVADERS--");
	move(0, 1);
	addstr("SCORE: ");
	move(0, 13);
	addstr("HP: ");
	move(0, COLS - 19);
	addstr("m = menu  q = quit");

	while (1) {
		/* 점수 표시 */
		sprintf(tellscore, "%d", score);
		move(0, 8);
		addstr(tellscore);
		
		sprintf(tellLife, "%d", life);
		move(0, 17);
		addstr(tellLife);
		

		/* 아군 이동 */
		move(tank.r, tank.c);
		addch(tank.ch);

		/* 적군 총알 이동 */
		if (loops % settings.bombs == 0)
			for (i = 0; i<MAX_BOMBS; ++i) {
				if (bomb[i].active == 1) {
					if (bomb[i].r < LINES) {
						if (bomb[i].loop != 0) {
							move(bomb[i].r - 1, bomb[i].c);
							addch(' ');
						}
						else
							++bomb[i].loop;

						move(bomb[i].r, bomb[i].c);
						addch(bomb[i].ch);

						++bomb[i].r;
					}
					else {
						bomb[i].active = 0;
						bomb[i].loop = 0;
						--currentbombs;
						move(bomb[i].r - 1, bomb[i].c);
						addch(' ');
					}
				}
			}

		/* Move shots */
		if (loops % settings.shots == 0)
			for (i = 0; i<3; ++i) {
				if (shot[i].active == 1) {
					if (shot[i].r > 0) {
						if (shot[i].r < LINES - 2) {
							move(shot[i].r + 1, shot[i].c);
							addch(' ');
						}

						for (j = 0; j<30; ++j) {
							if (aliens[j].alive == 1 && aliens[j].r == shot[i].r && aliens[j].pc == shot[i].c) {
								score += 20;
								aliens[j].alive = 0;
								shot[i].active = 0;
								--currentshots;
								--currentaliens;
								move(aliens[j].pr, aliens[j].pc);
								addch(' ');
								break;
							}
						}

						if (shot[i].active == 1) {
							move(shot[i].r, shot[i].c);
							addch(shot[i].ch);

							--shot[i].r;
						}
					}
					else {
						shot[i].active = 0;
						--currentshots;
						move(shot[i].r + 1, shot[i].c);
						addch(' ');
					}
				}
			}

		/* Move aliens */
		if (loops % settings.alien == 0)
			for (i = 0; i<30; ++i) {
				if (aliens[i].alive == 1) {
					move(aliens[i].pr, aliens[i].pc);
					addch(' ');

					move(aliens[i].r, aliens[i].c);
					addch(aliens[i].ch);

					aliens[i].pr = aliens[i].r;
					aliens[i].pc = aliens[i].c;

					/* Check if alien should drop bomb */
					random = 1 + (rand() % 100);
					if ((settings.bombchance - random) >= 0 && currentbombs < MAX_BOMBS) {
						for (j = 0; j<MAX_BOMBS; ++j) {
							if (bomb[j].active == 0) {
								bomb[j].active = 1;
								++currentbombs;
								bomb[j].r = aliens[i].r + 1;
								bomb[j].c = aliens[i].c;
								break;
							}
						}
					}

					/* Set alien's next position */
					if (aliens[i].direction == 'l')
						--aliens[i].c;
					else if (aliens[i].direction == 'r')
						++aliens[i].c;

					/* Check alien's next positions */
					if (aliens[i].c == COLS - 2) {
						++aliens[i].r;
						aliens[i].direction = 'l';
					}
					else if (aliens[i].c == 0) {
						++aliens[i].r;
						aliens[i].direction = 'r';
					}
				}
			}


		/* See if game has been won or lost*/
		if (currentaliens == 0) {
			win = 1;
			break;
		}
		for (i = 0; i<30; ++i) {
			if (aliens[i].r == LINES - 1) {
				win = 0;
				break;
			}
		}
		for (i = 0; i<MAX_BOMBS; ++i) {
			if (bomb[i].r == tank.r && bomb[i].c == tank.c) {
				life -= 1;
			}
		}
		
		if (life == 0) {
			win = 0;
			break;
		}

		move(0, COLS - 1);
		refresh();
		usleep(settings.overall);
		++loops;

		input = getch();
		move(tank.r, tank.c);
		addch(' ');

		/* Check input */
		if (input == 'q')
			win = 2;
		else if (input == KEY_LEFT)
			--tank.c;
		else if (input == KEY_RIGHT)
			++tank.c;
		else if (input == ' ' && currentshots < 3) {
			for (i = 0; i<3; ++i) {
				if (shot[i].active == 0) {
					shot[i].active = 1;
					++currentshots;
					--score;
					shot[i].r = LINES - 2;
					shot[i].c = tank.c;
					break;
				}
			}
		}
		else if (input == 'm')
			menu(&settings);

		if (win != -1)
			break;

		/* Check for valid tank position */
		if (tank.c > COLS - 2)
			tank.c = COLS - 2;
		else if (tank.c < 0)
			tank.c = 0;
	}

	gameover(win);
	endwin();
	return 0;
}

/* This function handles the menu options available to the user */


/* This function handles displaying the win/lose screen */
void gameover(int win) {

	nodelay(stdscr, 0);

	if (win == 0) {
		clear();
		move((LINES / 2) - 1, (COLS / 2) - 5);
		addstr("YOU LOSE!");
		move((LINES / 2), (COLS / 2) - 11);
		addstr("PRESS ANY KEY TO EXIT");
		move(0, COLS - 1);
		refresh();
		getch();
	}

	else if (win == 1) {
		clear();
		move((LINES / 2) - 1, (COLS / 2) - 5);
		addstr("YOU WIN!");
		move((LINES / 2), (COLS / 2) - 11);
		addstr("PRESS ANY KEY TO EXIT");
		move(0, COLS - 1);
		refresh();
		getch();
	}
}
