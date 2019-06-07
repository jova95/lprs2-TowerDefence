/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *
 */

#include <stdio.h>

#include "xio.h"
#include "xil_exception.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include "maps.h"
#include "vga_periph_mem.h"
#include "towerdefence_sprites.h"
#include "platform.h"
#include "xparameters.h"
#include "towerdefence.h"

#define GRASS 'G'
#define DIRTPREVIOUS 'P'
#define DIRT 'D'
#define BUSH 'B'
#define LAKE1 '1'
#define LAKE2 '2'
#define LAKE3 '3'
#define LAKE4 '4'
#define LAKE5 '5'
#define LAKE6 '6'
#define HP1 '7'
#define HP2 '8'
#define HP3 '9'
#define HP4 '0'
#define CREEP 0
#define CREEP1 1
#define CREEP2 2
#define CREEP3 3
#define CREEP4 4
#define MOVEDCREEP 'c'
#define SPOT 'S'
#define SELECTEDSPOT 'X'
#define TOWER 'T'
#define TOWER2 't'
#define SELECTEDTOWER 'O'
#define SELECTEDTOWER2 'o'

#define MAXCREEPS 10
#define MAX_ROUTE_LENGTH 100

#define MAP_HEIGHT 15
#define MAP_WIDTH 20

#define NISTA 0
#define GORE 1
#define DOLE 2
#define LEVO 3
#define DESNO 4

#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010

bool endGame = false;

unsigned char map[SIZEROW][SIZECOLUMN] = {{[0 ... SIZEROW-1] = GRASS}, {[0 ... SIZECOLUMN-1] = GRASS}};

char lastKey = 'n';
int currentI = 0;

// structures
struct GameState {
    int coins;
    int current_level;
};

struct CreepRoute {
    unsigned char creep_x[MAX_ROUTE_LENGTH];
    unsigned char creep_y[MAX_ROUTE_LENGTH];
    int length;
};

struct TowerPosition {
    unsigned char tower_x[10];
    unsigned char tower_y[10];
    int numOfTowers;
};

int btnCnt = 0;


void init(){

	VGA_PERIPH_MEM_mWriteMemory(
				XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); // direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); // display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x0); // show frame      2
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0xff); // font size       3
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); // foreground 4
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14,0x008000); // background color 5
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); // frame color      6
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 1);
}

//extracting pixel data from a picture for printing out on the display
void drawSprite(int in_x, int in_y, int out_x, int out_y, int width, int height) {
	int x, y, ox, oy, oi, iy, ix, ii, R, G, B, RGB;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			ox = out_x + x;
			oy = out_y + y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * towerdefence_sprites.width + ix;
			R = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel] >> 5;
			G = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel + 1] >> 5;
			B = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi * 4 , RGB);
		}
	}
}

int getSpriteIndex(int tile_type)
{
    switch (tile_type) {
		case GRASS: return 0;
		case DIRT: return 1;
		case DIRTPREVIOUS: return 2;
		case BUSH: return 3;
		case LAKE1: return 4;
		case LAKE2: return 5;
		case LAKE3: return 6;
		case LAKE4: return 7;
		case LAKE5: return 8;
		case LAKE6: return 9;
		case CREEP: return 10;
		case CREEP1: return 11;
		case CREEP2: return 12;
		case CREEP3: return 13;
		case CREEP4: return 14;
		case HP1: return 15;
		case HP2: return 16;
		case HP3: return 17;
		case HP4: return 18;
		case SPOT: return 19;
		case SELECTEDSPOT: return 20;
		case TOWER: return 21;
		case TOWER2: return 22;
		case SELECTEDTOWER: return 23;
		case SELECTEDTOWER2: return 24;
    }

}
void drawMap()
{
    const int sprite_x[] = {16, 0, 0, 32, 0, 16, 32, 0, 16, 32, 48, 48, 48, 48, 48, 112, 112, 112, 112, 64, 64, 64, 80, 64, 80};
    const int sprite_y[] = {0, 0, 0, 0, 16, 16, 16, 32, 32, 32, 0, 16, 32, 48, 64, 0, 16, 32, 48, 0, 16, 32, 32, 48, 48};

    int row, column;
    for (row = 0; row < SIZEROW; row++)
    {
        for (column = 0; column < SIZECOLUMN; column++)
        {
            if (mapChanges[row][column])
            {
                mapChanges[row][column] = false;
                int index = getSpriteIndex(map[row][column]);
                drawSprite(sprite_x[index], sprite_y[index], column * 16, row * 16, 16, 16);
            }
        }
    }
 }


