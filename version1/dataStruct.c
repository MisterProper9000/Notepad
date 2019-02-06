#include <windows.h>
#include <stdio.h>
#include <String.h>
#include <math.h>

#include "dataStruct.h"
#include "menu.h"


void NewFileRead(HWND hwnd, textParams * tp, BOOLEAN * err) {
    OPENFILENAME ofn;
    char filename[MAX_PATH];

    ZeroMemory(&ofn,sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All files\0*.*\0Source Files\0*.C\0Text Files\0*TXT\0";
    ofn.nFilterIndex = 1;

    GetOpenFileName(&ofn);

    FileRead(ofn.lpstrFile, tp, err);
    if(*err) *err = FALSE; //costûl :(
}

void FileRead(char * filename, textParams * tp, BOOLEAN * err) {
	int size, cStr = 0, i = 0, position = 0, maxLength = 0, lineLength = 0;
	FILE * f = fopen(filename, "r");
	if (f == NULL) {
        *err = TRUE;
        return;
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	free(tp->String);
	tp->String = calloc(size, sizeof(char));
	if(tp->String == NULL) {
        *err = TRUE;
        return;
	}
	fseek(f, 0, SEEK_SET);
	while (fgets(tp->String, size+1, f) != NULL)
        cStr++;
	free(tp->startLineIdx);
	tp->startLineIdx = calloc(cStr + 1, sizeof(int));
	if(tp->startLineIdx == NULL) {
        *err = TRUE;
        return;
	}
	fseek(f, 0, SEEK_SET);
	tp->startLineIdx[i++] = 0;
	while (fgets(tp->String, size + 1, f) != NULL)
	{
	    lineLength = strlen(tp->String);
	    if(lineLength > maxLength)
            maxLength = lineLength;
		position += lineLength;
		tp->startLineIdx[i++] = position;
	}
	fseek(f, 0, SEEK_SET);
	fread(tp->String, size, sizeof(char), f);
	tp->numLines = cStr;
	tp->maxLineLength = maxLength;
	tp->curLine  = 0;
	fclose(f);
}


void BackToNormalMetrics(textParams * tp, wndParams * wp, BOOLEAN * err) {
    int i = 0;

    wp->curY = tp->curLine;
    wp->curX = 0;
    wp->numLines = tp->numLines;
    wp->maxLineLength = tp->maxLineLength;
    free(wp->startLineIdx);
    wp->startLineIdx = NULL;
    wp->startLineIdx = calloc(tp->numLines+1,sizeof(int));
    if(wp->startLineIdx == NULL)
    {
        *err = TRUE;
        return;
    }


    for(i = 0;i <= tp->numLines;i++) {
        wp->startLineIdx[i] = tp->startLineIdx[i];
    }

}

void ReCalculate(int width , textParams * tp, wndParams * wp, BOOLEAN * err) {
    int j = 0, spacePos = 0, prevLine = 0, curPosition = 0, cStr = 0;
    wp->curX = 0;
    for(curPosition = 0; curPosition < tp->startLineIdx[tp->numLines]; curPosition++) {
        if(tp->String[curPosition] == ' ') {
            spacePos = curPosition;
        }
        else if(tp->String[curPosition] == '\n') {
            prevLine = curPosition;
            spacePos = curPosition;
            cStr++;
        }
        if(curPosition == prevLine + width){
            if(spacePos != prevLine) {
                prevLine = spacePos;
                curPosition = spacePos;
            }
            else {
                prevLine = curPosition;
                spacePos = curPosition;
            }
            cStr++;
        }
    }
    cStr++;
    wp->numLines = cStr;
    wp->maxLineLength = width - 1;
    free(wp->startLineIdx);
    wp->startLineIdx = calloc(cStr + 1, sizeof(int));
    if(wp->startLineIdx == NULL){
        *err = TRUE;
        return;
    }
    wp->startLineIdx[0] = 0;
    curPosition = 0;
    spacePos = 0;
    prevLine = 0;
    j++;
    while (curPosition != tp->startLineIdx[tp->numLines]){
        if (tp->String[curPosition] == ' '){
            spacePos = curPosition;
        }
        else if (tp->String[curPosition] == '\n'){
            prevLine = curPosition;
            spacePos = curPosition;
            wp->startLineIdx[j] = curPosition;
            if(tp->startLineIdx[tp->curLine] - 1 == wp->startLineIdx[j])
                wp->curY = j;
            j++;
        }
        if(curPosition == prevLine + width){
            if(spacePos != prevLine) {
                prevLine = spacePos;
                curPosition = spacePos;
                wp->startLineIdx[j] = spacePos + 1;
                if(tp->startLineIdx[tp->curLine] - 1 == wp->startLineIdx[j])
                    wp->curY = j;
                j++;
            }
            else {
                prevLine = curPosition;
                spacePos = curPosition;
                wp->startLineIdx[j] = curPosition;
                if(tp->startLineIdx[tp->curLine] - 1 == wp->startLineIdx[j])
                    wp->curY = j;
                j++;
            }
            cStr++;
        }
        curPosition++;
    }
    wp->startLineIdx[j] = curPosition;


}

void ChangeCurrentLine(textParams * tp, wndParams * wp, int direction) {
    int i = 0;
    if(direction == SCROLL_UP) {
        while(tp->startLineIdx[tp->curLine + i] - 1 <= wp->startLineIdx[wp->curY])
            i++;
        tp->curLine += i - 1;
    }
    else if(direction == SCROLL_DOWN) {
        while(tp->startLineIdx[tp->curLine + i] - 1 > wp->startLineIdx[wp->curY])
            i++;
        tp->curLine -= i;
    }
}

void SetScrollbars(HWND hwnd, SCROLLINFO * si, int cxClient, int cyClient, wndParams * wp, textParams * tp) {
    si->cbSize = sizeof(si);
    si->fMask = SIF_RANGE | SIF_PAGE;
    si->nMin = 0;
    si->nMax = wp->numLines;
    si->nPage = cyClient / wp->cySymbol;
    SetScrollInfo(hwnd, SB_VERT, si, TRUE);
    wp->numWndLines = cyClient / wp->cySymbol + 1;
    SetScrollPos(hwnd, SB_VERT, wp->curY, TRUE);
    si->nMax--; //for correct scrollbar move down
    if(si->nMax > si->nPage) {
        if (wp->curY + si->nPage > si->nMax && si->nMax > 0){
            wp->curY -= wp->curY + si->nPage - si->nMax-1;
            if (wp->mode == NORMAL_MODE)
                tp->curLine = wp->curY;
            if (wp->mode == WRAP_MODE)
                ChangeCurrentLine(tp, wp, SCROLL_UP);
        }
    }

    si->cbSize = sizeof(si);
    si->fMask = SIF_RANGE | SIF_PAGE;
    si->nMin = 0;
    si->nMax = wp->maxLineLength;
    si->nPage = cxClient / wp->cxSymbol;
    SetScrollInfo(hwnd,SB_HORZ,si,TRUE);
    wp->numWndSymbols = cxClient / wp->cxSymbol + 1;
    SetScrollPos(hwnd, SB_HORZ, wp->curX, TRUE);
}
