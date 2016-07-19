#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku.h"

#define NUM_DIGITS 9
#define NUM_ROWS   NUM_DIGITS
#define NUM_COLS   NUM_DIGITS
#define NUM_PEERS  20
#define NUM_UNITS  3
#define DIGITS     "123456789"
#define ROW_NAMES  "ABCDEFGHI"
#define COL_NAMES  DIGITS

typedef struct square {
  char vals[NUM_DIGITS+1]; // string of possible values
  int assigned;
  unsigned char row;
  unsigned char col;
  struct square *peers[NUM_PEERS];
  struct square *units[NUM_UNITS][NUM_DIGITS];
} square_t;

typedef struct puzzle {
  square_t squares[NUM_ROWS][NUM_COLS];
} puzzle_t;

void solve(unsigned char grid[9][9]);

// following are  ("private") function declarations --- add as needed

 puzzle_t *create_puzzle(unsigned char grid[9][9]);
 void init_peers_and_units(puzzle_t *puz, int row, int col);
 puzzle_t *copy_puzzle(puzzle_t *puz);
 void free_puzzle(puzzle_t *puz);
 void print_puzzle(puzzle_t *);
 puzzle_t *search(puzzle_t *puz);
 puzzle_t *assign(puzzle_t *puz, int row, int col, char val);
 puzzle_t *eliminate(puzzle_t *puz, int row, int col, char val);
 int holes(puzzle_t *gpuzzle);
 int is_solved(puzzle_t *gpuzzle);

/*************************/
/* Public solve function */
/*************************/
//int alloc = 0;

void solve(unsigned char grid[9][9]) {
  puzzle_t *puz = create_puzzle(grid);
  puzzle_t *solved;
  if ((solved = search(puz)) != NULL) {
    print_puzzle(solved);
  }
  free_puzzle(puz);
}

/*******************************************/
/* Puzzle data structure related functions */
/*******************************************/

puzzle_t *create_puzzle(unsigned char vals[9][9]) 
{
  puzzle_t *allocated_puzzle = malloc(sizeof(puzzle_t));
  int i,j;
  char allval[10] = "123456789";

  for(i=0;i<NUM_ROWS;i++) 
    {
    for(j=0; j<NUM_COLS; j++) 
      {     
	(allocated_puzzle->squares[i][j]).row = i;
	(allocated_puzzle->squares[i][j]).col = j;
	(allocated_puzzle->squares[i][j]).assigned = 0;
	init_peers_and_units(allocated_puzzle,i,j);
	if(vals[i][j] != '0' && vals[i][j] != '.') 
	  {
	    (allocated_puzzle->squares[i][j]).vals[0]=vals[i][j];
	    (allocated_puzzle->squares[i][j]).vals[1]='\0';
	  }
	else 
	  {
	    memcpy(&((allocated_puzzle->squares[i][j]).vals),allval,10);
	  }
      }
    }
  return allocated_puzzle;
}


void init_peers_and_units(puzzle_t *puz, int row, int col) 
{
  int i,j,k=0,z=0;
  int a=0,b=0,c=0;
  int row_div,col_div;
  char is_found = 0;
  square_t *found = &(puz->squares[row][col]);

  //FORMING ROW UNIT and COLUMN UNIT
  for(i=0; i<NUM_ROWS;i++) 
    {
    for(j=0; j<NUM_COLS;j++) 
      {
	if(i==row)
	  {
	    (puz->squares[row][col]).units[0][a++] = &(puz->squares[i][j]);
	  }
	if(j==col)
	  {
	    (puz->squares[row][col]).units[1][b++] = &(puz->squares[i][j]);
	  }    
      }
    }

  //FORMING  ROW and COLUMN PEERS
  for(i=0;i<NUM_ROWS;i++)
    {
    for(j=0;j<NUM_COLS;j++) 
      {
	if(i==row && j!=col) {
	  square_t *sq = &(puz->squares[i][j]);
	  found->peers[k]=sq;
	  k++;
	}
      if(j==col && i!=row) 
	{
	  square_t *pq = &(puz->squares[i][j]);
	  found->peers[k]=pq;
	  k++;
	}    
      }
    }

  
  //FORMING BOX UNIT and BOX PEERS
  row_div = (row/3)*3;
  col_div = (col/3)*3;
  for(i=0;i<3;i++)
    {
    for(j=0;j<3;j++)
      {
	(puz->squares[row][col]).units[2][c++] = &(puz->squares[row_div+i][col_div+j]);
	if(row == row_div+i || col == col_div+j)
	  continue;  
  	(puz->squares[row][col]).peers[k++] = &(puz->squares[row_div+i][col_div+j]); 
      }
    }
 }


 void free_puzzle(puzzle_t *puz) 
 {
  free(puz);
 }


 puzzle_t *copy_puzzle(puzzle_t *puz) {

  int i=0,j=0;
  puzzle_t *p = (puzzle_t *)malloc(sizeof(puzzle_t));
  memcpy(p,puz,sizeof(puzzle_t));

  for(i=0;i<NUM_ROWS;i++){
    for(j=0;j<NUM_COLS;j++){
      init_peers_and_units(p,i,j);
    }
  }
  return p;
}


