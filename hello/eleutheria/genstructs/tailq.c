/*
 * Compile with:
 * gcc tailq.c -o tailq -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

TAILQ_HEAD(tailhead, entry) head;

struct entry {
    TAILQ_ENTRY(entry) entries;
    const char *str;
} *np, *n;

int main(void)
{
    const char *str[] = { "this", "is", "a", "double", "linked", "tailq" };
    unsigned int i;

    /* Initialize list */
    TAILQ_INIT(&head);

    /* Populate list with str[] items */
    for (i = 0; i < sizeof str / sizeof *str; i++) {
        if ((n = malloc(sizeof(struct entry))) == NULL) {
            perror("malloc()");
           goto CLEANUP_AND_EXIT;
        }
        n->str = str[i];

        TAILQ_INSERT_TAIL(&head, n, entries);
    }

    /* Traverse list forward */
    TAILQ_FOREACH(np, &head, entries)
        printf("%s %s", np->str,
               TAILQ_NEXT(np, entries) != NULL ?
               "-> " : "\n");

    /* Traverse list in reverse order */
    TAILQ_FOREACH_REVERSE(np, &head, tailhead, entries)
        printf("%s %s", np->str,
               TAILQ_PREV(np, tailhead, entries) != NULL ?
               "-> " : "\n");

 CLEANUP_AND_EXIT:;
    /* Delete all elements */
    while (TAILQ_FIRST(&head) != NULL) {
        np = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, TAILQ_FIRST(&head), entries);
	free(np);
    }

    return EXIT_SUCCESS;
}
