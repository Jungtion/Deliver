#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

#pragma warning(disable: 4996) // Visual Studio에서 scanf를 사용하기 위함이니 지워도됩니다.

/* 
  definition of storage cell structure ----
  members :
  int building : building number of the destination
  int room : room number of the destination
  int cnt : number of packages in the cell
  char passwd[] : password setting (4 characters)
  char *contents : package context (message string)
*/
typedef struct {
	int building;
	int room;
	int cnt;
	char passwd[PASSWD_LEN+1];
	
	char *context;
} storage_t;


static storage_t** deliverySystem; 			//deliverySystem
static int storedCnt = 0;					//number of cells occupied
static int systemSize[2] = {0, 0};  		//row/column of the delivery system
static char masterPassword[PASSWD_LEN+1];	//master password
static FILE* fp = NULL;



// ------- inner functions ---------------

//print the inside context of a specific cell
//int x, int y : cell to print the context
static void printStorageInside(int x, int y) {
	printf("\n------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n");
	if (deliverySystem[x][y].cnt > 0)
		printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
	else
		printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		
	printf("------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n\n");
}

//initialize the storage
//set all the member variable as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {
	int i;
	
	//각각 기본값으로 초기화
	deliverySystem[x][y].building = 0;
	deliverySystem[x][y].room = 0;
	deliverySystem[x][y].context = (char*)malloc(sizeof(char) * 100);
	deliverySystem[x][y].cnt = 0;
	for(i = 0; i < PASSWD_LEN + 1; ++ i)
	{
		deliverySystem[x][y].passwd[i] = 0;
	}
}

//get password input and check if it is correct for the cell (x,y)
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {
	char passwd[PASSWD_LEN + 1];
	int i, j;

	printf(" - password : ");
	scanf("%4s", passwd);
	fflush(stdin);

	for(i = 0; i < PASSWD_LEN + 1; ++ i)
	{
		if (strcmp(passwd, "1234") == 0) { break; } // 마스터키일시 반복문 빠져나오기

		if(deliverySystem[y][x].passwd[i] != passwd[i])
		{
			return -1;
		}
	}

	return 0;
}





// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {
	int i, j;

	if(fp != NULL) // 이미 파일이 열려있다면 닫는다
	{
		fclose(fp);
	}

	fp = fopen(filepath, "w"); //파일 쓰기모드로 열기 읽기X
	if(fp == NULL)
	{
		return -1;
	}

	fprintf(fp, "%d %d\n", systemSize[0], systemSize[1]);
	fprintf(fp, "%s\n", masterPassword);

	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			if (deliverySystem[i][j].cnt > 0)
			{
				fprintf(fp, "%d %d %d %d %s %s\n", i, j, deliverySystem[i][j].building, deliverySystem[i][j].room, deliverySystem[i][j].passwd, deliverySystem[i][j].context);
			}
		}
	}
	
	return 0;
}