void printDigit(int digit, int x, int y)
{

	static const int sprite_x[10] = {16, 24, 32, 40, 16, 24, 32, 40, 16, 24};
	static const int sprite_y[10] = {48, 48, 48, 48, 56, 56, 56, 56, 64, 64};

    drawSprite(sprite_x[digit], sprite_y[digit], y, x, 8, 8);
}

void printNumber(int number, int x, int y)
{
    int left_digit = number / 10;
    int right_digit = number % 10;

    printDigit(right_digit, x, y + 8);
    if (left_digit == 0) {
        drawSprite(16, 0, y, x, 8, 8);
    }
    else {
        printDigit(left_digit, x, y);
    }
}


/*max state->coins 99, calls printNum for drawing left or right coin
void printstate->coins(){
	int l,r;
	l=state->coins/10;
	r=state->coins%10;
	printNum(0,8,r);
	if(l!=0){
		printNum(0,0,l);
	}
	else{
		drawSprite(16,0,0,0,8,8);
	}


}

//printing the number of creeps left
void printCreepNumb(){
	int l,r;
	l=creepsRem/10;
	r=creepsRem%10;
	printNum(8,8,r);
	if(l!=0){
		printNum(8,0,l);
	}
	else{
		drawSprite(16,0,0,8,8,8);
	}

}*/
void fillRoute(struct CreepRoute *route, int start_row, int start_column) {
    int column = start_column;
    int row = start_row;
    int i = 0;

    route->length = 0;

    int prethodni_smer = NISTA;
    bool finished = false;
    while (!finished) {
        route->creep_x[i] = row;
        route->creep_y[i] = column;

        if (column == MAP_WIDTH - 1) {
                    finished = true;
                }

        if (column < (MAP_WIDTH - 1) && map[row][column + 1] == DIRT) { // DESNO
            if (i == 0 || prethodni_smer != LEVO) {
                prethodni_smer = DESNO;
                column++;
                i++;
                continue;
            }
        }
        if (column > 0 && map[row][column - 1] == DIRT) { // LEVO
            if (i == 0 || prethodni_smer != DESNO) {
                prethodni_smer = LEVO;
                column--;
                i++;
                continue;
            }
        }
        if (row < MAP_HEIGHT - 1 && map[row + 1][column] == DIRT) { // DOLE
            if (i == 0 || prethodni_smer != GORE) {
                prethodni_smer = DOLE;
                row++;
                i++;
                continue;
            }
        }
        if (row > 0 && map[row - 1][column] == DIRT) { // GORE
            if (i == 0 || prethodni_smer != DOLE) {
                prethodni_smer = GORE;
                row--;
                i++;
                continue;
            }
        }
    }

    route->length = i + 1;
}


