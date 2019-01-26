/* NeoPixel Connect4 multiplayer, Stijn Slebos, Sanne Metten en Tijmen Smit */

//initialize neopixel variables
#include <Adafruit_NeoPixel.h> //include library

#define PIN           2    //arduino pin of neopixel strip
#define NUMPIXELS    42   //# of pixels

#define X_ROWS        7
#define Y_COLUMNS     6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //initialize strip
enum mode {
  startup = 0,
  singleplayer = 1,
  multiplayer = 2
};

//NEW CODE
int pixelGrid [X_ROWS][Y_COLUMNS] =
{
  {0, 13, 14, 27, 28, 41},
  {1, 12, 15, 26, 29, 40},
  {2, 11, 16, 25, 30, 39},
  {3, 10, 17, 24, 31, 38},
  {4, 9, 18, 23, 32, 37},
  {5, 8, 19, 22, 33, 36},
  {6, 7, 20, 21, 34, 35}
};
int colorGrid [X_ROWS][Y_COLUMNS] = {0}; //-1 for yellow, 1 for red, 0 for empty;
int turn = 1; //-1 for yellow, 1 for red;
mode gamemode = startup;
int maxDepth = 1;

/*initialize button variables*/
int inputPins[7] = {4, 5, 6, 7, 8, 9, 10};
int columns[7] = {0, 1, 2, 3, 4, 5, 6};
int rows[7] = {0, 1, 2, 3, 4, 5, 6};

void setup() {
  Serial.begin(9600);
  strip.begin(); //set up strip
  strip.setBrightness(100); //set brightness
  strip.show(); //sets all pixels to be off
}

void loop() {
  switch (gamemode) {
    case startup:
      _startup();
      break;
    case singleplayer:
      _singleplayer();
      break;
    case multiplayer:
      _multiplayer();
      break;
  }
}

void _startup() {
  for (int i = 0; i < 6; i++) {
    strip.setPixelColor(pixelGrid[0][i], 0, 0, 0); //empty
    strip.setPixelColor(pixelGrid[1][i], 10, ((millis() + 50 * (5 - i)) / 10) % 150, 30); //empty
    strip.setPixelColor(pixelGrid[2][i], 0, 0, 0); //empty
    strip.setPixelColor(pixelGrid[3][i], 0, 0, 0); //empty
    strip.setPixelColor(pixelGrid[4][i], 0, 0, 0); //empty
    strip.setPixelColor(pixelGrid[5][i], 80, ((millis() + 50 * (5 - i)) / 10) % 150, 0); //empty
    strip.setPixelColor(pixelGrid[6][i], 0, 0, 0); //empty
  }
  if (digitalRead(inputPins[1]) == HIGH) {
    gamemode = singleplayer;
    resetBoard();
    delay(300);
  }
  if (digitalRead(inputPins[5]) == HIGH) {
    gamemode = multiplayer;
    resetBoard();
    delay(300);
  }
  strip.show();
}

void _singleplayer() {
  updateColor();
  bool switched = false;
  if (turn == 1) {
    for (int i = 0; i < 7; i++) {
      if (digitalRead(inputPins[i]) == HIGH) {
        if (canMove(rows[i], colorGrid)) {
          addCoin(rows[i], turn);
          turn = -turn;
          switched = true;
        }
      }
    }
  } else {
    addCoin(rows[nextAIMove(colorGrid)], turn);
    turn = -turn;
    switched = true;
  }

  if (switched) {
    updateColor();
    delay(300);
    bool endGame = isWon(-turn, colorGrid);

    if (endGame) {
      Serial.println("end");
      gameIsWon(-turn);
      resetBoard();
      gamemode = startup;
    }
    if (isDraw(colorGrid)) {
      gameIsDraw();
      resetBoard();
      gamemode = startup;
    }
  }

}

