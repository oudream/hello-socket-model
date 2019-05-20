#include <sys/param.h>
#include <sys/systm.h>    /* for uiomove(9) */
#include <sys/proc.h>
/*#include <sys/errno.h>*/
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/malloc.h>    /* for free(9) */
#include <sys/mydev.h>
#include <sys/kauth.h>
#include <sys/syslog.h>
#include <prop/proplib.h>

/* These macros expand to function prototypes needed for autoconf(9) */
void mydevattach(struct device *parent, struct device *self, void *aux);
static dev_type_open(mydevopen);
static dev_type_close(mydevclose);
static dev_type_read(mydevread);
static dev_type_ioctl(mydevioctl);

static struct mydev_softc {
    struct device sc_mydev;    /* generic device info */

    unsigned int sc_usage;     /* number of open devices */
} mydev_softc;

/*
 * Each device driver must present to the system a standard
 * autoconfiguration interface. See driver(9).
 */
CFATTACH_DECL(mydev,                        /* driver name */
              sizeof (struct mydev_softc),  /* size of instance data */
              NULL,                         /* match/probe function */
              mydevattach,                  /* attach function */
              NULL,                         /* detach function */
              NULL);                        /* activate function */

/* Define the character dev handlers */
const struct cdevsw mydev_cdevsw = {
    mydevopen,
    mydevclose,
    mydevread,
    nowrite,
    mydevioctl,
    nostop,
    notty,
    nopoll,
    nommap,
    nokqfilter,
    D_OTHER,
};

/*
 * This private buffer is the pool we draw data from
 * and send them over to userspace upon a read() request.
 */
static char mybuffer[] = "This is a test";

/*
 * Attach for autoconfig to find.
 */
void
mydevattach(struct device *parent, struct device *self, void *aux)
{
    /*
     * This is where resources that need to be allocated/initialised
     * can be set up, prior to xxxopen() call.
    */
    log(LOG_DEBUG, "mydev: pseudo-device attached\n");
    mydev_softc.sc_usage = 0;
}

/*
 * Handle an open request on the dev.
 */
static int
mydevopen(dev_t dev, int flags, int fmt, struct lwp *proc)
{
    log(LOG_DEBUG, "mydev: pseudo-device open attempt by "
        "uid=%u, pid=%u. (dev=%u, major=%d, minor=%d)\n",
        kauth_cred_geteuid(proc->l_cred), proc->l_proc->p_pid,
        dev, major(dev), minor(dev));

    if (mydev_softc.sc_usage > 0) {
        log(LOG_ERR, "mydev: pseudo-device already in use\n");
        return EBUSY;
    }

    mydev_softc.sc_usage++;

    return 0;    /* Success */
}

/*
 * Handle the close request for the dev.
 */
static int
mydevclose(dev_t dev, int flags, int fmt, struct lwp *proc)
{
    if (mydev_softc.sc_usage > 0)
        mydev_softc.sc_usage--;

    log(LOG_DEBUG, "mydev: pseudo-device closed\n");

    return 0;    /* Success */
}

/*
 * Handle read request on the dev.
 */
static int
mydevread(dev_t dev, struct uio *uio, int ioflag)
{
    int ret;
    size_t nbytes;

    log(LOG_DEBUG, "mydev: uio: iov = %p, iovcnt = %d, resid = %u, vmspace = %p\n",
        uio->uio_iov, uio->uio_iovcnt, uio->uio_resid, uio->uio_vmspace);

    while (uio->uio_resid > 0) {
        nbytes = MIN(uio->uio_resid, sizeof mybuffer);
        if ((ret = uiomove(mybuffer, nbytes, uio)) != 0)
            return ret;    /* Error */
    }

    return 0;    /* Success */
}


/*
 * Handle the ioctl for the dev.
 */
static int
mydevioctl(dev_t dev, u_long cmd, void *data, int flags,
           struct lwp *proc)
{
    const struct mydev_params *params;
    const struct plistref *pref;
    char *val;
    prop_dictionary_t dict;
    prop_object_t po;
    int error = 0;

    switch (cmd) {
    case MYDEVOLDIOCTL:
        /* Pass data from userspace to kernel in the conventional way */
        params = (const struct mydev_params *)data;
        log(LOG_DEBUG, "mydev: got number of %d and string of %s\n",
            params->number, params->string);
        break;

    case MYDEVSETPROPS:
        /* Use proplib(3) for userspace/kernel communication */
        pref = (const struct plistref *)data;
        error = prop_dictionary_copyin_ioctl(pref, cmd, &dict);
        if (error)
            return error;

        /* Print dict's count for debugging purposes */
        log(LOG_DEBUG, "mydev: dict count = %u\n",
            prop_dictionary_count(dict));

        /* Retrieve object associated with "key" key */
        po = prop_dictionary_get(dict, "key");
        if (po == NULL || prop_object_type(po) != PROP_TYPE_STRING) {
            log(LOG_DEBUG, "mydev: prop_dictionary_get() failed\n");
            prop_object_release(dict);
            return -1;
        }

        /* Print data */
        val = prop_string_cstring(po);
        if (val == NULL) {
            log(LOG_DEBUG, "mydev: prop_string_cstring() failed\n");
            prop_object_release(po);
            prop_object_release(dict);
            return -1;
        }
        prop_object_release(po);
        log(LOG_DEBUG, "mydev: <x, y> = (%s, %s)\n", "key", val);
        free(val, M_TEMP);

        /* Done */
        prop_object_release(dict);
        break;

    default:
        /* Inappropriate ioctl for device */
        error = ENOTTY;
    }

    return error;
}