//moving creep forward
void moveCreep(struct CreepRoute *route, struct GameState *state, int *currentHP, int *creepsRem){
	int last = route->length - 1; /// r=110 a route->length = 42
	if(map[route->creep_x[last]][route->creep_y[last]] != DIRT){
		if(map[route->creep_x[last]][route->creep_y[last]]==CREEP4){
			(*creepsRem)--;
			map[route->creep_x[last]][route->creep_y[last]] = DIRT;
			mapChanges[route->creep_x[last]][route->creep_y[last]] = true;
		}
		else{
			(*currentHP)--;
			(*creepsRem)--;

			if((*currentHP) == 2){
				map[route->creep_x[last]-1][route->creep_y[last]] = HP2;
				map[route->creep_x[last]+1][route->creep_y[route->length]] = HP2;
				mapChanges[route->creep_x[last]-1][route->creep_y[last]] = true;
				mapChanges[route->creep_x[last]+1][route->creep_y[last]] = true;
			}
			else if((*currentHP) == 1){
				map[route->creep_x[last]-1][route->creep_y[last]] = HP3;
				map[route->creep_x[last]+1][route->creep_y[last]] = HP3;
				mapChanges[route->creep_x[last]-1][route->creep_y[last]] = true;
				mapChanges[route->creep_x[last]+1][route->creep_y[last]] = true;
			}
			else if((*currentHP) == 0){
				map[route->creep_x[last]-1][route->creep_y[last]] = HP4;
				map[route->creep_x[route->length]+1][route->creep_y[last]] = HP4;
				mapChanges[route->creep_x[last]-1][route->creep_y[last]] = true;
				mapChanges[route->creep_x[last]+1][route->creep_y[last]] = true;
				endGame = true;
			}
			map[route->creep_x[last]][route->creep_y[last]] = DIRT;
			mapChanges[route->creep_x[last]][route->creep_y[last]] = true;
		}
	}
	while(--last >= 0){
		if(map[route->creep_x[last]][route->creep_y[last]] != DIRT){
			if(map[route->creep_x[last]][route->creep_y[last]]==CREEP4){
				(*creepsRem)--;
				state->coins+=3;
				printNumber(state->coins, 0, 0);
				printNumber((*creepsRem), 8, 0);
				map[route->creep_x[last]][route->creep_y[last]] = DIRT;
				mapChanges[route->creep_x[last]][route->creep_y[last]] = true;
			}
			else{
				map[route->creep_x[last+1]][route->creep_y[last+1]] = map[route->creep_x[last]][route->creep_y[last]];
				map[route->creep_x[last]][route->creep_y[last]] = DIRT;
				mapChanges[route->creep_x[last+1]][route->creep_y[last+1]] = true;
				mapChanges[route->creep_x[last]][route->creep_y[last]] = true;
			}
		}
	}

	drawMap();
}

void getTowerPos(struct CreepRoute *route, struct TowerPosition *position ){

	int i,j,k,h;
	for(i=1;i<route->length;i++){
		for(j=-1;j<2;j++){
			for(k=-1;k<2;k++){
				if(map[route->creep_x[i]+j][route->creep_y[i]+k] == SPOT || map[route->creep_x[i]+j][route->creep_y[i]+k] == SELECTEDSPOT){
					bool exists = false;
					for(h=0;h<position->numOfTowers;h++){
						if((position->tower_x[h] == route->creep_x[i]+j) && (position->tower_y[h] == route->creep_y[i]+k)){
							exists = true;
						}
					}
					if(!exists){
						position->tower_x[position->numOfTowers] = route->creep_x[i]+j;
						position->tower_y[position->numOfTowers++] = route->creep_y[i]+k;
					}

				}
			}
		}
	}
}

char getPressedKey() {
	char pressedKey;

	if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0) {
		pressedKey = 'r';
	}

	else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0) {
		pressedKey = 'l';
	}
	else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0) {
		pressedKey = 'u';
	}
	else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0) {
		pressedKey = 'R';
	}

	if (pressedKey != lastKey) {
		lastKey = pressedKey;
		return pressedKey;
	}
	else {
		return 'n';
	}
}

