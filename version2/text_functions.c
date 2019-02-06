#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#include "text_functions.h"


void ResizeText(textParams *Text, caret_t* caret, int count);

/*
      IN:
        pFile - file
      OUT:
        Text - textParams structure
        1 or 0

      loads line from buffer. returns 1 for success and 0 for failure */

int loadText(textParams *Text, FILE* pFile) {

    if (pFile != NULL) {
        long fileSize;
        fseek(pFile, 0, SEEK_END);
        fileSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);


        Text->stringSize = fileSize;
        Text->String = (char *)calloc(Text->stringSize, sizeof(char));
        fread(Text->String, sizeof(char), Text->stringSize, pFile);
        return 1;
    }
    return 0;
}


/*
      OUT: Text - textParams structure

      moves caret to the beginning of the window */

void SetCaretOnFirstVisibleLine(textParams *Text, caret_t* caret)
{
    caret->xPos = 0;
    Text->textOffset = 0;
    caret->yPos = Text->workLine;
    SetCaretPos((caret->xPos + Text->textOffset) * Text->letterWidth, (caret->yPos - Text->workLine) * Text->letterHeight);
}


/*    IN:
        pFile - pointer to the file
      OUT:
        Text - textParams structure

      initiates textParams */

void InitText(textParams *Text, caret_t* caret, FILE *pFile)
{

    int i, j, offset, lineLen;

    Text->letterHeight = 16;
    Text->letterWidth = 10;
    Text->workLine = 0;
    Text->maxLineLen = 0;
    Text->textOffset = 0;
    caret->yPos = 0;
    caret->xPos = 0;

    // load file
    if (loadText(Text, pFile) == 0) {
        Text->String = NULL;

        return;
    }

    // calculate linesSize
    Text->defaultLinesSize = 0;
    for (i = 0; i < Text->stringSize; ++i)
    {
        if (Text->String[i] == '\n' || Text->String[i] == '\0')
            ++Text->defaultLinesSize;
        if (Text->String[i] == '\t')
            Text->String[i] = ' ';
        if (Text->String[i] == '\0')
            break;
    }

    Text->stringSize = i;
    Text->workLinesBufferSize = Text->wrapLinesSize = Text->defaultLinesSize;
    Text->pDefaultStartLineIdx = (long *)malloc((Text->defaultLinesSize + 1) * sizeof(long));
    Text->pWrapStartLineIdx = (long *)malloc((Text->wrapLinesSize + 1) * sizeof(long));

    // init Defoult lines
    lineLen = 0;
    offset = 0;
    Text->pWrapStartLineIdx[0] = Text->pDefaultStartLineIdx[0] = 0;
    for (i = 0, j = 1; i < Text->stringSize; ++i)
    {
        if (offset > 0)
         Text->String[i] = Text->String[i + offset];

        lineLen++;

        if (Text->String[i] == '\n' || Text->String[i] == '\0')
        {
            if (lineLen > Text->maxLineLen)
            Text->maxLineLen = lineLen;
            lineLen = 0;
            Text->pWrapStartLineIdx[j] = Text->pDefaultStartLineIdx[j] = i;
            j++;
        }

        if (Text->String[i] == '\r' || Text->String[i] == '\n') {
            offset++;
            Text->String[i] = Text->String[i + offset];
            i--;
            Text->stringSize--;
        }
    }

    Text->pWrapStartLineIdx[Text->wrapLinesSize] = Text->stringSize;
    Text->pDefaultStartLineIdx[Text->defaultLinesSize] = Text->stringSize;

    Text->wrap = 1;

    ResizeText(Text, caret, Text->maxLineLen * Text->letterWidth);

    Text->wrap = 0;

    SetCaretOnFirstVisibleLine(Text, caret);

}



/*    IN/OUT:
        Text - textParams structure
      IN:
        newWindowWidth - window Width in characters
      OUT: -

      resize text by new width */

