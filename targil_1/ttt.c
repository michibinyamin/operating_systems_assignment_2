#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ROW_COL 3 //size of matrix (3X3)
int digitsPlay[9]; //steps of the program
int matrixPlay[ROW_COL][ROW_COL] = {0}; //the matrix of ttt

/*
 * function that check invalid arg
 */
void input_validation(int argc , char *argv[])
{
    //Too many parameters
    if(argc > 2)
    {
        printf("Error- you need to input *only one* arg!\n");
        exit(EXIT_FAILURE);  // Terminate the program successfully
    }

    //no parameter
    if(argc == 1)
    {
        printf("Error- you need to input arg, for example: '156324789'!\n");
        exit(EXIT_FAILURE);
    }

    char* inputPlay = argv[1];
    unsigned long int size = strlen(inputPlay);
    int countD[10]={0}; //hold digits appears
    int converTodigit;


    //only 9 digit
    if(size != 9)
    {
        printf("Error- you need to input arg with only 9 digits length!\n");
        exit(EXIT_FAILURE);
    }

    //only digit 1-9
    for(size_t i = 0; i < size; i++)
    {
        char c = inputPlay[i];
        if(c < '1' || c > '9')
        {
            printf("Error- you need to only digits 1-9!\n");
            exit(EXIT_FAILURE);
        }

        converTodigit = c - '0';
        countD[converTodigit]++;
        if(countD[converTodigit] > 1)
        {
            printf("Error- there is a digit that appears more than once\n");
            exit(EXIT_FAILURE);
        }

        digitsPlay[i] = converTodigit;
    }

    //if digit appears more than once
    for (int i = 1; i < 10; ++i)
    {
        if(countD[i] != 1)
        {
            printf("Error- there is a digit that appears more than once\n");
            exit(EXIT_FAILURE);
        }
    }

}

/*
 * func that check if the game finished.
 * if the program win-will print "I WIN"
 * else-will print "I LOST"
 */

bool checkFinishGame()
{
    //check diagonals
    if (matrixPlay[0][0] == matrixPlay[1][1] && matrixPlay[1][1] == matrixPlay[2][2] && matrixPlay[0][0] != 0)
    {
        if(matrixPlay[0][0] == -1)
        {
            printf("I WIN!\n");
            return true;
        } else if(matrixPlay[0][0] == -2){
            printf("I LOST.. \n");
            return true;
        }
    }

    if (matrixPlay[0][2] == matrixPlay[1][1] && matrixPlay[1][1] == matrixPlay[2][0] && matrixPlay[0][2] != 0)
    {
        if(matrixPlay[0][2] == -1)
        {
            printf("I WIN!\n");
            return true;
        } else if(matrixPlay[0][2] == -2){
            printf("I LOST.. \n");
            return true;
        }
    }

    //check ROWS and COLS
    for (int i = 0; i < ROW_COL; ++i)
    {
        if(matrixPlay[i][0]==matrixPlay[i][1] && matrixPlay[i][1]==matrixPlay[i][2] && matrixPlay[i][0] != 0)
        {
            if(matrixPlay[i][0] == -1)
            {
                printf("I WIN!\n");
                return true;
            } else if(matrixPlay[i][0] == -2){
                printf("I LOST.. \n");
                return true;
            }
        }
        if(matrixPlay[0][i]==matrixPlay[1][i] && matrixPlay[1][i]==matrixPlay[2][i] && matrixPlay[0][i] != 0)
        {
            if(matrixPlay[0][i] == -1)
            {
                printf("I WIN!\n");
                return true;
            } else if(matrixPlay[0][i] == -2){
                printf("I LOST.. \n");
                return true;
            }
        }
    }
    return false;
}


/*
 * Function that updates the computers list of moves
 */
void update_list(int position)
{
    for (size_t i = 0; i < sizeof(digitsPlay); i++)
    {
        if (digitsPlay[i] == position)
        {
            digitsPlay[i] = 0;  // Mark as played
            return;
        }
    }
}


// program step - check in list for the next move + add to board
void programStep()
{
    int position;
    for (size_t i = 0; i < sizeof(digitsPlay); i++)
    {
        position = digitsPlay[i];
        if (position != 0)  // If has not been played yet
        {
            ((int*)matrixPlay)[position-1] = -1;  // Mark on board
            update_list(position); // Mark in list as played
            printf("Computer moved on : %d\n", position);
            return;
        }
    }
}

// player's step - check if legal move + add to board
void playerStep()
{
    int position;
    bool legal = false;
    int is_digit;
    // A loop which makes sure that player's move is legal
    while (!legal)
    {
        printf("Your move : ");
        is_digit = scanf("%d", &position);  // If is_digit is 0 then the user did not input a number
        if (position > 9 || position < 1 || is_digit != 1)  // user input is not between 1 and 9
        {
            getchar();  // In case there was a digit input
            printf("please enter a number between 1 and 9\n");
            legal = false;
        }

        else    // user input is between 1 and 9
        {
            if (((int*)matrixPlay)[position-1] != 0)  // Position is taken
            {     
                printf("This position is taken\n");
                legal = false;
            }

            else    // Valid input
            {   
                ((int*)matrixPlay)[position-1] = -2;  // Mark on board
                update_list(position);  // Mark in list as played
                printf("moved succesfully\n");
                legal = true;
            }   
        }
    }
}

int main(int argc , char *argv[])
{
    // Checks if input is correct + add the values to the digitplay
    input_validation(argc,argv);
    bool turn = true;   // true = computer's turn. false = player's turn

    // A loop which itarates for all of the moves needed
    for (size_t i = 0; i < ROW_COL*ROW_COL; i++)
    {
        if (turn)   // Computers turn
        {
            programStep();
            turn = false;
        }
        else        // Player's turn
        {
            playerStep();
            turn = true;
        }

        if (checkFinishGame())
        {
            return 0;
        }
    }
    // End of the loop the board is full, so if it has reached here then there is no winner, tie.
    printf("TIE!\n");
    return 0;
}