void _multiplayer() {
  updateColor();
  bool switched = false;
  for (int i = 0; i < 7; i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      if (canMove(rows[i], colorGrid)) {
        addCoin(rows[i], turn);
        turn = -turn;
        switched = true;
      }
    }
  }

  if (switched) {
    updateColor();
    delay(300);
    bool endGame = isWon(-turn, colorGrid);

    if (endGame) {
      Serial.println("end");
      gameIsWon(-turn);
      resetBoard();
      gamemode = startup;
    }
    if (isDraw(colorGrid)) {
      gameIsDraw();
      resetBoard();
      gamemode = startup;
    }

  }

}

//add a coin on top of a coin (fill the column)
void addCoin(int row, int incomingColor) {
  for (int y  = 5; y >= 0; y--) {
    if (colorGrid[row][y] == 0) { //if the pixel is empty, fill it with the color
      colorGrid[row][y] = incomingColor;
      break;
    }
  }
}

bool canMove(int row, int grid[X_ROWS][Y_COLUMNS]) {
  for (int column = 0; column < Y_COLUMNS; column++) {
    if (grid[row][column] == 0) { //if the pixel is empty, fill it with the color
      return true;
      break;
    }
  }
  return false;
}

void updateColor() {
  float timer = millis() / 10;

  for (int row = 0; row < X_ROWS; row++) {
    for (int column = 0; column < Y_COLUMNS; column++) {
      if (colorGrid[row][column] == 0) {
        strip.setPixelColor(pixelGrid[row][column], 0, 40, 130 + 60 * sin(float(timer) / 150 * PI)); //empty
      }
      if (colorGrid[row][column] == 1) {
        strip.setPixelColor(pixelGrid[row][column], 255, 0, 0); //red
      }
      if (colorGrid[row][column] == -1) {
        strip.setPixelColor(pixelGrid[row][column], 255, 249, 0); //yellow
      }
    }
  }
  strip.show();
}

void resetBoard() {
  for (int row = 0; row < X_ROWS; row++) {
    for (int column = 0; column < Y_COLUMNS; column++) {
      colorGrid [row][column] = 0 ;
    }
  }
  turn = 1;
  updateColor();
}

void gameIsWon(int winner) {
  for (int row = 0; row < X_ROWS; row++) {
    for (int column = 0; column < Y_COLUMNS; column++) {
      if (winner == 1) {
        colorGrid[row][column] = winner;
      }
      if (winner == -1) {
        colorGrid[row][column] = winner;
      }
    }
  }
  updateColor();
  delay(2000);
}

void gameIsDraw() {
  for (int ties = 0; ties < 5; ties++) {
    for (int row = 0; row < X_ROWS; row++) {
      for (int column = 0; column < Y_COLUMNS; column++) {
        strip.setPixelColor(pixelGrid[row][column], 100, ties * 50, 250 - ties * 50); //empty
      }
    }
    strip.show();
    delay(400);
  }
}

bool isDraw(int _colorGrid[X_ROWS][Y_COLUMNS]) {
  for (int row = 0; row < X_ROWS; row++) {
    if (canMove(row, _colorGrid)) {
      return false;
    }
  }
  return true;
}

bool isWon(int Player, int _colorGrid[X_ROWS][Y_COLUMNS]) {
  //-----------------------
  //Vertical
  for (int row = 0; row < X_ROWS; row++) {
    for (int column  = 0; column < Y_COLUMNS - 3; column++) {
      if (_colorGrid[row][column] == Player &&
          _colorGrid[row][column + 1] == Player &&
          _colorGrid[row][column + 2] == Player &&
          _colorGrid[row][column + 3] == Player) {
        return true;
      }
    }
  }


  //Horizontal
  for (int column  = 0; column < Y_COLUMNS; column++) {
    for (int row = 0; row < X_ROWS - 3; row++) {
      if (_colorGrid[row][column] == Player &&
          _colorGrid[row + 1][column] == Player &&
          _colorGrid[row + 2][column] == Player &&
          _colorGrid[row + 3][column] == Player) {
        return true;
      }
    }
  }

  //Diagonal NE
  for (int column  = Y_COLUMNS - 1; column > 2; column--) { //checking for starting at column 5,4,3; so 6-1=5 and >2
    for (int row = 0; row < X_ROWS - 3; row++) {
      if (_colorGrid[row][column] == Player &&
          _colorGrid[row + 1][column - 1] == Player &&
          _colorGrid[row + 2][column - 2] == Player &&
          _colorGrid[row + 3][column - 3] == Player) {
        return true;
      }
    }
  }

  //Diagonal NW
  for (int column  = 0; column < Y_COLUMNS - 3; column++) {
    for (int row = 0; row < X_ROWS - 3; row++) {
      if (_colorGrid[row][column] == Player &&
          _colorGrid[row + 1][column + 1] == Player &&
          _colorGrid[row + 2][column + 2] == Player &&
          _colorGrid[row + 3][column + 3] == Player) {
        return true;
      }
    }
  }
  return false;

}