//create delivery system on the double pointer deliverySystem
//char* filepath : filepath and name to read config parameters (row, column, master password, past contexts of the delivery system
//return : 0 - successfully created, -1 - failed to create the system
int str_createSystem(char* filepath) {
	int row = 0;
	int column = 0;
	int i, j;

	fp = fopen(filepath, "r"); // 파일을 읽기모드로 열기 쓰기 X

	if(fp == NULL)
	{
		return -1;
	}
	
	fscanf(fp, "%d%d", &row, &column); // 열과 행을 받는 부분
	fscanf(fp, "%s", masterPassword); // 마스터키 얻는 부분

	//SystemSize 정의 (택배 보관함 크기)
	systemSize[0] = row;
	systemSize[1] = column;

	//!< deliverySystem을 사용하기 위해 동적할당을 하는 부분
	deliverySystem = (storage_t**)malloc(sizeof(storage_t) * row);
	for(i = 0; i < row; ++ i)
	{
		deliverySystem[i] = (storage_t*)malloc(sizeof(storage_t) * column);
	}

	//택배보관함 초기화
	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			initStorage(i, j);
		}
	}

	while (!feof(fp)) // 파일 끝까지 읽는다
	{
		//파일을 읽기위한 지역변수들
		int row, column;
		int building, room;
		char passwd[PASSWD_LEN + 1];
		char context[100];

		int i;

		//파일 한줄씩 읽기
		if(fscanf(fp, "%d%d%d%d%s%s", &row, &column, &building, &room, passwd, context) != 6)
		{
			break;
		}

		//정보 저장
		deliverySystem[row][column].building = building;
		deliverySystem[row][column].room = room;
		deliverySystem[row][column].cnt = 1; // 택배가 차있음

		for(i = 0; i < PASSWD_LEN + 1; ++ i)
		{
			deliverySystem[row][column].passwd[i] = passwd[i];
		}
		if(context != NULL)
		{
			strcpy(deliverySystem[row][column].context, context); // 문자열 복사
		}
	}

	return 0;
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {
	if(fp != NULL) // 파일 해제
	{
		fclose(fp); 
		fp = NULL;
	} 

	if(deliverySystem != NULL) 
	{
		int i, j;
		for (i = 0; i < systemSize[0]; i++)
		{
			for (j = 0; j < systemSize[1]; j++)
			{
				free(deliverySystem[i][j].context); // 동적할당을 해준 context부터 릴리즈
			}
		}

		for(i = 0; i < systemSize[0]; ++ i) // 차례대로 보관함 해제
		{
			free(deliverySystem[i]);
		}

		free(deliverySystem);
	}
}



//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0]*systemSize[1]);
	
	printf("\t");
	for (j=0;j<systemSize[1];j++)
	{
		printf(" %i\t\t",j);
	}
	printf("\n-----------------------------------------------------------------------------------------------------------------\n");
	
	for (i=0;i<systemSize[0];i++)
	{
		printf("%i|\t",i);
		for (j=0;j<systemSize[1];j++)
		{
			if (deliverySystem[i][j].cnt > 0)
			{
				printf("%i,%i\t|\t", deliverySystem[i][j].building, deliverySystem[i][j].room);
			}
			else
			{
				printf(" -  \t|\t");
			}
		}
		printf("\n");
	}
	printf("--------------------------------------- Delivery Storage System Status --------------------------------------------\n\n");
}



//check if the input cell (x,y) is valid and whether it is occupied or not
int str_checkStorage(int x, int y) {
	if (x < 0 || x >= systemSize[0])
	{
		return -1;
	}
	
	if (y < 0 || y >= systemSize[1])
	{
		return -1;
	}
	
	return deliverySystem[x][y].cnt;	
}


//put a package (msg) to the cell
//input parameters
//int x, int y : coordinate of the cell to put the package
//int nBuilding, int nRoom : building and room numbers of the destination
//char msg[] : package context (message string)
//char passwd[] : password string (4 characters)
//return : 0 - successfully put the package, -1 - failed to put
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE+1], char passwd[PASSWD_LEN+1]) {
	/*
	!예외 처리가 필요할수도 있는 코드
	*/

	//차례대로 정보를 입력
	int i;

	deliverySystem[x][y].building = nBuilding;
	deliverySystem[x][y].building = nRoom;
	strcpy(deliverySystem[x][y].context, msg);

	for(i = 0; i < PASSWD_LEN+1; ++ i)
	{
		deliverySystem[x][y].passwd[i] = passwd[i];
	}
	
	deliverySystem[x][y].cnt = 1;

	return 0;
}



//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {
	char passwd[PASSWD_LEN + 1];
	int i;

	if(inputPasswd(x, y) != 0)
	{
		return -1;
	}

	printStorageInside(x, y);

	initStorage(x, y);

	return 0;
}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {
	int i, j;
	int cnt = 0;

	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			if(deliverySystem[i][j].building == nBuilding && deliverySystem[i][j].room == nRoom)
			{
				printf(" -----------> Found a package in (%d, %d)\n", i, j);
				cnt ++;
			}
		}
	}
	return cnt;
}
