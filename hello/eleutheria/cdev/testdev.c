/*
 * Compile with:
 * gcc testdev.c -o testdev -I /usr/src/sys -lprop -Wall -W -ansi
 */

#include <err.h>    /* for err() */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* for close() */
#include <sys/ioctl.h>
#include <sys/mydev.h>
#include <prop/proplib.h>

int main(void)
{
    char buffer[1000];
    struct mydev_params params;
    prop_dictionary_t pd;
    prop_string_t ps;
    int devfd, ret;

    /* Open device */
    if ((devfd = open("/dev/mydev", O_RDONLY, 0)) < 0) {
        fprintf(stderr, "Failed to open /dev/mydev\n");
        exit(EXIT_FAILURE);
    }

    /* Send ioctl request in the traditional way */
    params.number = 42;
    strncpy(params.string, "Hello World", MAX_STR);

    if (ioctl(devfd, MYDEVOLDIOCTL, &params) == -1) {
        close(devfd);
        err(EXIT_FAILURE, "ioctl()");
    }

    /* Create dictionary and add a <key, value> pair in it */
    pd = prop_dictionary_create();
    if (pd == NULL) {
        close(devfd);
        err(EXIT_FAILURE, "prop_dictionary_create()");
    }

    ps = prop_string_create_cstring("value");
    if (ps == NULL) {
        close(devfd);
        prop_object_release(pd);
        err(EXIT_FAILURE, "prop_string_create_cstring()");
    }

    if (prop_dictionary_set(pd, "key", ps) == false) {
        close(devfd);
        prop_object_release(ps);
        prop_object_release(pd);
        err(EXIT_FAILURE, "prop_dictionary_set()");
    }

    prop_object_release(ps);

    /* Send dictionary to kernel space */
    prop_dictionary_send_ioctl(pd, devfd, MYDEVSETPROPS);

    prop_object_release(pd);

    /* Read data from device */
    if ((ret = read(devfd, buffer, sizeof buffer)) < 0)
        err(EXIT_FAILURE, "read()");

    printf("testdev: ret = %d, buffer = %s\n", ret, buffer);

    /* Close device */
    if (close(devfd) == -1)
        err(EXIT_FAILURE, "close()");

    return EXIT_SUCCESS;
}
