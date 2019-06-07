/*
 * maps.h
 *
 *  Created on: 08.05.2018.
 *      Author: student
 */
#include<stdbool.h>
#ifndef MAPS_H_
#define MAPS_H_
#define SIZEROW 15
#define SIZECOLUMN 20
struct Level {
    unsigned char initial_map[SIZEROW][SIZECOLUMN];
    int start_row;
    int start_column;
};

// Map 1
struct Level level1 = {
	.initial_map =
	{
	{'G','G','D','G','G','G','G','G','G','G','G','G','B','B','G','G','G','G','G','O'},
	{'G','G','D','G','G','G','G','G','G','G','G','G','B','B','1','2','3','G','G','t'},
	{'G','G','D','G','G','G','D','D','D','G','G','G','G','G','4','5','6','G','G','G'},
	{'G','G','D','G','G','G','D','S','D','G','G','G','G','G','G','G','G','G','G','G'},
	{'G','G','D','G','G','G','D','G','D','G','G','G','G','G','G','G','G','G','G','G'},
	{'G','G','D','X','G','S','D','G','D','G','G','G','G','G','G','G','G','S','G','7'},
	{'G','G','D','D','D','D','D','G','D','G','G','G','G','D','D','D','D','D','D','D'},
	{'G','G','G','G','G','G','G','G','D','G','G','G','G','D','S','G','G','G','G','7'},
	{'G','G','G','G','G','G','G','G','D','G','G','G','G','D','G','G','G','G','G','G'},
	{'G','B','G','G','G','G','G','G','D','G','G','G','G','D','G','G','B','G','G','G'},
	{'B','G','B','B','G','G','G','G','D','S','G','G','S','D','G','G','G','G','B','G'},
	{'B','G','G','B','B','G','B','G','D','D','D','D','D','D','G','G','G','G','G','B'},
	{'G','B','B','G','G','G','G','G','G','G','G','G','G','G','G','G','G','B','B','B'},
	{'G','G','B','B','B','G','G','G','G','G','G','G','G','G','B','G','B','B','G','B'},
	{'B','B','G','B','G','G','G','B','B','B','G','B','G','G','G','B','G','B','B','G'},
},

.start_row = 0,
.start_column = 2};

struct Level level2 = {
	.initial_map =
	{
	{'G','B','B','B','B','B','B','G','B','G','G','G','G','B','B','G','G','G','G','O'},
	{'G','G','G','B','B','B','B','B','G','B','G','G','B','B','G','G','G','B','G','t'},
	{'B','G','G','B','B','B','G','G','G','G','B','B','B','G','G','B','B','G','S','7'},
	{'B','B','B','B','G','B','G','G','B','B','B','B','B','D','D','D','D','D','D','D'},
	{'G','G','B','G','G','B','G','G','B','B','B','B','B','D','S','B','G','B','B','7'},
	{'G','B','B','B','B','G','B','B','S','B','B','B','S','D','G','G','G','B','B','B'},
	{'B','B','B','B','D','D','D','D','D','D','D','D','D','D','B','B','B','B','B','B'},
	{'B','G','G','G','D','S','G','G','G','G','G','B','B','G','B','B','G','G','B','B'},
	{'B','B','B','B','D','B','G','B','B','B','B','B','B','G','G','G','B','B','B','B'},
	{'B','B','B','B','D','D','D','B','G','G','B','B','B','B','B','B','B','B','G','B'},
	{'B','B','X','B','B','S','D','B','G','B','B','B','B','B','B','B','G','B','B','B'},
	{'D','D','D','D','D','D','D','B','B','B','B','G','B','B','B','G','G','B','B','B'},
	{'G','G','B','B','S','B','G','B','B','B','G','B','B','B','B','B','G','G','G','G'},
	{'G','G','G','B','B','B','B','B','G','B','B','G','B','B','G','B','G','1','2','3'},
	{'G','G','G','B','B','B','B','B','B','B','B','B','B','G','B','B','B','4','5','6'},
},

.start_row = 11,
.start_column = 0};