void placeTower(struct GameState *state, struct TowerPosition *position){

	char pressedKey = getPressedKey();

	switch(pressedKey){
		case 'r':
			if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SELECTEDSPOT){
				map[position->tower_x[currentI]][position->tower_y[currentI]] = SPOT;
			}
			else{
				if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SELECTEDTOWER){
					map[position->tower_x[currentI]][position->tower_y[currentI]] = TOWER;
				}
				else{
					map[position->tower_x[currentI]][position->tower_y[currentI]] = TOWER2;
				}
			}

			mapChanges[position->tower_x[currentI]][position->tower_y[currentI]] = true;
			currentI++;
			if (currentI == position->numOfTowers) {
				currentI = 0;
			}
			if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SPOT){
				map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map[position->tower_x[currentI]][position->tower_y[currentI]] == TOWER){
					map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER;
				}
				else{
					map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER2;
				}
			}
			mapChanges[position->tower_x[currentI]][position->tower_y[currentI]] = true;
			break;

		case 'l':

			if(map[position->tower_x[currentI]][position->tower_y[currentI]] != TOWER &&
				map[position->tower_x[currentI]][position->tower_y[currentI]] != SELECTEDTOWER &&
				map[position->tower_x[currentI]][position->tower_y[currentI]] != TOWER2 &&
				map[position->tower_x[currentI]][position->tower_y[currentI]] != SELECTEDTOWER2){
				map[position->tower_x[currentI]][position->tower_y[currentI]] = SPOT;
			}
			else{
				if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SELECTEDTOWER){
					map[position->tower_x[currentI]][position->tower_y[currentI]] = TOWER;
				}
				else{
					map[position->tower_x[currentI]][position->tower_y[currentI]] = TOWER2;
				}
			}
			mapChanges[position->tower_x[currentI]][position->tower_y[currentI]] = true;

			currentI--;
			if (currentI < 0) {
				currentI = position->numOfTowers-1;
			}

			if(map[position->tower_x[currentI]][position->tower_y[currentI]] != TOWER &&
					map[position->tower_x[currentI]][position->tower_y[currentI]] != TOWER2){
				map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map[position->tower_x[currentI]][position->tower_y[currentI]] == TOWER){
					map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER;
				}
				else{
					map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER2;
				}
			}
			mapChanges[position->tower_x[currentI]][position->tower_y[currentI]] = true;
			break;

		case 'u':
			btnCnt = (btnCnt + 1)%2;
			if(btnCnt==0){
				map[0][19] = SELECTEDTOWER;
				map[1][19] = TOWER2;
			}
			else{
				map[0][19] = TOWER;
				map[1][19] = SELECTEDTOWER2;
			}
			mapChanges[0][19] = true;
			mapChanges[1][19] = true;
			break;


		case 'R':

			if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SELECTEDSPOT){
				if(btnCnt==0){
					if(state->coins >= 5){
						map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER;
						state->coins -= 5;
						printNumber(state->coins, 0, 0);
					}
				}
				else{
					if(state->coins >= 7){
						map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER2;
						state->coins -= 7;
						printNumber(state->coins, 0, 0);
					}
				}
			}
			else if(map[position->tower_x[currentI]][position->tower_y[currentI]] == SELECTEDTOWER){
				if(state->coins >= 2){
					map[position->tower_x[currentI]][position->tower_y[currentI]] = SELECTEDTOWER2;
					state->coins -= 2;
					printNumber(state->coins, 0, 0);
				}
			}
			mapChanges[position->tower_x[currentI]][position->tower_y[currentI]] = true;
			break;


	}

	drawMap();

}

void insertCreep(struct CreepRoute *route, int *creepsSpawned)
{
	map[route->creep_x[0]][route->creep_y[0]] = CREEP;
	mapChanges[route->creep_x[0]][route->creep_y[0]] = true;
	drawMap();
	(*creepsSpawned)++;
}

void turretOneFire(){
	int row,column;
	for (row = 0; row < SIZEROW-1; row++) {
		for (column = 0; column < SIZECOLUMN-1; column++) {
			if((map[row][column] == TOWER )  || map[row][column] == SELECTEDTOWER){
				int i,j;
				for(i=-1;i<2;i++){
					for(j=-1;j<2;j++){
						if(map[row+i][column+j] == CREEP || map[row+i][column+j] == CREEP1 || map[row+i][column+j] == CREEP2 || map[row+i][column+j] == CREEP3){
							drawSprite(64,64,column*16,row*16,16,16);
							mapChanges[row][column] = true;
							map[row+i][column+j]++;
							mapChanges[row+i][column+j]=true;
						}
					}
				}
			}
		}
	}
	for(row = 0 ; row < 500000;row++);

	drawMap();
}

void turretTwoFire(){
	int row,column;
	for (row = 0; row < SIZEROW-1; row++) {
		for (column = 0; column < SIZECOLUMN-1; column++) {
			if((map[row][column] == TOWER2 )  || map[row][column] == SELECTEDTOWER2){
				int i,j;
				for(i=-1;i<2;i++){
					for(j=-1;j<2;j++){
						if(map[row+i][column+j] == CREEP || map[row+i][column+j] == CREEP1 || map[row+i][column+j] == CREEP2 || map[row+i][column+j] == CREEP3){
							drawSprite(80,64,column*16,row*16,16,16);
							mapChanges[row][column] = true;
							map[row+i][column+j]++;
							mapChanges[row+i][column+j]=true;
						}

					}
				}
			}
		}
	}
	for(row = 0 ; row < 500000;row++);

	drawMap();
}

void drawWinLvl(){
	int row,column;
	while(1){

		if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0){
			break;
		}

		for (row = 0; row < SIZEROW; row++) {
			for (column = 0; column < SIZECOLUMN; column++) {
				map[row][column] = nextLvl[row][column];
				mapChanges[row][column] = true;

			}
		}
		drawMap();
	}
}

