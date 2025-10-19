#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "motd.h"

void motd() {
    // Initialize time variables
    time_t t;
    struct tm tm;
    int month, day;

    t = time(NULL);
    tm = *localtime(&t);
    month = tm.tm_mon + 1;  // months are 1–12
    day   = tm.tm_mday;

    if (month == 10 && day == 11) {
        printf("\n\033[1;35mHappy Birthday, WNU OS!\033[0m\n");
        printf("Thank you for being part of our journey since October 11, 2025.\n\n");
    }
    else if (month == 12 && day == 25) {
        printf("\n\033[1;32mMerry Christmas!\033[0m\n");
        printf("WNU OS wishes you a joyful holiday season filled with peace and happiness.\n\n");
    }
    else if (month == 1 && day == 1) {
        printf("\n\033[1;34mHappy New Year!\033[0m\n");
        printf("WNU OS wishes you a prosperous and exciting year ahead!\n\n");
    }
    else if (month == 10 && day == 1) {
        printf("\n\033[1;33mProject Anniversary!\033[0m\n");
        printf("Commit 5c825c434d6d7e0f7472369693a98406423fe02b started WNU OS on October 1, 2025.\n\n");
    }
    else if (month == 9 && day == 18) {
        printf("\n\033[1;36mHappy Founding Day!\033[0m\n");
        printf("The WNU Project organization was created on September 18, 2025.\n\n");
    }
    else if (month == 11 && day == 26) {
        printf("\n\033[1;33mHappy Birthday, Coolis1362!\033[0m\n");
        printf("Thank For Coolis1362 For Making The WNU Project\n\n");
    }
    else {
        printf("\n\033[1;36mWelcome to WNU OS 1.0.1 Update 2!\033[0m\n");
    }
}