int nextAIMove(int _colorGrid[X_ROWS][Y_COLUMNS]) {
  int myNextMove = 6;
  int evaluation = -1000;

  int currentTurn = turn;

  for (int nextMove = 0; nextMove < X_ROWS; nextMove++) {
    int fakeGrid[X_ROWS][Y_COLUMNS] = {0};
    //first copy field
    for (int row = 0; row < X_ROWS; row++) for (int column = 0; column < Y_COLUMNS; column++) {
        fakeGrid[row][column] = _colorGrid[row][column]; //copy grid
      }

    if (canMove(nextMove, fakeGrid)) {
      //DOMOVE
      for (int y  = 5; y >= 0; y--) {
        if (fakeGrid[nextMove][y] == 0) {
          fakeGrid[nextMove][y] = currentTurn;
          break;
        }
      }
      //Evaluate
      if (isWon(currentTurn, fakeGrid)) {
        myNextMove = nextMove;
        break;
      }
      int score = AIDepthScore(fakeGrid, 0, currentTurn);
      if ( score > evaluation ||
           score == evaluation && int(random(0, 100)) <= 25) { // if they are identical 50% CHANGE
        evaluation = score;
        myNextMove = nextMove;
      }

    }
  }
  return myNextMove;
}

int AIDepthScore(int fakeGrid[X_ROWS][Y_COLUMNS], int depth, int currentTurn) {
  int nextTurn = -currentTurn;

  if (isWon(currentTurn, fakeGrid)) {
    return 30;
  } else if (isWon(nextTurn, fakeGrid)) {
    return -30;
  }

  if (depth > maxDepth) {
    return heuristicScore(fakeGrid, currentTurn);
  } else {
    int evaluation  = -1000;

    for (int nextMove = 0; nextMove < X_ROWS; nextMove++) {
      int _fakeGrid[X_ROWS][Y_COLUMNS];
      //first copy field
      for (int row = 0; row < X_ROWS; row++) for (int column = 0; column < Y_COLUMNS; column++) {
          _fakeGrid[row][column] = fakeGrid[row][column]; //copy grid
        }
      //----
      if (canMove(nextMove, _fakeGrid)) {
        //DOMOVE
        for (int y  = 5; y >= 0; y--) {
          if (_fakeGrid[nextMove][y] == 0) {
            _fakeGrid[nextMove][y] = nextTurn;
            break;
          }
        }
        //evaluate move
        int score = AIDepthScore(_fakeGrid, depth + 1, nextTurn);
        if ( score > evaluation) {
          evaluation = score;
        }
      }
    }
    return -evaluation;
  }

}

