// main.c
// Win32 GUI front-end for railway booking system.

// IMPORTANT: Compile with all three C files: 
// gcc main.c data_structs.c file_io.c -o RailwayGUI.exe -mwindows

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "backend.h"

// =================================================================
// 0. CONSTANTS AND GLOBAL HANDLES
// =================================================================

// Main button IDs
#define ID_BTN_ADD      101
#define ID_BTN_DELETE   102
#define ID_BTN_SEARCH   103
#define ID_BTN_DISPLAY  104
#define ID_BTN_EXIT     105

// Data File Name
#define DATA_FILE_NAME "passengers.txt"

// Controls inside popups
// Add popup control IDs
#define ID_ADD_NAME     201
#define ID_ADD_AGE      202
#define ID_ADD_CAT      203
#define ID_ADD_OK       204
#define ID_ADD_CANCEL   205

// Delete popup control IDs
#define ID_DEL_PNR      301
#define ID_DEL_OK       302
#define ID_DEL_CANCEL   303

// Search popup control IDs
#define ID_SEARCH_PNR   401
#define ID_SEARCH_OK    402
#define ID_SEARCH_CANCEL 403

// Display popup control IDs
#define ID_DISPLAY_LIST 501

// Global Handles for Controls (used by popup procedures)
HWND hAddName, hAddAge, hAddCat;
HWND hDelPNR;
HWND hSearchPNR, hSearchResult;
HWND hDisplayList;

