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

#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010
#define MAXCREEPS 20

bool endGame = false;

//////////////BUGS////////////////////
// level 2 tower cnt bug,
// something is wrong with array initialization or define(working after first pass)
//////////////////////////////////////

int rowDirtFields[60]={[0 ... 59] = -1};
int columnDirtFields[60]={[0 ... 59] = -1};
int dirtLen=0;

int rowTowerFields[10]={[0 ... 9] = -1};
int columnTowerFields[10]={[0 ... 9] = -1};
int numOfTowers = 0;



char lastKey = 'n';

typedef struct dirt_position{

	unsigned char start_position[SIZEROW][SIZECOLUMN];
	unsigned char map[SIZEROW][SIZECOLUMN];

}Drit_position;

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

void drawMap(){
	int row, column;
	for (row = 0; row < SIZEROW; row++) {
		for (column = 0; column < SIZECOLUMN; column++) {
			if(mapChanges[row][column]){
				mapChanges[row][column] = false;
				if (map[row][column] == GRASS){
					drawSprite(16, 0, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == DIRT || map[row][column] == DIRTPREVIOUS) {
					drawSprite(0, 0, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == BUSH){
					drawSprite(32, 0, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == CREEP){
					drawSprite(48, 0, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == CREEP1){
					drawSprite(48, 16, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == CREEP2){
					drawSprite(48, 32, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == CREEP3){
					drawSprite(48, 48, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == CREEP4){
					drawSprite(48, 64, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE1){
					drawSprite(0, 16, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE2){
					drawSprite(16, 16, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE3){
					drawSprite(32, 16, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE4){
					drawSprite(0, 32, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE5){
					drawSprite(16, 32, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == LAKE6){
					drawSprite(32, 32, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == SPOT){
					drawSprite(64, 0, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == SELECTEDSPOT){
					drawSprite(64, 16, column * 16, row * 16, 16, 16);
				}
				else if (map[row][column] == TOWER){
					drawSprite(64,32,column*16,row*16,16,16);
				}
				else if (map[row][column] == TOWER2){
					drawSprite(80,32,column*16,row*16,16,16);
				}
				else if(map[row][column] == SELECTEDTOWER){
					drawSprite(64,48,column*16,row*16,16,16);
				}
				else if(map[row][column] == SELECTEDTOWER2){
					drawSprite(80,48,column*16,row*16,16,16);
				}
				else if(map[row][column] == HP1){
					drawSprite(112,0,column*16,row*16,16,16);
				}
				else if(map[row][column] == HP2){
					drawSprite(112,16,column*16,row*16,16,16);
				}
				else if(map[row][column] == HP3){
					drawSprite(112,32,column*16,row*16,16,16);
				}
				else if(map[row][column] == HP4){
					drawSprite(112,48,column*16,row*16,16,16);
				}


			}
		}
	}
}

//draw amount of coins
void printNum(int row,int column,int num){

	if(num==0){
		drawSprite(16,48,column,row,8,8);
	}
	else if(num==1){
		drawSprite(24,48,column,row,8,8);
	}

	else if(num==2){
		drawSprite(32,48,column,row,8,8);
	}
	else if(num==3){
		drawSprite(40,48,column,row,8,8);
	}
	else if(num==4){
		drawSprite(16,56,column,row,8,8);
	}
	else if(num==5){
		drawSprite(24,56,column,row,8,8);
	}
	else if(num==6){
		drawSprite(32,56,column,row,8,8);
	}
	else if(num==7){
		drawSprite(40,56,column,row,8,8);
	}
	else if(num==8){
		drawSprite(16,64,column,row,8,8);
	}
	if(num==9){
		drawSprite(24,64,column,row,8,8);
	}


}

//max coins 99, calls printNum for drawing left or right coin
void printCoins(){
	int l,r;
	l=coins/10;
	r=coins%10;
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

}

//moving creep forward
void moveCreep(){
	int i = dirtLen;
	if(map[rowDirtFields[i]][columnDirtFields[i]] != DIRT){
		if(map[rowDirtFields[i]][columnDirtFields[i]]==CREEP4){
			creepsRem--;
			map[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
			mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
		}
		else{
			currentHP--;
			creepsRem--;

			if(currentHP == 2){
				map[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP2;
				map[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP2;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
			}
			else if(currentHP == 1){
				map[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP3;
				map[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP3;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
			}
			else if(currentHP == 0){
				map[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP4;
				map[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP4;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
				endGame = true;
			}
			map[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
			mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
		}
	}
	while((i--) >= 0){
		if(map[rowDirtFields[i]][columnDirtFields[i]] != DIRT){
			if(map[rowDirtFields[i]][columnDirtFields[i]]==CREEP4){
				creepsRem--;
				coins+=3;
				printCoins();
				map[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
				mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
			}
			else{
				map[rowDirtFields[i+1]][columnDirtFields[i+1]] = map[rowDirtFields[i]][columnDirtFields[i]];
				map[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
				mapChanges[rowDirtFields[i+1]][columnDirtFields[i+1]] = true;
				mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
			}
		}
	}

	drawMap();
}

void getDirtPos(){



	while(rowDirtFields[dirtLen] != 0 && rowDirtFields[dirtLen] != 14 && columnDirtFields[dirtLen] != 19)
	{
		prevRow = rowDirtFields[dirtLen-1];
		prevColumn = columnDirtFields[dirtLen-1];
		dirtLen++;
		if(map[rowDirtFields[dirtLen-1]+1][columnDirtFields[dirtLen-1]] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1]+1)||(prevColumn!=columnDirtFields[dirtLen-1])){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1]+1;
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1];
			}

		}
		if(map[rowDirtFields[dirtLen-1]-1][columnDirtFields[dirtLen-1]] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1]-1)||(prevColumn!=columnDirtFields[dirtLen-1])){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1]-1;
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1];

			}
		}
		if(map[rowDirtFields[dirtLen-1]][columnDirtFields[dirtLen-1]+1] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1])||(prevColumn!=columnDirtFields[dirtLen-1]+1)){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1];
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1]+1;

			}
		}
		if(map[rowDirtFields[dirtLen-1]][columnDirtFields[dirtLen-1]-1] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1])||(prevColumn!=columnDirtFields[dirtLen-1]-1)){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1];
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1]-1;
			}
		}
	}
}

void getTowerPos(){

	int i,j,k,h;
	for(i=1;i<dirtLen-1;i++){
		for(j=-1;j<2;j++){
			for(k=-1;k<2;k++){
				if(map[rowDirtFields[i]+j][columnDirtFields[i]+k] == SPOT || map[rowDirtFields[i]+j][columnDirtFields[i]+k] == SELECTEDSPOT){
					bool exists = false;
					for(h=0;h<i;h++){
						if((rowTowerFields[h] == rowDirtFields[i]+j) && (columnTowerFields[h] == columnDirtFields[i]+k)){
							exists = true;
						}
					}
					if(!exists){
						rowTowerFields[numOfTowers] = rowDirtFields[i]+j;
						columnTowerFields[numOfTowers++] = columnDirtFields[i]+k;
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

void placeTower(){

	char pressedKey = getPressedKey();

	switch(pressedKey){
		case 'r':
			if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDSPOT){
				map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SPOT;
			}
			else{
				if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER;
				}
				else{
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER2;
				}
			}

			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
			currentI++;
			if (currentI == numOfTowers) {
				currentI = 0;
			}
			if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SPOT){
				map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == TOWER){
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
				}
				else{
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
				}
			}
			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
			break;

		case 'l':

			if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER &&
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] != SELECTEDTOWER &&
				map[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER2 &&
									map[rowTowerFields[currentI]][columnTowerFields[currentI]] != SELECTEDTOWER2){
				map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SPOT;
			}
			else{
				if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER;
				}
				else{
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER2;
				}
			}
			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;

			currentI--;
			if (currentI < 0) {
				currentI = numOfTowers-1;
			}

			if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER &&
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER2){
				map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == TOWER){
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
				}
				else{
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
				}
			}
			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
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

			if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDSPOT){
				if(btnCnt==0){
					if(coins >= 5){
						map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
						coins -= 5;
						printCoins();
					}
				}
				else{
					if(coins >= 7){
						map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
						coins -= 7;
						printCoins();
					}
				}
			}
			else if(map[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
				if(coins >= 2){
					map[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
					coins -= 2;
					printCoins();
				}
			}
			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
			break;


	}
	//mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
	drawMap();

}

void insertCreep(){
	//////////////////////////////////////////////////////////////////////////////////////
	//static int DONTTOUCH = 0; /// NEMOJ DIRATI OVO NIKADA NI SLUCAJNO
	//DONTTOUCH++;			  /// A NI OVO!!!!!!!!!!!!!!!!!! NIKADA :) OZBILJNO TO MISLIM.... KENJICU NE SERI!
	//////////////////////////////////////////////////////////////////////////////////////

	map[rowDirtFields[0]][columnDirtFields[0]] = CREEP;
	mapChanges[rowDirtFields[0]][columnDirtFields[0]] = true;
	drawMap();
	creepsSpawned++;
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


int main() {

	cleanup_platform();

	init_platform();
	init();

	while(1)
	{
		int row,column;
		for (row = 0; row < SIZEROW; row++) {
			for (column = 0; column < SIZECOLUMN; column++) {
				map[row][column] = map1Origin[row][column];
				mapChanges[row][column] = true;
			}
		}

		int turrentOneFire=0, turrentTwoFire=0;
		unsigned int placeTowerSpeed=0;
		unsigned int creepSpeed = 0;
		int creepTime = 0;

		int creepsSpawned = 0;
		int creepsRem = MAXCREEPS;
		int currentI = 0;
		int btnCnt = 0;
		int currentHP = 3;
		int coins = 20;
		// reset global variables

		endGame = false;
		creepsRem = MAXCREEPS;
		dirtLen=0;
		numOfTowers = 0;
		bool passed = false;

		drawMap(); // init map

		printCoins();
		printCreepNumb();
		drawSprite(8,64,16,0,8,8);
		drawSprite(8,72,16,8,8,8);
		getDirtFilds();
		getTowerPos();

		while(1){

					if(endGame){
						passed = false;
						break;
					}

					if (creepsRem == 0){
						passed = true;
						break;
					}
					if (placeTowerSpeed == 10000){
						placeTower();
						placeTowerSpeed = 0;
					}

					if (creepSpeed == 1000000){
						if(creepTime == 5){
							if(creepsSpawned < MAXCREEPS){
								insertCreep();
							}

							creepTime = 0;

						}

						moveCreep();
						printCreepNumb();

						if(turrentOneFire == 6 ){
							turretOneFire();
							turrentOneFire = 0;
						}

						if(turrentTwoFire == 4 ){
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

	return 0;
}
