#ifndef DATASTRUCT_H
#define DATASTRUCT_H


typedef struct param {
    char * String;        // text buffer
    int maxLineLength;  // max length of lines
    int numLines;       // number of lines in our text
    int * startLineIdx;    // buffer with start indexes of start lines
    int curLine;        // current line on top of the screen
}textParams;

typedef struct wndParam {
    int * startLineIdx;    // buffer with indexes of line's beginning
    int curX;           // shift by X in symbols
    int curY;           // shift by Y in lines
    int mode;           // current text mode
    int cxSymbol;       // width of the symbol
    int cySymbol;       // height of the symbol
    int numLines;       // number of lines in String
    int maxLineLength;  // max length of lines
    int numWndLines;    // max number of lines in window
    int numWndSymbols;  // max number of symbols in window
}wndParams;

/** BackToNormalMetrics
* wndParams[IN/OUT]
* textParams[IN]
* err[IN/OUT]
* changes in wndParams curX, curY, numLines, maxLineLength, startLineIdx
* takes parameters from textParams and put it to wndParams to set normal metrics
* in case of correct operation doesn't change err value (TRUE by default), otherwise set TRUE to err
*/
void BackToNormalMetrics(textParams * tp, wndParams * wp, BOOLEAN * err);

/** ReCalculate
* wndParams[IN/OUT]
* textParams[IN]
* width[IN]
* err[IN/OUT]
* changes in wndParams curX, curY, numLines, maxLineLength, startLineIdx
* recalculate view metrics when wrap mode is on
* in case of correct operation doesn't change err value (TRUE by default), otherwise set TRUE to err
*/
void ReCalculate(int width , textParams * tp, wndParams * wp, BOOLEAN * err);

/** FileRead
* textParams[IN/OUT]
* filename[IN]
* err[IN/OUT]
* tries to open file, if file has opened sets textParams
* change in textParams String, numLines, strStarts, numLines and maxLineLength
* in case of correct operation doesn't change err value (TRUE by default), otherwise set TRUE to err
*/
void FileRead(char * filename, textParams * tp, BOOLEAN * err);

/** NewFileRead
* hwnd[IN]
* textParams[IN]
* err[IN/OUT]
* gets new name of file and tries to open it
* in case of correct operation doesn't change err value (TRUE by default), otherwise set TRUE to err
*/
void NewFileRead(HWND hwnd, textParams * tp, BOOLEAN * err);

/** SetScrollbars
* hwnd[IN]
* si[IN/OUT]
* cxClient[IN]
* cyClient[IN]
* wndParams[IN/OUT]
* textParams[IN]
* changes SCROLLINFO parameters
* changes in wndParams curY, curX
* set scroll bars, when window metrics changed or change scroll bars position
*/
void SetScrollbars(HWND hwnd, SCROLLINFO * si, int cxClient, int cyClient, wndParams * wp, textParams * tp);

/** ChangeCurrentLine
* textParams[IN/OUT]
* wndParams[IN]
* scrollDirection[IN]
* base on scrollDirection change current line to the close line when wrap mode is on
*/
void ChangeCurrentLine(textParams * tp, wndParams * wp, int direction);

/** AddMenus
* hwnd[IN]
* add menu items: File(Open, Exit), Mode(Normal, Wrap)
*/
void AddMenus(HWND hwnd, HMENU myMenu, HMENU myMenuPopup);

#endif // DATASTRUCT_H