void print_puzzle(puzzle_t *p) {
  int i, j;
  for (i=0; i<NUM_ROWS; i++) {
    for (j=0; j<NUM_COLS; j++) {
      //printf(" %9s", p->squares[i][j].vals); // may be useful while debugging
      printf(" %2s", p->squares[i][j].vals);
    }
    printf("\n");
  }
}



/**********/
/* Search */
/**********/

 puzzle_t *search(puzzle_t *puz) {
  int i=0,j=0;
  char value,found_v;
  int num_assigned = 0;
  do {
    num_assigned = 0;
    for(i = 0; i < NUM_ROWS; i++) {
      for(j=0;j<NUM_COLS;j++)
	{
	  if(puz->squares[i][j].assigned == 0 && strlen(puz->squares[i][j].vals) == 1)
	    {
	      value = (puz->squares[i][j]).vals[0];
              num_assigned++;
	      if (assign(puz,i,j,value) == NULL) 
		{
		  return NULL;
		}
	    }
	}
    }
  }while(num_assigned > 0);


  if (holes(puz)) {
    printf("FATAL uncaught hole\n");
    print_puzzle(puz);
    exit(1);
  }


  if(is_solved(puz))
    {
      return puz;
    }
  else
    {
      int lp=10;
      int winner_i = -1, winner_j = -1;

      for(i=0;i<NUM_ROWS;i++)
	{
	  for(j=0;j<NUM_COLS;j++)
	    {
              int len = strlen((puz->squares[i][j]).vals);
	      if(len > 1 && len < lp)
		{
		  winner_i=i;
		  winner_j=j;
		  lp = len;
		}
	    }
	}

      if (winner_i == -1 || winner_j == -1) 
	{
	  printf("ERROR!\n");
	  exit(1);
	}

      for(i=0;i<lp;i++)
	{
	  puzzle_t *cpy_puz = copy_puzzle(puz);
	  found_v = (cpy_puz->squares[winner_i][winner_j]).vals[i];
	  assign(cpy_puz,winner_i,winner_j,found_v);
	  puzzle_t *result = search(cpy_puz);
	  if (result != NULL)
	    {
	      if (result != cpy_puz)
		{
		  free_puzzle(cpy_puz);
		}
	    	return result;
	    }
	  else
	    {
	      if (result != cpy_puz)
		{
		  free_puzzle(result);
		}
	      //free_puzzle(result);
	      free_puzzle(cpy_puz);
	    }
	}
    }

  //  free_puzzle(puz);
  return NULL; // ALL GUESSES FAILED
}



// CHECKS IF A PUZZLE IS SOLVED
 int is_solved(puzzle_t *gpuzzle)
{
  int i=0,j=0,bool=0;
  for(i=0;i<NUM_ROWS;i++)
    {
      for(j=0;j<NUM_COLS;j++)
	{
	  if(strlen((gpuzzle->squares[i][j]).vals)!=1)
	    {
	      bool=1;
	    }
	}
    }
  return !bool;
}


 int holes(puzzle_t *gpuzzle)
{
  int i=0,j=0,bool=0;
  for(i=0;i<NUM_ROWS;i++)
    {
      for(j=0;j<NUM_COLS;j++)
	{
	  if(strlen((gpuzzle->squares[i][j]).vals)==0)
	    {
	      bool=1;
              printf("hole at <%d,%d>\n", i, j);
	    }
	}
    }
  return bool;
}

/**************************/
/* Constraint propagation */
/**************************/

//ASSIGNS A PARTICULAR VALUE TO A SQUARE AND ELIMINATES THAT VALUE FROM ITS PEERS

 puzzle_t *assign(puzzle_t *puz, int row, int col, char val) {
 square_t *sqr = &(puz->squares[row][col]);

      sqr->vals[0]=val;
      sqr->vals[1]='\0';
      sqr->assigned = 1;
      int p;
      for (p = 0; p < NUM_PEERS; p++) {
         if (eliminate(puz, sqr->peers[p]->row, sqr->peers[p]->col, val) == NULL) {
             return NULL;
         }
      }
      return puz;
}


//REMOVES VALUES FROM ALL PEERS AND ASSIGNS A PARTICULAR VALUE IF FOUND
 puzzle_t *eliminate(puzzle_t *puz, int row, int col, char val) {

  square_t *sqr = &(puz->squares[row][col]);
  char *c = strchr(sqr->vals, val);

  if(!c) {
    return puz;
  }

  while(*c)
    {
      *c = *(c+1);
      c++;
    }

  int values_left = strlen(sqr->vals);
  if(values_left == 0) {
     return NULL;
  }
  return puz;
}

/*****************************************/
/* Misc (e.g., utility) functions follow */
/*****************************************/