int heuristicScore(int fakeGrid[X_ROWS][Y_COLUMNS], int  currentTurn) {

  int nextTurn = -currentTurn;
  int evaluation = 0;

  //Vertical +3
  for (int row = 0; row < X_ROWS; row++) {
    for (int column  = 0; column < Y_COLUMNS - 3; column++) {
      if (fakeGrid[row][column] == 0 &&
          fakeGrid[row][column + 1] == fakeGrid[row][column + 2] &&
          fakeGrid[row][column + 2] == fakeGrid[row][column + 3]) {
        if (fakeGrid[row][column + 3] == currentTurn) {
          evaluation += 5;
        } else if (fakeGrid[row][column] == nextTurn) {
          evaluation -= 5;
        }
      }
    }
  }

  //Vertical +1
  for (int row = 0; row < X_ROWS; row++) {
    for (int column  = 0; column < Y_COLUMNS - 3; column++) {
      if (fakeGrid[row][column + 1] == 0 &&
          fakeGrid[row][column + 2] == fakeGrid[row][column + 3] ) {
        if (fakeGrid[row][column + 3] == currentTurn) {
          evaluation += 1;
        } else if (fakeGrid[row][column] == nextTurn) {
          evaluation -= 1;
        }
      }
    }
  }

  //Horizontal +3
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = 0; column < Y_COLUMNS; column++) {
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility % X_ROWS);
        if (fakeGrid[row][column] == fakeGrid[row + 1][column] &&
            fakeGrid[row + 1][column] == fakeGrid[row + 2][column] &&
            fakeGrid[row + 3][column] == 0) {
          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 5;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 5;
          }
        }
      }
    }
  }

  //Horizontal +1
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = 0; column < Y_COLUMNS; column++) {
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility % X_ROWS);
        if (
          (
            (fakeGrid[row][column] == fakeGrid[row + 1][column] &&
             fakeGrid[row + 2][column] == 0)

            ||

            (fakeGrid[row][column] == fakeGrid[row + 2][column] &&
             fakeGrid[row + 1][column] == 0)

          ) &&

          fakeGrid[row + 3][column] == 0

        ) {
          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 1;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 1;
          }
        }
      }
    }
  }

  //Diagonal NE +3
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = Y_COLUMNS - 1; column > 2; column--) { //checking for starting at column 5,4,3; so 6-1=5 and >2
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility) % X_ROWS;
        column = (column - possibility) % Y_COLUMNS;

        if (fakeGrid[row][column] == fakeGrid[row + 1][column - 1] &&
            fakeGrid[row + 1][column - 1] == fakeGrid[row + 2][column - 2] &&
            fakeGrid[row + 3][column - 3] == 0) {

          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 5;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 5;
          }
        }
      }
    }
  }

  //Diagonal NE +1
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = Y_COLUMNS - 1; column > 2; column--) { //checking for starting at column 5,4,3; so 6-1=5 and >2
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility) % X_ROWS;
        column = (column - possibility) % Y_COLUMNS;

        if ((
              (fakeGrid[row][column] == fakeGrid[row + 1][column - 1]
               && fakeGrid[row + 2][column - 2] == 0)

              ||

              (fakeGrid[row][column] == fakeGrid[row + 2][column - 2]
               && fakeGrid[row + 1][column - 1] == 0)

            ) &&

            fakeGrid[row + 3][column - 3] == 0) {

          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 1;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 1;
          }
        }
      }
    }
  }

  //    //Diagonal NW +3
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = 0; column < Y_COLUMNS - 3; column++) {
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility) % X_ROWS;
        column = (column + possibility) % Y_COLUMNS;

        if (fakeGrid[row][column] == fakeGrid[row + 1][column + 1] &&
            fakeGrid[row + 1][column + 1] == fakeGrid[row + 2][column + 2] &&
            fakeGrid[row + 3][column + 3] == 0) {

          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 5;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 5;
          }
        }
      }
    }
  }

  //Diagonal NW +1
  for (int possibility = 0; possibility < 4; possibility++) {
    for (int column  = 0; column < Y_COLUMNS - 3; column++) {
      for (int row = 0; row < X_ROWS - 3; row++) {
        row = (row + possibility) % X_ROWS;
        column = (column + possibility) % Y_COLUMNS;

        if ((
              (fakeGrid[row][column] == fakeGrid[row + 1][column + 1]
               && fakeGrid[row + 2][column + 2] == 0)

              ||

              (fakeGrid[row][column] == fakeGrid[row + 2][column + 2]
               && fakeGrid[row + 1][column + 1] == 0)

            ) &&

            fakeGrid[row + 3][column + 3] == 0) {

          if (fakeGrid[row][column] == currentTurn) {
            evaluation += 1;
          } else if (fakeGrid[row][column] == nextTurn) {
            evaluation -= 1;
          }
        }
      }
    }
  }


  Serial.println(evaluation);
  return evaluation;
}