void ResizeText(textParams *Text, caret_t* caret, int newWindowWidth)
{
    int count, i, j, caretBufferPos, gotCaret = 1, isCaretOnEnd = 0;
    if (Text->String == NULL || Text->wrap == 0)//if buffer is empty or wrap doesn't need, then nothing to do here
        return;

    int currentBufferPos = Text->pWrapStartLineIdx[Text->workLine];//save idx of first line on screen
    Text->wrapLinesSize = 0;

    if (caret->xPos == (Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos]) && caret->xPos != 0)
        isCaretOnEnd = 1;//check if caret at the end of text (for 234 line, kind of kostыl')

    count = newWindowWidth / Text->letterWidth; //width of screen in symbols

    caretBufferPos = Text->pWrapStartLineIdx[caret->yPos] + caret->xPos; //calculating caret's next symbol

    for (i = 0; i < Text->defaultLinesSize; ++i) // count work lines after resize
    {
        Text->wrapLinesSize += ceil((double)(Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i]) / count);
        if (Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i] == 0)
            Text->wrapLinesSize++;
    }
    if (Text->workLinesBufferSize < Text->wrapLinesSize) //realloc if we get more lines after resize
    {
        Text->workLinesBufferSize = Text->wrapLinesSize;
        Text->pWrapStartLineIdx = (long *)realloc(Text->pWrapStartLineIdx, (Text->wrapLinesSize + 2) * sizeof(long));
    }
    for (i = 0, j = 0; i < Text->defaultLinesSize; ++i) //j - work i - default
    {
        int currWorkLineLenUsed = 0, k;

        if (Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i] == 0)
        {
            Text->pWrapStartLineIdx[j] = Text->pDefaultStartLineIdx[i];
            j++;
        }

        for (k = 0; (Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i]) != currWorkLineLenUsed && j < Text->wrapLinesSize; ++k, ++j) // make work lines from default lines
        {
            int lineLen;
            Text->pWrapStartLineIdx[j] = Text->pDefaultStartLineIdx[i] + currWorkLineLenUsed;//set new start line index
            if ((Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i]) - currWorkLineLenUsed < count)//if i'th default line length without CWLLU less then max count of chars in line
                lineLen = Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i] - currWorkLineLenUsed;//then no need in wrap
            else
                lineLen = count;//else wrap happens

            if (Text->pWrapStartLineIdx[j] < currentBufferPos && Text->pWrapStartLineIdx[j] + lineLen > currentBufferPos)
            {
                lineLen = currentBufferPos - Text->pWrapStartLineIdx[j];
                //printf("lineLen %i\n", lineLen);
            }
            if (Text->pWrapStartLineIdx[j] == currentBufferPos)//save first line
                Text->workLine = j;

            //printf("currWorkLineLenUsed before += %i\n", lineLen);
            currWorkLineLenUsed += lineLen;

            //check for caret pos in text
            if (j > 0 && Text->pWrapStartLineIdx[j - 1] < caretBufferPos && Text->pWrapStartLineIdx[j] > caretBufferPos && gotCaret)
            {
                caret->yPos = j - 1;
                caret->xPos = caretBufferPos - Text->pWrapStartLineIdx[j - 1];
                gotCaret = 0;
            }

            if (j > 0 && Text->pWrapStartLineIdx[j] == caretBufferPos && gotCaret)//if caret in "between" lines, set new caret pos
            {
                if (isCaretOnEnd) //if caret before '\0' set special pos
                {
                    caret->yPos = j - 1;
                    caret->xPos = caretBufferPos - Text->pWrapStartLineIdx[j - 1];
                }
                else
                {
                    caret->yPos = j;
                    caret->xPos = 0;
                }
            }
        }
        if ((int)ceil((double)(Text->pDefaultStartLineIdx[i + 1] - Text->pDefaultStartLineIdx[i]) / count) != k)
            Text->wrapLinesSize++;
    }

    if (Text->pWrapStartLineIdx[j - 1] <= caretBufferPos && Text->stringSize >= caretBufferPos && gotCaret)
    {
        caret->yPos = j - 1;
        caret->xPos = caretBufferPos - Text->pWrapStartLineIdx[j - 1];
        gotCaret = 0;//caret already processed and gotCaret won't be equal to 1 never again
    }
    Text->pWrapStartLineIdx[Text->wrapLinesSize] = Text->stringSize;
}