void drawWon(){
	int row,column;
	while(1){

		if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0){
			break;
		}

		for (row = 0; row < SIZEROW; row++) {
			for (column = 0; column < SIZECOLUMN; column++) {
				map[row][column] = youWon[row][column];
				mapChanges[row][column] = true;

			}
		}
		drawMap();
	}
}

void drawEndGame(){
	int row,column;
	while(1){

		if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0){
			break;
		}

		for (row = 0; row < SIZEROW; row++) {
			for (column = 0; column < SIZECOLUMN; column++) {
				if (gameOver[row][column] == DIRT) {
					drawSprite(0, 0, column * 16, row * 16, 16, 16);
				}
				else if (gameOver[row][column] == CREEP){
					drawSprite(48, 0, column * 16, row * 16, 16, 16);
					gameOver[row][column] = CREEP4;
				}
				else if (gameOver[row][column] == CREEP4){
					drawSprite(48, 64, column * 16, row * 16, 16, 16);
					gameOver[row][column] = CREEP;
				}
			}
		}
		for (row=0;row < 25;row++);
	}
}

bool play_level(struct Level *level, struct GameState *game_state, struct CreepRoute *route, struct TowerPosition *position)
{
    int row, column;
    for (row = 0; row < SIZEROW; row++)
	{
        for (column = 0; column < SIZECOLUMN; column++)
		{
            map[row][column] = level->initial_map[row][column];
            mapChanges[row][column] = true;
        }
    }

    int currentHP = 3;
    int creepSpeed = 0;
    int creepTime = 0;
    int turrentOneFire=0, turrentTwoFire=0;
    int creepsRem = MAXCREEPS;
    int creepsSpawned = 0;
    position->numOfTowers = 0;
    game_state->coins = 20;
    endGame = false;
    lastKey = 'n';
    currentI = 0;
    btnCnt = 0;

    unsigned int placeTowerSpeed = 0;

    drawMap();

    printNumber(game_state->coins, 0, 0);
    printNumber(creepsRem, 8, 0);
    drawSprite(8, 64, 16, 0, 8, 8);
    drawSprite(8, 72, 16, 8, 8, 8);

    fillRoute(route, level->start_row, level->start_column);

    getTowerPos(route, position);
    bool passed;
	while(1){

			if(endGame)
			{
				passed = false;
				break;
			}
			if (creepsRem == 0)
			{
				passed = true;
				break;
			}
			if (placeTowerSpeed == 10000)
			{
				placeTower(game_state, position);
				placeTowerSpeed = 0;
			}

			if (creepSpeed == 1000000)
			{
				moveCreep(route, game_state, &currentHP, &creepsRem);
				printNumber(creepsRem, 8, 0);
				printNumber(game_state->coins, 0, 0);

				if(creepTime == 5)
				{
					if(creepsSpawned < MAXCREEPS)
					{
						insertCreep(route, &creepsSpawned);
					}
				creepTime = 0;
				}

				if(turrentOneFire == 6 )
				{
					turretOneFire();
					turrentOneFire = 0;
				}

				if(turrentTwoFire == 4 )
				{
					turretTwoFire();
					turrentTwoFire = 0;
				}
				turrentOneFire++;
				turrentTwoFire++;
				creepTime++;
				creepSpeed=0;
			}
			creepSpeed++;
			placeTowerSpeed++;
		}
    return passed;
}

int main()
{

    cleanup_platform();

    init_platform();
    init();

    struct GameState state;
    struct CreepRoute route;
    struct Level levels[] = {level1, level2};
    struct TowerPosition position;
    state.coins = 20;
    state.current_level = 0;
    int lastLevel = sizeof(levels)/sizeof(struct Level);

    while (1)
	{


        bool won = play_level(&levels[state.current_level], &state, &route, &position);


        if (won) {
            if (state.current_level + 1 < lastLevel) {
                state.current_level++;
                drawWinLvl();
            }
            else {
                drawEndGame();
				state.current_level = 0;
				state.coins = 20;
            }
        }
        else {
            drawEndGame();
			state.current_level = 0;
			state.coins = 20;

        }
    }

    return 0;
}



