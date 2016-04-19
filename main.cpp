#ifndef NDEBUG
#define PDEBUG 1
#endif

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
#include <cctype>
#include <vector>
#include "checkers.h"
#include "boardpos.h"
#include "inputerror.h"
#include "connectfour.h"

//using namespace std;

//board position
//    c0 c1 c2 ...
// r0
// r1
// r2
// ...

std::string player1 = "Player 1";
std::string player2 = "Player 2";

BoardPos getPos(const char*);
ifpos_pair_type getPosPair();

int **board = NULL;
const int INITBOARD[ROW][COL] = { { RED, VACANT, RED, VACANT, RED, VACANT, RED, VACANT }, { VACANT, RED, VACANT, RED, VACANT, RED, VACANT, RED }, { RED, VACANT, RED, VACANT, RED, VACANT, RED, VACANT }, { VACANT, VACANT, VACANT, VACANT, VACANT, VACANT, VACANT, VACANT }, { VACANT, VACANT, VACANT, VACANT, VACANT, VACANT, VACANT, VACANT }, { VACANT, BLACK, VACANT, BLACK, VACANT, BLACK, VACANT, BLACK }, { BLACK, VACANT, BLACK, VACANT, BLACK, VACANT, BLACK, VACANT }, { VACANT, BLACK, VACANT, BLACK, VACANT, BLACK, VACANT, BLACK } };

void initBoard() {
    if(board != NULL) {
        for(int i = 0; i < ROW; ++i)
            delete board[i];
        delete board;
    }
    board = new int*[ROW];
    for(int i = 0; i < ROW; ++i) {
        board[i] = new int[COL];
        for(int j = 0; j < COL; ++j)
            board[i][j] = INITBOARD[i][j];
    }
    BoardPos::board = ::board;
}

int numRedsAlive = 24;
int numBlacksAlive = 24; //#BlackLivesMatter

bool blackTurn = false;

int getTurn() {
    return blackTurn ? BLACK : RED;
}

char pieceRep(int i) {
    switch(i) {
        case RED:
            return 'r';
        case RKING:
            return 'R';
        case BLACK:
            return 'b';
        case BKING:
            return 'B';
    }
    return ' ';
}

#if PDEBUG
void printBoard() {
    for(int l = 0; l < ROW; ++l) {
        std::cout << std::setw(1) << l + 1;
        for(int m = 0; m < COL; ++m)
            std::cout << std::setw(2) << pieceRep(*BoardPos(l, m));
        std::cout << std::endl;
    }
    std::cout << "  A B C D E F G H" << std::endl;
    std::cout << "# Reds Taken = " << 24 - numRedsAlive << std::endl;
    std::cout << "# Blacks Taken = " << 24 - numBlacksAlive << std::endl;
    std::cout << (blackTurn ? player2 : player1) << "'s turn!" << std::endl;
}
#else
void printBoard() {
    std::cout << " x---x---x---x---x---x---x---x---x\n";
    for(int l = 0; l < ROW; ++l) {
        std::cout << std::setw(1) << l + 1;
        for(int m = 0; m < COL; ++m)
            std::cout << std::setw(4) << pieceRep(*BoardPos(l, m));
        std::cout << "\n x---x---x---x---x---x---x---x---x\n";
    }
    std::cout << "    A   B   C   D   E   F   G   H\n";
}
#endif

int columnFinder(const std::string& p) {
    int c = int(p[0]);
    if('A' <= c && c <= 'H') return c - 'A';
    if('a' <= c && c <= 'h') return c - 'a';
    return -1;
}

void moveCheck(const BoardPos& i, const BoardPos& f) {
    BoardPos d(f - i);
    d.r = std::abs(d.r);
    d.c = std::abs(d.c);
    
    //time to play baseball
    if(isVacant(*i)) throw input_error("can only move a red or black piece");
    if(!isMatching(*i, getTurn())) throw input_error("can only move your own piece");
    if(!isVacant(*f)) throw input_error("can only move to an empty space");
    if(d.r != d.c) throw input_error("can only move diagonal");
    if(d.r > 2) throw input_error("can only move as far as 2 diagonal spaces");
    if(d.r == 0) throw input_error("cannot stay at the same spot");
    if(d.r == 2 && !isOpposite(*i, *(i / f))) throw input_error("can only jump over opposite piece");
    if(!isForward(i, f)) throw input_error("can only move forward");
}

std::vector<BoardPos> getValidJumps(const BoardPos& p) {
    const BoardPos main(2, 2), anti(2, -2);
    BoardPos f[] = { p + main, p - anti, p - main, p + anti };
    std::vector<BoardPos> ret;
    for(unsigned i = 0; i < 4; ++i) {
        try {
            moveCheck(p, i[f]);
            ret.push_back(f[i]);
        } catch(const std::exception& e) {
        }
    }
    return ret;
}