/*    IN/OUT:
        Text - textParams structure
      IN:
        windowInfo - winParam structure
        hwnd - window descriptor
      returns: -

      shift screen to the caret if it was moved with scrolls */

void MoveCaretIntoView(textParams *Text, caret_t* caret, winParam *windowInfo, HWND hwnd)
{
    //vertical
    if (caret->yPos < Text->workLine)
        Text->workLine = caret->yPos;
    if (caret->yPos > Text->workLine + windowInfo->rect.bottom / Text->letterHeight - 1)
        Text->workLine = caret->yPos;

    //horizontal
    if ((-Text->textOffset) - caret->xPos > 0)
        Text->textOffset = -caret->xPos;
    if (caret->xPos + Text->textOffset > windowInfo->rect.right / Text->letterWidth)
        Text->textOffset = max(-caret->xPos, -(Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos] - windowInfo->rect.right / Text->letterWidth));

}

/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - winParam structure
        hwnd - window descriptor
      OUT: -

    moves caret UP or shifts it if there is no symbols in destination line */

void MoveCaretUp(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd)
{
    caret->yPos--;

    if (caret->yPos >= 0)
    {
        if (caret->yPos < Text->workLine)
            SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
        if (Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos] < caret->xPos)
            caret->xPos = Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos];
    }
    else
    {
        caret->yPos++;
    }
    MoveCaretIntoView(Text, caret, windowInfo, hwnd);
    SetCaretPos((caret->xPos + Text->textOffset) * Text->letterWidth, (caret->yPos - Text->workLine) * Text->letterHeight);
}

/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - winParam structure
        hwnd - window descriptor
      OUT: -

      moves caret DOWN or shifts it if there is no symbols in destination line */

void MoveCaretDown(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd)
{
    caret->yPos++;

    if (caret->yPos < Text->wrapLinesSize)
    {
        if (caret->yPos >= Text->workLine + windowInfo->rect.bottom / Text->letterHeight)
            SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
        if (Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos] < caret->xPos)
            caret->xPos = Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos];
    }
    else
    {
        caret->yPos--;
    }

    MoveCaretIntoView(Text, caret, windowInfo, hwnd);
    SetCaretPos((caret->xPos + Text->textOffset) * Text->letterWidth, (caret->yPos - Text->workLine) * Text->letterHeight);
}

/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - winParam structure
        hwnd - window descriptor
      OUT: -

      moves caret LEFT */

void MoveCaretLeft(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd)
{
    caret->xPos--;

    if (caret->xPos < 0)
    {
        caret->xPos++;
    }
    else if (caret->xPos + Text->textOffset < 0)
    {
        SendMessage(hwnd, WM_HSCROLL, SB_LINELEFT, 0L);
    }
    MoveCaretIntoView(Text, caret, windowInfo, hwnd);
    SetCaretPos((caret->xPos + Text->textOffset) * Text->letterWidth, (caret->yPos - Text->workLine) * Text->letterHeight);
}



/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - winParam structure
        hwnd - window descriptor
      OUT: -

      moves caret RIGHT */

void MoveCaretRight(winParam *windowInfo, caret_t* caret, textParams *Text, HWND hwnd)
{
    caret->xPos++;

    if (caret->xPos > (Text->pWrapStartLineIdx[caret->yPos + 1] - Text->pWrapStartLineIdx[caret->yPos]))
    {
        caret->xPos--;
    }
    else if (caret->xPos + Text->textOffset > windowInfo->rect.right / Text->letterWidth)
    {
        SendMessage(hwnd, WM_HSCROLL, SB_LINERIGHT, 0L);
    }

    MoveCaretIntoView(Text, caret, windowInfo, hwnd);
    SetCaretPos((caret->xPos + Text->textOffset) * Text->letterWidth, (caret->yPos - Text->workLine) * Text->letterHeight);
}

