// Sudoku.cpp : Defines the entry point for the console application.
//
//2 number location
//  column, row (this is often referred to in reverse order
//  because it's often hard to conceptionalize what exactly a column is)
//3 number location
//  square, column, row
//

#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>

using namespace std;

struct guess {
    int r;
    int c;
    int a;
    string guess_type;
};

class Sudoku_Board {
    int smallboards[9][3][3];
    int possible[9][9][9];
public:
    int board[9][9];
    string inputs[9];
    bool multiple_ans;
    bool unsolvable;
    bool stop_solving;
    void initialize();
    void initialize(Sudoku_Board copy);
    void output();
    bool done();
    string validate_board();
    void solve();
    //solving functions
    void set_possible(int i, int j, int k, int l);
    bool check_for_solve();
    void new_number(int i, int j, int k, int l, int answer);
    void guess_and_check(int i, int j, int k, int l, int answer, string guess_type);
    void wrong_guess();
};

string addcommas(string str);
void commaHandling(string& puzzlecommas);
string removezeros(string str);

void main_game();
void create_board();
void instructions();
void settings();
string validate_row(string input);


//settings
bool slowly = false;
bool hide_board = false;
bool sound_when_finished = false;
bool check_for_multi_ans = false;

vector<Sudoku_Board> backups;
vector<guess> guesses;
Sudoku_Board Board;
Sudoku_Board ans;

char inputtype;
int main()
{
    system("title Sudoku Solver");
    //system("mode 1000");
    string input;
    //main menu
    do {
        cout << "Sudoku Solver" << endl
        << "created by Logan Seaburg and Jackson Feddock" << endl << endl;
        cout << "Enter 1-5 based on the list:" << endl
        << endl
        << "    1. Solver" << endl
        << "    2. Create Board (In Beta)" << endl
        << "    3. Instructions" << endl
        << "    4. Settings" << endl
        << "    5. Quit" << endl
        << endl
        << "Where would you like to go? : ";
        cin >> input;
        if (input == "1")
        {
            system("CLS");
            main_game();
            system("CLS");
        }
        if (input == "2")
        {
            system("CLS");
            create_board();
            system("CLS");
        }
        else if (input == "3")
        {
            system("CLS");
            instructions();
            system("CLS");
        }
        else if (input == "4")
        {
            system("CLS");
            settings();
            system("CLS");
        }
        if (input != "1" && input != "2" && input != "3" && input != "4" && input != "5")
        {
            system("CLS");
        }
    } while (input != "5");
    //if you're feeling very ambitious, save the current settings in an outside file for next time
    return 0;
}

