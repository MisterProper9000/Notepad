#pragma once

#include <Windows.h>
#include <stdio.h>
#define maxHeight 1080 // max window Height
#define maxWidth 1920  // max window Width


typedef struct winParam {
    HDC         hMemDC;         // HDC 1, 2 -for second buffering
    PAINTSTRUCT ps;             // information about application
    RECT        rect;           // window rect
    int         modeChecked;    // checked mod in menu
}winParam;

typedef struct caret_type
{
    int xPos;               // offset of caret from beginning of the line
    int yPos;               // number of the current line for caret
    //int nextSymbol;       //caret's next symbol in String
}caret_t;

typedef struct param
{
    char *String;                      // buffer of income text
    int  stringSize;                   // size of buffer
    int  letterWidth, letterHeight;    // width and height of symbols
    int  wrap;                         // need word wrap or not
    int  textOffset;                   // horizontal offset

    long *pDefaultStartLineIdx;        // array of lines offsets in original text
    long *pWrapStartLineIdx;           // array of lines offsets after wrap
    long defaultLinesSize;             // pDefaultStartLineIdx array size
    long wrapLinesSize;                // pWrapStartLineIdx array size
    long workLinesBufferSize;          // additional buffer size for pWrapStartLineIdx
    long workLine;                     // number of first line on screen in pWrapStartLineIdx
    long maxLineLen;                   // maximum line length


}textParams;



void InitText(textParams *Text, caret_t* caret, FILE *pFile);
void FreeText(textParams *Text);



void ResizeText(textParams *Text, caret_t* caret, int newWindowWidth);

void SetCaretOnFirstVisibleLine(textParams *Text, caret_t* caret);
void MoveCaretUp(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd);
void MoveCaretDown(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd);
void MoveCaretLeft(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd);
void MoveCaretRight(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd);
void UpdateCaretPos(textParams *Text, caret_t* caret);
void MoveCaretIntoView(textParams *Text, caret_t* caret, winParam *windowInfo, HWND hwnd);