//code's a bit jumbled here
//forgive me plz :c
BoardPos jumpFrom(const BoardPos& p) {
    auto valid = getValidJumps(p); //number of valid jumps from p
    if(valid.size() == 0) {
        blackTurn = !blackTurn;
        return p;
    }
    {
        std::cout << "Continue jumping? (Y/N)\n";
        char c = '\0';
        while(c != 'y') {
            std::string s;
            std::cin >> s;
            if((c = std::tolower(s[0])) == 'n') {
                blackTurn = !blackTurn;
                return p;
            }
        }
    }
    BoardPos jump = valid[0];
    if(valid.size() != 1) {
        jump = BoardPos(-1, -1);
        bool move = false;
        while(!move)
            try { //TODO: implement lambda for try/catch
                moveCheck(p, jump);
                jump = getPos("Where would you like to jump to?\n");
            } catch(const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
    }
    *(p / jump) = VACANT;
    std::swap(*p, *jump);
    if(isAtEnd(p)) *p = toKing(*p);
    return jump;
}

bool movePiece(const BoardPos& i, const BoardPos& f) {
    moveCheck(i, f);
    if(std::abs((f - i).r) == 1) {
        std::swap(*i, *f);
        blackTurn = !blackTurn;
        if(isAtEnd(f)) *f = toKing(*f);
        return false;
    }
    if(isRed(*(i / f))) --numRedsAlive;
    else --numBlacksAlive;
    *(i / f) = VACANT;
    std::swap(*i, *f);
    if(isAtEnd(f)) *f = toKing(*f);
    return true;
}

inline bool movePiece(const ifpos_pair_type& p) {
    return movePiece(p.first, p.second);
}

BoardPos getPos(const char* msg) {
    std::cout << msg;
    std::string s;
    std::cin >> s;
    if(s.length() != 2) throw input_error("input length is not 2");
    int r = int(s[1]) - int('1'), c = columnFinder(s);
    if(c == -1) throw input_error("invalid column");
    if(!(0 <= r && r < ROW)) throw input_error("invalid row");
    return BoardPos(r, c);
}

//Ask the user what pieces they want to move
ifpos_pair_type getPosPair() {
    BoardPos i = getPos("What piece do you want to move?\n");
    BoardPos f = getPos("Where do want to move it to?\n");
    return ifpos_pair_type(i, f);
}

void play() {
    printBoard();
    try {
        auto move = getPosPair();
        BoardPos i, f;
        if(movePiece(move)) {
            printBoard();
            i = move.second;
            while(!((f = jumpFrom(i)) == i)) {
                printBoard();
                i = f;
            }
        }
    } catch(const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

void setPlayer1Name(std::string name){
    player1 = name;
}

std::string getPlayer1Name(){
    return player1;
}

void setPlayer2Name(std::string name){
    player2 = name;
}

std::string getPlayer2Name(){
    return player2;
}


int main() {
    
    int checkersChoice = 0;
    int connectChoice = 0;
    int gameChoice = 0;
    
    do{
        std::cout<<"Welcome to our Game Box!";
        std::cout<<std::endl;
        std::cout<<"1. Play Checkers";
        std::cout<<std::endl;
        std::cout<<"2. Play Connect 4";
        std::cout<<std::endl;
        std::cout<<"3. Exit";
        std::cout<<std::endl;
        
        std::cin>>gameChoice;
        
        if(gameChoice == 1){
            do{
                
                std::cout<<"Welcome to Checkers!";
                std::cout<<std::endl;
                std::cout<<"1. Choose names of players";
                std::cout<<std::endl;
                std::cout<<"2. How To Play";
                std::cout<<std::endl;
                std::cout<<"3. Play";
                std::cout<<std::endl;
                std::cout<<"4. Exit";
                std::cout<<std::endl;
                
                std::cin>>checkersChoice;
                
                while(checkersChoice == 1){
                    std::string name1;
                    std::string name2;
                    int playerChoice;
                    std::cout<<player1; std::cout<<" is Red. "; std::cout<<player2; std::cout<<" is Black. Which player name would you like to change? 1 or 2? Choose 0 to exit.";
                    std::cout<<std::endl;
                    std::cin>>playerChoice;
                    if(playerChoice == 1){
                        std::cout<<"What would you like to change Player 1's name to?";
                        std::cout<<std::endl;
                        std::cin>>name1;
                        setPlayer1Name(name1);
                    }
                    if(playerChoice == 2){
                        std::cout<<"What would you like to change Player 2's name to?";
                        std::cout<<std::endl;
                        std::cin>>name2;
                        setPlayer2Name(name2);
                    }
                    if(playerChoice == 0){
                        break;
                    }
                    if(playerChoice != 1 && playerChoice != 2 && playerChoice != 0){
                        std::cout<<"Invalid Choice. Please choose 1 or 2.";
                        std::cout<<std::endl;
                        std::cout<<std::endl;
                    }
                }
                if(checkersChoice == 2){
                    std::cout<<"This is a 2 player game of Checkers. Once the game begins, "; std::cout<<player1; std::cout<<" has the red pieces and "; std::cout<<player2; std::cout<<" has the black pieces. In order to move your pieces, simply type what row and column the piece is in and what row and column to move the piece to. Be sure to type the column letter first, then the row number. For example: A1 or D6. You can only move pieces diagonally forward, until you become a King piece and can move backwards. You become a King piece by getting to your opponent's side of the board. Have fun and good luck!";
                    std::cout<<std::endl;
                    std::cout<<std::endl;
                }
                if(checkersChoice == 3){
                    initBoard();
                    while(numRedsAlive && numBlacksAlive)
                        play();
                    std::cout << "Winner: " << (!numBlacksAlive ? "Black" : "Red");
                }
                if(checkersChoice == 4){
                    break;
                }
            }while(checkersChoice == 1 || checkersChoice == 2 || checkersChoice == 3);
        }
        if(gameChoice == 2){
            
            do{
            
            std::cout<<"Welcome to Connect Four!";
            std::cout<<std::endl;
            std::cout<<"1. Play";
            std::cout<<std::endl;
            std::cout<<"2. Exit";
            std::cout<<std::endl;
            
            std::cin>>connectChoice;
            
            if(connectChoice == 1){
            con4::initBoard();
            while(con4::isVacant(con4::winner()))
                con4::play();
            con4::printBoard();
            std::cout << "Winner: " << (con4::isBlackTurn ? "Red" : "Black") << std::endl;
            }
            if(connectChoice == 2){
                break;
            }
            }while(connectChoice == 1);
        }
        if(gameChoice == 3){
            break;
        }
    }while(gameChoice == 1 || gameChoice == 2);
}