void main_game() {
    Sudoku_Board beginning;
    backups.clear();
    guesses.clear();
    time_t start, end;
    int i;
    string error,commas,input;
    //Enter the current board pattern
puzzleinput:
    //cout << "Enter method of input, either (C)ommas or (R)ows: ";
    cout << "Enter comma-delimited form or row 1: ";
    cin >> input;
    if (input.length() == 9)
        inputtype = 'r';
    else
        inputtype = 'c';
    if (inputtype == 'c')
    {
        commaHandling(input);
        for (int i = 0; i < 9; i++)
        {
            Board.inputs[i] = input.substr(i * 9, 9);
            error = validate_row(Board.inputs[i]);
            if (error != "")
                break;
        }
        if (error != "")
            goto puzzleinput;
    }
    else if (inputtype == 'r')
    {
        Board.inputs[0] = input;
        error = validate_row(Board.inputs[0]);
        if (error != "")
            goto puzzleinput;
        for (i = 1; i < 9; i++)
        {
            cout << "Enter row " << i + 1 << ": ";
            cin >> Board.inputs[i];
            error = validate_row(Board.inputs[i]);
            while (error != "")
            {
                cout << error << " Enter a new number for row " << i + 1 << ": ";
                cin >> Board.inputs[i];
                error = validate_row(Board.inputs[i]);
            }
        }
    }
    Board.initialize();
    beginning = Board;
    error = Board.validate_board();
    if (error != "")
    {
        cout << "The board is not valid. " + error << endl;
        system("pause");
        return;
    }
    cout << endl;
    system("CLS");
    Board.output();
    //start solving the game
    time(&start);
    Board.solve();
    time(&end);
    system("CLS");
    beginning.output();
    if (Board.unsolvable)
        cout << "This board has no solution currently." << endl << endl;
    else
    {
        if (Board.multiple_ans)
            cout << "The Board entered has multiple solutions. One is shown below." << endl << endl << endl;
        Board.output();
    }
    commas = "";
    for (i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                for (int l = 0; l < 3; l++)
                    commas += to_string(static_cast<long long>(Board.board[i * 3 + k][j * 3 + l]));
    cout << "Commas: " << addcommas(commas) << endl;
    //outputs time taken
    double dif = difftime(end, start);
    cout << "Time taken: ";
    if (dif > 60)
        cout << (int)dif / 60 << " minutes and " << dif - ((int)dif / 60) * 60 << " seconds" << endl;
    else
        cout << dif << " seconds" << endl;
    //makes noise
    system("pause");
    return;
}

void Sudoku_Board::solve()
{
    int i, j, k, l;
    unsolvable = false;
    multiple_ans = false;
    stop_solving = false;
    //sets possible values for all spaces that equal 0
    bool check_again = true;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            for (k = 0; k < 3; k++)
                for (l = 0; l < 3; l++)
                    //if the number is not known
                    if (board[i * 3 + k][j * 3 + l] == 0)
                        //big row, big column, small row, small column
                        set_possible(i, j, k, l);
    int infinite = 0;
    while (check_again)
    {
        check_again = check_for_solve();
        if (validate_board() != "")
        {
            wrong_guess();
            check_again = true;
        }
        else if (done())
            check_again = false;
        if (stop_solving)
            check_again = false;
    }
    if (done())
    {
        if (backups.empty())
            return;
        else if (!check_for_multi_ans)
            return;
        else
        {
            ans = Board;
            //tries to solve the board again to see if there are multiple solutions
            check_again = true;
            wrong_guess();
            //same solve the board function
            while (check_again)
            {
                check_again = check_for_solve();
                if (validate_board() != "")
                {
                    wrong_guess();
                    check_again = true;
                }
                else if (done())
                {
                    ans.multiple_ans = true;
                    check_again = false;
                }
                if (stop_solving)
                    check_again = false;
            }
            Board = ans;
            return;
        }
    }
    unsolvable = true;
    return;
}
//sets all possible values for all unknown squares
void Sudoku_Board::set_possible(int i, int j, int k, int l)
{
    //counter, counter, row, column, square, row in square layout, column in square layout
    int m, n, r, c, s;
    r = i * 3 + k;
    c = j * 3 + l;
    s = i * 3 + j;
    for (m = 0; m < 9; m++)
    {
        //checks rows
        if (board[r][m] != 0)
            possible[r][c][board[r][m] - 1] = 0;
        //checks columns
        if (board[m][c] != 0)
            possible[r][c][board[m][c] - 1] = 0;
    }
    //checks box
    for (m = 0; m < 3; m++)
        for (n = 0; n < 3; n++)
            if (smallboards[s][m][n] != 0)
                possible[r][c][smallboards[s][m][n] - 1] = 0;
}

bool Sudoku_Board::check_for_solve()
{
    //checks if any square has only 1 possibility
    int i, j, k, l, m, n, temp;
    for (n = 1; n < 10; n++)
    {
        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                for (k = 0; k < 3; k++)
                    for (l = 0; l < 3; l++)
                        if (board[i * 3 + k][j * 3 + l] == 0)
                        {
                            temp = 0;
                            for (m = 0; m < 9; m++)
                                if (possible[i * 3 + k][j * 3 + l][m] != 0)
                                    temp++;
                            if (temp == 0)
                            {
                                wrong_guess();
                                return true;
                            }
                            else if (temp == n)
                            {
                                temp = 0;
                                for (m = 0; m < 9; m++)
                                {
                                    temp += possible[i * 3 + k][j * 3 + l][m];
                                    if (temp != 0)
                                        break;
                                }
                                if (n == 1)
                                    new_number(i, j, k, l, temp);
                                else
                                    guess_and_check(i, j, k, l, temp, "one_possibility");
                                return true;
                            }
                        }
        //it checks squares that have one possible value
        //needs to check rows and columns and big squares
        //  where only 1 non-taken square can be a number
        //the row
        for (i = 0; i < 9; i++)
            //the value being checked
            for (m = 0; m < 9; m++)
            {
                temp = 0;
                //moving along the row
                for (j = 0; j < 9; j++)
                    if (possible[i][j][m] != 0 && board[i][j] == 0)
                        temp++;
                if (temp == n)
                {
                    j = 0;
                    while (!(possible[i][j][m] != 0 && board[i][j] == 0))
                        j++;
                    if (n == 1)
                        new_number((int)i / 3, (int)j / 3, i % 3, j % 3, m + 1);
                    else
                        guess_and_check((int)i / 3, (int)j / 3, i % 3, j % 3, m + 1, "row");
                    return true;
                }
            }
        //the column
        for (j = 0; j < 9; j++)
            //the value being checked
            for (m = 0; m < 9; m++)
            {
                temp = 0;
                //moving along the row
                for (i = 0; i < 9; i++)
                    if (possible[i][j][m] != 0 && board[i][j] == 0)
                        temp++;
                if (temp == n)
                {
                    i = 0;
                    while (!(possible[i][j][m] != 0 && board[i][j] == 0))
                        i++;
                    if (n == 1)
                        new_number((int)i / 3, (int)j / 3, i % 3, j % 3, m + 1);
                    else
                        guess_and_check((int)i / 3, (int)j / 3, i % 3, j % 3, m + 1, "column");
                    return true;
                }
            }
        //boxes
        for (i = 0; i < 9; i++)
            for (m = 0; m < 9; m++)
            {
                temp = 0;
                for (j = 0; j < 3; j++)
                    for (k = 0; k < 3; k++)
                        if (smallboards[i][j][k] == 0 && possible[((int)i / 3) * 3 + j][(i % 3) * 3 + k][m] != 0)
                            temp++;
                if (temp == n)
                {
                    j = 0;
                    k = 0;
                    while (!(smallboards[i][j][k] == 0 && possible[((int)i / 3) * 3 + j][(i % 3) * 3 + k][m] != 0))
                    {
                        k++;
                        if (k == 3)
                        {
                            k = 0;
                            j++;
                        }
                    }
                    if (n == 1)
                        new_number((int)i / 3, i % 3, j, k, m + 1);
                    else
                        guess_and_check((int)i / 3, i % 3, j, k, m + 1, "box");
                    return true;
                }
            }
    }
    return false;
}

void Sudoku_Board::new_number(int i, int j, int k, int l, int answer)
{
    int m, n, r, c, s;
    r = i * 3 + k;
    c = j * 3 + l;
    s = i * 3 + j;
    board[r][c] = answer;
    smallboards[s][k][l] = answer;
    if (!hide_board)
    {
        system("CLS");
        output();
    }
    //look around for other values that were affected
    for (m = 0; m < 9; m++)
    {
        //checks rows (really a row)
        if (board[r][m] == 0)
            set_possible(i, (int)m / 3, k, m % 3);
        //checks columns (really a column)
        if (board[m][c] == 0)
            set_possible((int)m / 3, j, m % 3, l);
    }
    //checks box
    for (m = 0; m < 3; m++)
        for (n = 0; n < 3; n++)
            if (smallboards[s][m][n] == 0)
                set_possible(i, j, m, n);
}

void Sudoku_Board::guess_and_check(int i, int j, int k, int l, int answer, string guess_type)
{
    guess temp;
    //creates a backup for a tree of possibilites
    backups.push_back(*this);
    //stores the guess
    temp.r = i * 3 + k;
    temp.c = j * 3 + l;
    temp.a = answer;
    temp.guess_type = guess_type;
    guesses.push_back(temp);
    new_number(i, j, k, l, answer);
    return;
}

void Sudoku_Board::wrong_guess()
{
    int m;
    int big_r, big_c;
    int small_square;
    guess temp;
    if (backups.empty())
    {
        stop_solving = true;
        return;
    }
    //loads the backup back into the main board
    *this = backups[backups.size() - 1];
    //loads the last guess into more weildly values
    
    temp.r = guesses[guesses.size() - 1].r;
    temp.c = guesses[guesses.size() - 1].c;
    temp.a = guesses[guesses.size() - 1].a;
    temp.guess_type = guesses[guesses.size() - 1].guess_type;
    guesses.erase(guesses.begin() + guesses.size() - 1);
    if (temp.guess_type == "one_possibility")
    {
        for (m = temp.a; m < 9; m++)
            if (possible[temp.r][temp.c][m] != 0)
            {
                temp.a = m + 1;
                guesses.push_back(temp);
                new_number((int)temp.r / 3, (int)temp.c / 3, temp.r % 3, temp.c % 3, temp.a);
                return;
            }
    }
    else if (temp.guess_type == "row")
    {
        for (m = temp.c + 1; m < 9; m++)
            if (possible[temp.r][m][temp.a - 1] == temp.a && board[temp.r][m] == 0)
            {
                temp.c = m;
                guesses.push_back(temp);
                new_number((int)temp.r / 3, (int)temp.c / 3, temp.r % 3, temp.c % 3, temp.a);
                return;
            }
    }
    else if (temp.guess_type == "column")
    {
        for (m = temp.r + 1; m < 9; m++)
            if (possible[m][temp.c][temp.a - 1] == temp.a && board[m][temp.c] == 0)
            {
                temp.r = m;
                guesses.push_back(temp);
                new_number((int)temp.r / 3, (int)temp.c / 3, temp.r % 3, temp.c % 3, temp.a);
                return;
            }
    }
    else
    {
        big_r = (int)temp.r / 3;
        big_c = (int)temp.c / 3;
        small_square = (temp.r % 3) * 3 + temp.c % 3;
        for (m = small_square + 1; m < 9; m++)
            if (possible[big_r * 3 + (int)m / 3][big_c * 3 + m % 3][temp.a - 1] == temp.a && board[big_r * 3 + (int)m / 3][big_c * 3 + m % 3] == 0)
            {
                temp.r = big_r * 3 + (int)m / 3;
                temp.c = big_c * 3 + m % 3;
                guesses.push_back(temp);
                new_number((int)temp.r / 3, (int)temp.c / 3, temp.r % 3, temp.c % 3, temp.a);
                return;
            }
    }
    backups.erase(backups.begin() + backups.size() - 1);
    if (backups.size() != 0)
        wrong_guess();
    else
        stop_solving = true;
    return;
}

void create_board()
{
    //if desire to change diffuculty, change the variable diffuculty
    //  lower value results in an easier puzzle
    int diffuculty = 5;
    Sudoku_Board end, last, save;
    int i, save_rand;
    bool cur_hide_board = hide_board;
    bool cur_slowly = slowly;
    bool cur_check_for_multi_ans = check_for_multi_ans;
    hide_board = true;
    slowly = false;
    check_for_multi_ans = true;
    bool redo = true;
    cout << "This may take some time.";
    while (redo)
    {
        backups.clear();
        guesses.clear();
        for (i = 0; i < 9; i++)
            Board.inputs[i] = "000000000";
        //creates a random board that is accurate
        //generates the seed
        for (i = 1; i <= 14; i++)
        {
            save_rand = (int)rand() * 9 - 1;
            Board.inputs[save_rand] = Board.inputs[save_rand][(int)rand() * 9 - 1] = (int)rand() * 9 + 48;
        }
        Board.initialize();
        Board.solve();
        if (!Board.unsolvable)
            redo = false;
        if (Board.validate_board() != "")
            redo = true;
    }
    end = Board;
    i = 0;
    while (i < diffuculty)
    {
        //last = Board;
        backups.clear();
        guesses.clear();
        last.initialize(Board);
        //Board.initialize(Board);
        Board.board[(int)rand() * 9 - 1][(int)rand() * 9 - 1] = 0;
        Board.initialize(Board);
        save = Board;
        Board.solve();
        if (Board.unsolvable || Board.multiple_ans)
        {
            i++;
            Board = last;
        }
        else
        {
            i = 0;
            Board = save;
        }
    }
    //the output section
    system("CLS");
    last.output();
    string commas = "";
    for (i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                for (int l = 0; l < 3; l++)
                    commas += to_string(static_cast<long long>(last.board[i * 3 + j][k * 3 + l]));
    cout << "Commas: " << removezeros(addcommas(commas)) << endl;
checkanswer:
    cout << "View answer? (Y/N): ";
    char answer;
    cin >> answer;
    answer = tolower(answer);
    if (answer == 'n')
        goto skipanswer;
    else if (answer == 'y')
        goto viewanswer;
    else
        goto checkanswer;
viewanswer:
    cout << endl << endl;
    end.output();
    commas = "";
    for (i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                for (int l = 0; l < 3; l++)
                    commas += to_string(static_cast<long long>(end.board[i * 3 + j][k * 3 + l]));
    cout << "Commas: " << addcommas(commas) << endl;
    system("pause");
skipanswer:
    hide_board = cur_hide_board;
    slowly = cur_slowly;
    check_for_multi_ans = cur_check_for_multi_ans;
}

string validate_row(string input)
{
    int i;
    if (input.length() < 9)
        return "Too short.";
    else if (input.length() > 9)
        return "Too long.";
    else
    {
        for (i = 0; i < 9; i++)
            if ((int)input[i] < 48 || (int)input[i] > 57)
                return "Numbers only.";
    }
    return "";
}

void Sudoku_Board::initialize()
{
    int i, j, k, l;
    for (i = 0; i < 9; i++)
        for (j = 0; j < 9; j++)
            board[i][j] = (int)inputs[i][j] - 48;
    //initializes smallboards
    for (i = 0; i < 3; i++)
        //large columns
        for (j = 0; j < 3; j++)
            //small rows
            for (k = 0; k < 3; k++)
                //small columns
                for (l = 0; l < 3; l++)
                    smallboards[i * 3 + j][k][l] = board[i * 3 + k][j * 3 + l];
    for (i = 0; i < 9; i++)
        for (j = 0; j < 9; j++)
            for (k = 0; k < 9; k++)
                possible[i][j][k] = k + 1;
    return;
}

void Sudoku_Board::initialize(Sudoku_Board copy)
{
    int i, j;
    for (i = 0; i < 9; i++)
    {
        inputs[i] = "";
        for (j = 0; j < 9; j++)
            inputs[i] += copy.board[i][j] + 48;
    }
    initialize();
}

void Sudoku_Board::output()
{
    int i, j, k, l;
    cout << "-------------------------" << endl;
    //groups of 3's of rows
    for (i = 0; i < 3; i++)
    {
        //single rows in those groups
        for (j = 0; j < 3; j++)
        {
            //for groups of 3 columns
            cout << "| ";
            for (k = 0; k < 3; k++)
            {
                //for single columns in those groups of 3
                for (l = 0; l < 3; l++)
                {
                    if (board[3 * i + j][3 * k + l] == 0)
                        cout << ". ";
                    else
                        cout << board[3 * i + j][3 * k + l] << " ";
                }
                if (k != 2)
                    cout << "| ";
                else
                    cout << "|" << endl;
            }
        }
        if (i != 2)
            cout << "|-------+-------+-------|" << endl;
    }
    cout << "-------------------------" << endl;
    cout << endl << endl;
    return;
}

bool Sudoku_Board::done()
{
    int i, j;
    for (i = 0; i < 9; i++)
        for (j = 0; j < 9; j++)
            if (board[i][j] == 0)
                return false;
    return true;
}

string Sudoku_Board::validate_board()
{
    int i, j, k;
    int temp[10];
    //checks so 2 same values can't be in the same row
    for (i = 0; i < 9; i++)
    {
        std::fill(temp, temp + 10, 0);
        for (j = 0; j < 9; j++)
            temp[board[i][j]]++;
        for (j = 1; j <= 9; j++)
            if (temp[j] > 1)
                return "Numbers in the same row.";
    }
    //checks so 2 same values can't be in the same column
    for (i = 0; i < 9; i++)
    {
        std::fill(temp, temp + 10, 0);
        for (j = 0; j < 9; j++)
            temp[board[j][i]]++;
        for (j = 1; j <= 9; j++)
            if (temp[j] > 1)
                return "Numbers in the same column.";
    }
    //checks so 2 same values can't be in the same square
    for (i = 0; i < 9; i++)
    {
        std::fill(temp, temp + 10, 0);
        for (j = 0; j < 3; j++)
            for (k = 0; k < 3; k++)
                temp[smallboards[i][j][k]]++;
        for (j = 1; j <= 9; j++)
            if (temp[j] > 1)
                return "Numbers in the same box.";
    }
    return "";
}

void instructions() {
    cout << "Enter the board as you see it, with zeros for spaces." << endl
    << "Then wait until the program solves the board." << endl << endl;
    system("pause");
    return;
}

void settings()
{
    string input;
    do {
        cout << "Sudoku Solver Settings" << endl
        << "Enter a number to toggle a setting. The current selection is shown in []." << endl
        << endl
        << "    1. Solve board extra slowly.                               [";
        if (slowly)
            cout << "On ]" << endl;
        else
            cout << "Off]" << endl;
        cout << "    2. Solve board extra fast.                                 [";
        if (hide_board)
            cout << "On ]" << endl;
        else
            cout << "Off]" << endl;
        cout << "    3. Play notification sound when finished.                  [";
        if (sound_when_finished)
            cout << "On ]" << endl;
        else
            cout << "Off]" << endl;
        cout << "    4. Check for multiple answers. (Fast solve recommended)    [";
        if (check_for_multi_ans)
            cout << "On ]" << endl;
        else
            cout << "Off]" << endl;
        cout << "    5. Back" << endl
        << endl
        << "What would you like to do? : ";
        cin >> input;
        if (input == "1")
        {
            slowly = !slowly;
            if (slowly)
                hide_board = false;
            system("CLS");
        }
        else if (input == "2")
        {
            hide_board = !hide_board;
            if (hide_board)
                slowly = false;
            system("CLS");
        }
        else if (input == "3")
        {
            sound_when_finished = !sound_when_finished;
            system("CLS");
        }
        else if (input == "4")
        {
            check_for_multi_ans = !check_for_multi_ans;
            system("CLS");
        }
        if (input != "1" && input != "2" && input != "3" && input != "4" && input != "5")
        {
            system("CLS");
        }
    } while (input != "5");
    return;
}
void commaHandling(string& puzzlecommas)
{
    if (puzzlecommas[0] == ',')
        puzzlecommas = '0' + puzzlecommas;
    if (puzzlecommas[puzzlecommas.length() - 1] == ',')
        puzzlecommas[puzzlecommas.length() - 1] = '0';
    //cout << puzzlecommas << endl;
    //system("pause");
    int start = 0;
    while (puzzlecommas.find(",") != string::npos)
    {
        start = puzzlecommas.find_first_of(",");
        if (start < puzzlecommas.length() - 1 && puzzlecommas[start + 1] == ',')
        {
            puzzlecommas[start] = '0';
        }
        else
        {
            puzzlecommas = puzzlecommas.substr(0, start) + puzzlecommas.substr(start + 1, puzzlecommas.length() - start - 1);
        }
    }
    return;
}
string addcommas(string str)
{
    string str2 = "";
    str2 += str[0];
    for (int i = 1; i < str.length(); i++)
    {
        str2 += ",";
        str2 += str[i];
    }
    return str2;
}
string removezeros(string str)
{
    for (int i = 0; i < str.length() - 1; i++)
        if (str[i] == '0')
            str = str.substr(0, i) + str.substr(i + 1, str.length() - i - 1);
    if (str[str.length() - 1] == '0')
        str[str.length() - 1] = ',';
    return str;
}
/*
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 -------+-------+-------
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 -------+-------+-------
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 1 2 3 | 4 5 6 | 7 8 9
 */