// =================================================================
// 1. WINDOW PROCEDURE PROTOTYPES
// =================================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AddWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DeleteWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisplayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// =================================================================
// 2. MAIN WINDOW PROCEDURE (MainWndProc)
// =================================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    const char *title;
    int x, y, width, height;

    switch (uMsg) {
        case WM_CREATE:
            // Create main buttons (The HMENU parameter is the control ID, cast from int)
            CreateWindowA("BUTTON", "Add Passenger", WS_VISIBLE | WS_CHILD, 10, 10, 160, 30, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
            CreateWindowA("BUTTON", "Delete Passenger", WS_VISIBLE | WS_CHILD, 190, 10, 160, 30, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            CreateWindowA("BUTTON", "Search Passenger", WS_VISIBLE | WS_CHILD, 10, 50, 160, 30, hwnd, (HMENU)ID_BTN_SEARCH, NULL, NULL);
            CreateWindowA("BUTTON", "Display All", WS_VISIBLE | WS_CHILD, 190, 50, 160, 30, hwnd, (HMENU)ID_BTN_DISPLAY, NULL, NULL);
            CreateWindowA("BUTTON", "Exit Program", WS_VISIBLE | WS_CHILD, 100, 150, 160, 30, hwnd, (HMENU)ID_BTN_EXIT, NULL, NULL);
            break;

        case WM_COMMAND:
            // Check for button click notification (BN_CLICKED is default)
            // LOWORD(wParam) contains the control ID
            switch (LOWORD(wParam)) {
                case ID_BTN_ADD:
                    title = "Add Passenger"; x=100; y=100; width=350; height=250;
                    CreateWindowExA(WS_EX_DLGMODALFRAME, "AddPopupClass", title, WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, width, height, hwnd, NULL, NULL, NULL);
                    break;
                case ID_BTN_DELETE:
                    title = "Delete Passenger"; x=200; y=100; width=300; height=150;
                    CreateWindowExA(WS_EX_DLGMODALFRAME, "DeletePopupClass", title, WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, width, height, hwnd, NULL, NULL, NULL);
                    break;
                case ID_BTN_SEARCH:
                    title = "Search Passenger"; x=300; y=100; width=400; height=250;
                    CreateWindowExA(WS_EX_DLGMODALFRAME, "SearchPopupClass", title, WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, width, height, hwnd, NULL, NULL, NULL);
                    break;
                case ID_BTN_DISPLAY:
                    title = "All Passengers"; x=400; y=100; width=600; height=400;
                    CreateWindowExA(WS_EX_DLGMODALFRAME, "DisplayPopupClass", title, WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, width, height, hwnd, NULL, NULL, NULL);
                    break;
                case ID_BTN_EXIT:
                    // Send a close message to ourselves
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                
                // For all other WM_COMMAND messages (like OK/Cancel from popups), 
                // the system handles the routing, but we must return 0 if we don't
                // want to process them further here.
                default:
                    return 0;
            }
            return 0; // Crucial: Indicate the WM_COMMAND message was handled

        case WM_CLOSE:
            // Save data before closing
            save_passengers_to_file(DATA_FILE_NAME);
            cleanup_passengers();
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// =================================================================
// 3. POPUP WINDOW PROCEDURES
// =================================================================

LRESULT CALLBACK AddWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char name[64], ageStr[5], category[32];
    int age, pnr;
    switch (uMsg) {
        case WM_CREATE:
            CreateWindowA("STATIC", "Name:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, hwnd, NULL, NULL, NULL);
            hAddName = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 10, 200, 20, hwnd, (HMENU)ID_ADD_NAME, NULL, NULL);
            
            CreateWindowA("STATIC", "Age:", WS_VISIBLE | WS_CHILD, 10, 40, 100, 20, hwnd, NULL, NULL, NULL);
            hAddAge = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 120, 40, 50, 20, hwnd, (HMENU)ID_ADD_AGE, NULL, NULL);
            
            CreateWindowA("STATIC", "Category:", WS_VISIBLE | WS_CHILD, 10, 70, 100, 20, hwnd, NULL, NULL, NULL);
            hAddCat = CreateWindowA("EDIT", "General", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 70, 100, 20, hwnd, (HMENU)ID_ADD_CAT, NULL, NULL);
            
            CreateWindowA("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 150, 100, 30, hwnd, (HMENU)ID_ADD_OK, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD, 180, 150, 100, 30, hwnd, (HMENU)ID_ADD_CANCEL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_ADD_OK) {
                GetWindowTextA(hAddName, name, sizeof(name));
                GetWindowTextA(hAddAge, ageStr, sizeof(ageStr));
                GetWindowTextA(hAddCat, category, sizeof(category));
                
                age = atoi(ageStr);

                if (name[0] != '\0' && age > 0) {
                    add_passenger_node(name, age, category, &pnr);
                    char message[100];
                    snprintf(message, sizeof(message), "Ticket Booked!\nPNR: %d", pnr);
                    MessageBoxA(hwnd, message, "Success", MB_OK | MB_ICONINFORMATION);
                    DestroyWindow(hwnd);
                } else {
                    MessageBoxA(hwnd, "Please enter valid Name and Age.", "Error", MB_OK | MB_ICONERROR);
                }
            } else if (LOWORD(wParam) == ID_ADD_CANCEL) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
            
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK DeleteWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char pnrStr[10];
    int pnr;
    switch (uMsg) {
        case WM_CREATE:
            CreateWindowA("STATIC", "Enter PNR to Delete:", WS_VISIBLE | WS_CHILD, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
            hDelPNR = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 170, 10, 100, 20, hwnd, (HMENU)ID_DEL_PNR, NULL, NULL);
            
            CreateWindowA("BUTTON", "Delete", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 80, 100, 30, hwnd, (HMENU)ID_DEL_OK, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD, 180, 80, 100, 30, hwnd, (HMENU)ID_DEL_CANCEL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_DEL_OK) {
                GetWindowTextA(hDelPNR, pnrStr, sizeof(pnrStr));
                pnr = atoi(pnrStr);

                if (delete_passenger_by_pnr(pnr)) {
                    MessageBoxA(hwnd, "Passenger deleted successfully.", "Success", MB_OK | MB_ICONINFORMATION);
                    DestroyWindow(hwnd);
                } else {
                    MessageBoxA(hwnd, "PNR not found.", "Error", MB_OK | MB_ICONERROR);
                }
            } else if (LOWORD(wParam) == ID_DEL_CANCEL) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
            
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char pnrStr[10];
    int pnr;
    Passenger *p;
    switch (uMsg) {
        case WM_CREATE:
            CreateWindowA("STATIC", "Enter PNR to Search:", WS_VISIBLE | WS_CHILD, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
            hSearchPNR = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 170, 10, 100, 20, hwnd, (HMENU)ID_SEARCH_PNR, NULL, NULL);
            
            CreateWindowA("BUTTON", "Search", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 50, 100, 30, hwnd, (HMENU)ID_SEARCH_OK, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD, 180, 50, 100, 30, hwnd, (HMENU)ID_SEARCH_CANCEL, NULL, NULL);
            
            hSearchResult = CreateWindowA("EDIT", "Result will appear here.", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 10, 100, 360, 100, hwnd, NULL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_SEARCH_OK) {
                GetWindowTextA(hSearchPNR, pnrStr, sizeof(pnrStr));
                pnr = atoi(pnrStr);

                p = search_passenger_by_pnr(pnr);
                if (p) {
                    char result[256];
                    snprintf(result, sizeof(result), "PNR: %d\r\nName: %s\r\nAge: %d\r\nCategory: %s", 
                             p->pnr, p->name, p->age, p->category);
                    SetWindowTextA(hSearchResult, result);
                } else {
                    SetWindowTextA(hSearchResult, "PNR not found.");
                }
            } else if (LOWORD(wParam) == ID_SEARCH_CANCEL) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
            
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK DisplayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char displayBuffer[4096]; // Increased size for list
    switch (uMsg) {
        case WM_CREATE:
            hDisplayList = CreateWindowA("EDIT", "Loading...", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_HSCROLL, 
                                         10, 10, 560, 340, hwnd, (HMENU)ID_DISPLAY_LIST, NULL, NULL);
            
            build_passenger_list_string(displayBuffer, sizeof(displayBuffer));
            SetWindowTextA(hDisplayList, displayBuffer);
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
            
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// =================================================================
// 4. MAIN ENTRY POINT (WinMain)
// =================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "MainWindowClass";
    WNDCLASSA wc = {0};

    // 1. Register Main Window Class
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) return 0;

    // 2. Register classes for popup windows (Essential for correct closing and messaging)
    WNDCLASSA wcPopup = {0};
    wcPopup.style = CS_HREDRAW | CS_VREDRAW;
    wcPopup.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); 
    wcPopup.hInstance = hInstance;
    wcPopup.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    wcPopup.lpfnWndProc = AddWndProc;
    wcPopup.lpszClassName = "AddPopupClass";
    RegisterClassA(&wcPopup);
    
    wcPopup.lpfnWndProc = DeleteWndProc;
    wcPopup.lpszClassName = "DeletePopupClass";
    RegisterClassA(&wcPopup);

    wcPopup.lpfnWndProc = SearchWndProc;
    wcPopup.lpszClassName = "SearchPopupClass";
    RegisterClassA(&wcPopup);
    
    wcPopup.lpfnWndProc = DisplayWndProc;
    wcPopup.lpszClassName = "DisplayPopupClass";
    RegisterClassA(&wcPopup);
    
    // LOAD DATA ON STARTUP
    load_passengers_from_file(DATA_FILE_NAME);

    // 3. Create Main Window
    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Advanced Railway Booking System",
        // Added WS_MINIMIZEBOX to allow minimizing
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
        CW_USEDEFAULT, CW_USEDEFAULT, 380, 250,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    // 4. Show Window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 5. Message Loop
    MSG msg = {0};
    // The message loop MUST be here and correctly structured
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}