// game Over
unsigned char gameOver[SIZEROW][SIZECOLUMN] = {
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
	{'D',0,0,0,0,'D',0,0,0,'D',0,'D','D','D',0,'D',0,0,0,'D'},
	{'D',0,'D','D',0,'D',0,'D',0,'D',0,0,'D',0,0,'D',0,'D','D','D'},
	{'D',0,'D','D','D','D',0,'D',0,'D',0,'D',0,'D',0,'D',0,'D','D','D'},
	{'D',0,'D',0,0,'D',0,0,0,'D',0,'D',0,'D',0,'D',0,0,'D','D'},
	{'D',0,'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,'D','D','D'},
	{'D',0,0,0,0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,0,0,'D'},
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
	{'D','D',0,0,0,0,'D',0,'D',0,'D',0,0,0,'D',0,0,0,'D','D'},
	{'D','D',0,'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,'D','D'},
	{'D','D',0,'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,'D','D'},
	{'D','D',0,'D','D',0,'D',0,'D',0,'D',0,0,'D','D',0,0,0,'D','D'},
	{'D','D',0,'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,0,'D','D','D'},
	{'D','D',0,0,0,0,'D','D',0,'D','D',0,0,0,'D',0,'D',0,'D','D'},
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
};

// lvl complete
unsigned char nextLvl[SIZEROW][SIZECOLUMN] = {
	{'G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G'},
	{'G','G',0,'G','G',0,'G',0,0,0,'G',0,'G',0,'G',0,0,0,'G','G'},
	{'G','G',0,0,'G',0,'G',0,'G','G','G','G',0,'G','G','G',0,'G','G','G'},
	{'G','G',0,'G',0,0,'G',0,0,0,'G','G',0,'G','G','G',0,'G','G','G'},
	{'G','G',0,'G','G',0,'G',0,'G','G','G','G',0,'G','G','G',0,'G','G','G'},
	{'G','G',0,'G','G',0,'G',0,0,0,'G',0,'G',0,'G','G',0,'G','G','G'},
	{'G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G'},
	{'G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G'},
	{'G','G','G','G',0,'G','G','G',0,'G',0,'G','G',0,'G','G','G','G','G','G'},
	{'G','G','G','G',0,'G','G','G',0,'G',0,'G','G',0,'G','G','G','G','G','G'},
	{'G','G','G','G',0,'G','G','G',0,'G',0,'G','G',0,'G','G','G','G','G','G'},
	{'G','G','G','G',0,'G','G','G',0,'G',0,'G','G',0,'G','G','G','G','G','G'},
	{'G','G','G','G',0,'G','G','G',0,'G',0,'G','G',0,'G','G','G','G','G','G'},
	{'G','G','G','G',0,0,0,'G','G',0,'G','G','G',0,0,0,'G','G','G','G'},
	{'G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G','G'},
};

// You won
unsigned char youWon[SIZEROW][SIZECOLUMN]= {
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
	{'D','D',0,'D','D','D',0,'D',0,0,0,0,0,'D',0,'D','D',0,'D','D'},
	{'D','D','D',0,'D',0,'D','D',0,'D','D','D',0,'D',0,'D','D',0,'D','D'},
	{'D','D','D','D',0,'D','D','D',0,'D','D','D',0,'D',0,'D','D',0,'D','D'},
	{'D','D','D','D',0,'D','D','D',0,'D','D','D',0,'D',0,'D','D',0,'D','D'},
	{'D','D','D','D',0,'D','D','D',0,'D','D','D',0,'D',0,'D','D',0,'D','D'},
	{'D','D','D','D',0,'D','D','D',0,0,0,0,0,'D',0,0,0,0,'D','D'},
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
	{'D','D',0,'D','D','D',0,'D',0,0,0,0,0,'D',0,'D','D',0,'D','D'},
	{'D','D',0,'D','D','D',0,'D',0,'D','D','D',0,'D',0,0,'D',0,'D','D'},
	{'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,0,'D',0,'D','D'},
	{'D','D',0,'D',0,'D',0,'D',0,'D','D','D',0,'D',0,'D',0,0,'D','D'},
	{'D','D',0,0,'D',0,0,'D',0,'D','D','D',0,'D',0,'D',0,0,'D','D'},
	{'D','D',0,'D','D','D',0,'D',0,0,0,0,0,'D',0,'D','D',0,'D','D'},
	{'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D'},
};

bool mapChanges[SIZEROW][SIZECOLUMN]={
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
	{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true},
};

#endif /* MAPS_H_ */
