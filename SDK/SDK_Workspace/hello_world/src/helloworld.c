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
#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "xil_exception.h"
#include "vga_periph_mem.h"
#include "towerdefence_sprites.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include "maps.h"

#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010

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
#define MAXCREEPS 5

bool endGame = false;
//Places for towers

unsigned char map1[SIZEROW][SIZECOLUMN];

int rowDirtFields[60]={[0 ... 59] = -1};
int columnDirtFields[60]={[0 ... 59] = -1};
int dirtLen=0;

int rowTowerFields[10]={[0 ... 9] = -1};
int columnTowerFields[10]={[0 ... 9] = -1};
int numOfTowers = 0;

int creepsSpawned = 0;
int creepsRem = 30;
int currentI = 0;
int btnCnt = 0;
int currentHP = 3;
int coins = 31;
unsigned int placetowerscnt = 0;
unsigned int creepspeedcnt = 0;

//Pokusaj dodavanja novog nivos
int numOfCreepsAlive = MAXCREEPS;
int lvlCnt = 0;


void init(){
	int row,column;
	for (row = 0; row < SIZEROW; row++) {
		for (column = 0; column < SIZECOLUMN; column++) {
			map1[row][column] = map1Origin[row][column];
			mapChanges[row][column] = true;

		}
	}

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
				if (map1[row][column] == GRASS){
					drawSprite(16, 0, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == DIRT || map1[row][column] == DIRTPREVIOUS) {
					drawSprite(0, 0, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == BUSH){
					drawSprite(32, 0, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == CREEP){
					drawSprite(48, 0, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == CREEP1){
					drawSprite(48, 16, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == CREEP2){
					drawSprite(48, 32, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == CREEP3){
					drawSprite(48, 48, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == CREEP4){
					drawSprite(48, 64, column * 16, row * 16, 16, 16);

				}
				else if (map1[row][column] == LAKE1){
					drawSprite(0, 16, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == LAKE2){
					drawSprite(16, 16, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == LAKE3){
					drawSprite(32, 16, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == LAKE4){
					drawSprite(0, 32, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == LAKE5){
					drawSprite(16, 32, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == LAKE6){
					drawSprite(32, 32, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == SPOT){
					drawSprite(64, 0, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == SELECTEDSPOT){
					drawSprite(64, 16, column * 16, row * 16, 16, 16);
				}
				else if (map1[row][column] == TOWER){
					drawSprite(64,32,column*16,row*16,16,16);
				}
				else if (map1[row][column] == TOWER2){
					drawSprite(80,32,column*16,row*16,16,16);
				}
				else if(map1[row][column] == SELECTEDTOWER){
					drawSprite(64,48,column*16,row*16,16,16);
				}
				else if(map1[row][column] == SELECTEDTOWER2){
					drawSprite(80,48,column*16,row*16,16,16);
				}
				else if(map1[row][column] == HP1){
					drawSprite(112,0,column*16,row*16,16,16);
				}
				else if(map1[row][column] == HP2){
					drawSprite(112,16,column*16,row*16,16,16);
				}
				else if(map1[row][column] == HP3){
					drawSprite(112,32,column*16,row*16,16,16);
				}
				else if(map1[row][column] == HP4){
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
	if(map1[rowDirtFields[i]][columnDirtFields[i]] != DIRT){
		if(map1[rowDirtFields[i]][columnDirtFields[i]]==CREEP4){
			numOfCreepsAlive--;
			creepsRem--;
			map1[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
			mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
		}
		else{
			currentHP--;
			creepsRem--;

			if(currentHP == 2){
				map1[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP2;
				map1[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP2;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
			}
			else if(currentHP == 1){
				map1[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP3;
				map1[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP3;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
			}
			else if(currentHP == 0){
				map1[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = HP4;
				map1[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = HP4;
				mapChanges[rowDirtFields[dirtLen]-1][columnDirtFields[dirtLen]] = true;
				mapChanges[rowDirtFields[dirtLen]+1][columnDirtFields[dirtLen]] = true;
				endGame = true;
			}
			map1[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
			mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
		}
	}
	while((i--) >= 0){
		if(map1[rowDirtFields[i]][columnDirtFields[i]] != DIRT){
			if(map1[rowDirtFields[i]][columnDirtFields[i]]==CREEP4){
				numOfCreepsAlive--;
				creepsRem--;
				coins+=3;
				printCoins();
				map1[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
				mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
			}
			else{
				map1[rowDirtFields[i+1]][columnDirtFields[i+1]] = map1[rowDirtFields[i]][columnDirtFields[i]];
				map1[rowDirtFields[i]][columnDirtFields[i]] = DIRT;
				mapChanges[rowDirtFields[i+1]][columnDirtFields[i+1]] = true;
				mapChanges[rowDirtFields[i]][columnDirtFields[i]] = true;
			}
		}
	}


}

void getDirtPos(int startRow,int startColumn){
	int prevRow,prevColumn;
	rowDirtFields[dirtLen]=startRow;
	columnDirtFields[dirtLen++]=startColumn;

	if(startRow == 0){
		if(map1[startRow+1][startColumn] == DIRT){
			rowDirtFields[dirtLen]=startRow+1;
			columnDirtFields[dirtLen]=startColumn;
			startRow++;
		}
	}
	if(startColumn == 0){
		if(map1[startRow][startColumn+1] == DIRT){
			rowDirtFields[dirtLen]=startRow;
			columnDirtFields[dirtLen]=startColumn+1;
			startColumn++;
		}
	}


	while(rowDirtFields[dirtLen] != 0 && rowDirtFields[dirtLen] != 14 && columnDirtFields[dirtLen] != 19){
		prevRow = rowDirtFields[dirtLen-1];
		prevColumn = columnDirtFields[dirtLen-1];
		dirtLen++;
		if(map1[rowDirtFields[dirtLen-1]+1][columnDirtFields[dirtLen-1]] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1]+1)||(prevColumn!=columnDirtFields[dirtLen-1])){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1]+1;
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1];
			}

		}
		if(map1[rowDirtFields[dirtLen-1]-1][columnDirtFields[dirtLen-1]] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1]-1)||(prevColumn!=columnDirtFields[dirtLen-1])){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1]-1;
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1];

			}
		}
		if(map1[rowDirtFields[dirtLen-1]][columnDirtFields[dirtLen-1]+1] == DIRT){
			if((prevRow!=rowDirtFields[dirtLen-1])||(prevColumn!=columnDirtFields[dirtLen-1]+1)){
				rowDirtFields[dirtLen] = rowDirtFields[dirtLen-1];
				columnDirtFields[dirtLen] = columnDirtFields[dirtLen-1]+1;

			}
		}
		if(map1[rowDirtFields[dirtLen-1]][columnDirtFields[dirtLen-1]-1] == DIRT){
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
				if(map1[rowDirtFields[i]+j][columnDirtFields[i]+k] == SPOT || map1[rowDirtFields[i]+j][columnDirtFields[i]+k] == SELECTEDSPOT){
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


void placeTower(){

	if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0){

			if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDSPOT){
				map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SPOT;
			}
			else{
				if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER;
				}
				else{
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER2;
				}
			}

			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
			currentI++;
			currentI %= numOfTowers;
			if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SPOT){
				map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == TOWER){
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
				}
				else{
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
				}
			}
		}

	if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0){

			if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER &&
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != SELECTEDTOWER &&
				map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER2 &&
									map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != SELECTEDTOWER2){
				map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SPOT;
			}
			else{
				if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER;
				}
				else{
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = TOWER2;
				}
			}
			mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;

			currentI=(6+currentI)%numOfTowers;

			if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER &&
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] != TOWER2){
				map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDSPOT;
			}
			else{
				if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == TOWER){
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
				}
				else{
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
				}
			}
		}

	if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0){
		btnCnt = (btnCnt + 1)%2;
		if(btnCnt==0){
			map1[0][19] = SELECTEDTOWER;
			map1[1][19] = TOWER2;
		}
		else{
			map1[0][19] = TOWER;
			map1[1][19] = SELECTEDTOWER2;
		}
		mapChanges[0][19] = true;
		mapChanges[1][19] = true;
	}


	if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0){

			if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDSPOT){
				if(btnCnt==0){
					if(coins >= 5){
						map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER;
						coins -= 5;
						printCoins();
					}
				}
				else{
					if(coins >= 7){
						map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
						coins -= 7;
						printCoins();
					}
				}
			}
			else if(map1[rowTowerFields[currentI]][columnTowerFields[currentI]] == SELECTEDTOWER){
				if(coins >= 2){
					map1[rowTowerFields[currentI]][columnTowerFields[currentI]] = SELECTEDTOWER2;
					coins -= 2;
					printCoins();
				}
			}

		}

	mapChanges[rowTowerFields[currentI]][columnTowerFields[currentI]] = true;
	drawMap();
}

void insertCreep(){
	//////////////////////////////////////////////////////////////////////////////////////
	static int numOfCreepsSpawed = 0; /// NEMOJ DIRATI OVO NIKADA NI SLUCAJNO
	numOfCreepsSpawed++;			  /// A NI OVO!!!!!!!!!!!!!!!!!! NIKADA :) OZBILJNO TO MISLIM.... KENJICU NE SERI!
	//////////////////////////////////////////////////////////////////////////////////////

	map1[rowDirtFields[0]][columnDirtFields[0]] = CREEP;
	mapChanges[rowDirtFields[0]][columnDirtFields[0]] = true;
	drawMap();
	creepsSpawned++;
}

void turretOneFire(){
	int row,column;
	for (row = 0; row < SIZEROW-1; row++) {
		for (column = 0; column < SIZECOLUMN-1; column++) {
			if((map1[row][column] == TOWER )  || map1[row][column] == SELECTEDTOWER){
				int i,j;
				for(i=-1;i<2;i++){
					for(j=-1;j<2;j++){
						if(map1[row+i][column+j] == CREEP || map1[row+i][column+j] == CREEP1 || map1[row+i][column+j] == CREEP2 || map1[row+i][column+j] == CREEP3){
							drawSprite(64,64,column*16,row*16,16,16);
							mapChanges[row][column] = true;
							map1[row+i][column+j]++;
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
				if((map1[row][column] == TOWER2 )  || map1[row][column] == SELECTEDTOWER2){
					int i,j;
					for(i=-1;i<2;i++){
						for(j=-1;j<2;j++){
							if(map1[row+i][column+j] == CREEP || map1[row+i][column+j] == CREEP1 || map1[row+i][column+j] == CREEP2 || map1[row+i][column+j] == CREEP3){
								drawSprite(80,64,column*16,row*16,16,16);
								mapChanges[row][column] = true;
								map1[row+i][column+j]++;
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

void drawEndGame(){
	int row,column;
	while(1){

		if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER) == 0){
			for (row = 0; row < SIZEROW; row++) {
				for (column = 0; column < SIZECOLUMN; column++) {
					map1[row][column] = map1Origin[row][column];
					mapChanges[row][column] = true;

				}
			}

			currentHP = 3;
			coins = 21;

			numOfCreepsAlive = MAXCREEPS;
			currentI = 0;
			endGame = false;
			drawMap(); // init map
			printCoins();
			drawSprite(8,64,16,0,8,8);

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

	int cnt=0,cnt1=0;
	init_platform();
	srand(30);
	init();

	int creepCnt = 0;

	drawMap(); // init map

	printCoins();

	printCreepNumb();

	drawSprite(8,64,16,0,8,8);

	drawSprite(8,72,16,8,8,8);

	getDirtPos(11,0);
	getTowerPos();



	while(1){

		if(endGame){
			//drawEndGame();
		}

		if (placetowerscnt == 200000){
			placetowerscnt = 0;
			placeTower();
		}


		//provera da li je kraj lvl-a



		if (creepspeedcnt == 1000000){
			moveCreep();
			printCreepNumb();



			if(creepCnt == 5){
				if(creepsSpawned < 30){
					insertCreep();
				}

				creepCnt = 0;

			}

			if(cnt == 6 ){
				turretOneFire();
				cnt = 0;
			}
			if(cnt1 == 4 ){
				turretTwoFire();
				cnt1 = 0;
			}
			cnt++;
			cnt1++;
			creepCnt++;
			creepspeedcnt=0;
		}


		placetowerscnt++;
		creepspeedcnt++;

	}

	//

	return 0;
}
