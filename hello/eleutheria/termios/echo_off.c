/*
 * Compile with:
 * gcc echo_off -o echo_off -Wall -W -Wextra -ansi -pedantic
 *
 * Whenever a user types in a password, it is desirable that
 * the password itself doesn't show up at all. To implement
 * such behaviour, we use the termios(4) interface to disable
 * echo'ing.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#define PASSLEN 64

/* Function prototypes */
void diep(const char *s);

int main(int argc, char *argv[])
{
    struct termios oldt, newt;
    char password[PASSLEN];
    int fd;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s tty\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open terminal device */
    if ((fd = open(argv[1], O_RDONLY | O_NOCTTY) == -1))
        diep("open");

    /* Get current termios structure */
    if (tcgetattr(fd, &oldt) == -1)
        diep("tcgetattr");

    /* Set new termios structure */
    newt = oldt;
    newt.c_lflag &= ~ECHO;     /* disable echoing */
    newt.c_lflag |= ECHONL;    /* echo NL even if ECHO is off */

    if (tcsetattr(fd, TCSANOW, &newt) == -1)
        diep("tcsetattr");

    /* Prompt for password and get it */
    printf("Password: ");
    fgets(password, PASSLEN, stdin);

    /* Restore old termios structure */
    if (tcsetattr(fd, TCSANOW, &oldt) == -1)
        diep("tcsetattr");

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
