/*
 * Facility for queueing callback functions to be run from the
 * top-level event loop after the current top-level activity finishes.
 */

#include <stddef.h>

#include "putty.h"

struct callback {
    struct callback *next;

    toplevel_callback_fn_t fn;
    void *ctx;
};

struct callback *cbcurr = NULL, *cbhead = NULL, *cbtail = NULL;

toplevel_callback_notify_fn_t notify_frontend = NULL;
void *frontend = NULL;

void request_callback_notifications(toplevel_callback_notify_fn_t fn,
                                    void *fr)
{
    MPEXT_PUTTY_SECTION_ENTER;
    notify_frontend = fn;
    frontend = fr;
    MPEXT_PUTTY_SECTION_LEAVE;
}

void delete_callbacks_for_context(void *ctx)
{
    struct callback *newhead, *newtail;

    newhead = newtail = NULL;
    while (cbhead) {
        struct callback *cb = cbhead;
        cbhead = cbhead->next;
        if (cb->ctx == ctx) {
            sfree(cb);
        } else {
            if (!newhead)
                newhead = cb;
            else
                newtail->next = cb;

            newtail = cb;
        }
    }

    cbhead = newhead;
    cbtail = newtail;
}

void queue_toplevel_callback(toplevel_callback_fn_t fn, void *ctx)
{
    struct callback *cb;

    MPEXT_PUTTY_SECTION_ENTER;
    cb = snew(struct callback);
    cb->fn = fn;
    cb->ctx = ctx;

    /*
     * If the front end has requested notification of pending
     * callbacks, and we didn't already have one queued, let it know
     * we do have one now.
     *
     * If cbcurr is non-NULL, i.e. we are actually in the middle of
     * executing a callback right now, then we count that as the queue
     * already having been non-empty. That saves the front end getting
     * a constant stream of needless re-notifications if the last
     * callback keeps re-scheduling itself.
     */
    if (notify_frontend && !cbhead && !cbcurr)
        notify_frontend(frontend);

    if (cbtail)
        cbtail->next = cb;
    else
        cbhead = cb;
    cbtail = cb;
    cb->next = NULL;
    MPEXT_PUTTY_SECTION_LEAVE;
}

void run_toplevel_callbacks(void)
{
    MPEXT_PUTTY_SECTION_ENTER;
    if (cbhead) {
#ifdef MPEXT
        toplevel_callback_fn_t fn = cbcurr->fn;
        void * ctx = cbcurr->ctx;
        cbhead = cbcurr->next;
        if (!cbhead)
            cbtail = NULL;
        cbhead = cbcurr->next;
        sfree(cbcurr);
        MPEXT_PUTTY_SECTION_LEAVE;
        fn(ctx);
#else
        /*
         * Transfer the head callback into cbcurr to indicate that
         * it's being executed. Then operations which transform the
         * queue, like delete_callbacks_for_context, can proceed as if
         * it's not there.
         */
        cbcurr = cbhead;
        cbhead = cbhead->next;
        if (!cbhead)
            cbtail = NULL;
#endif
    }
#ifdef MPEXT
    else
    {
      MPEXT_PUTTY_SECTION_LEAVE;

        /*
         * Now run the callback, and then clear it out of cbcurr.
         */
        cbcurr->fn(cbcurr->ctx);
        sfree(cbcurr);
        cbcurr = NULL;
    }
#endif
}

int toplevel_callback_pending(void)
{
    // MP does not have to be guarded
    return cbcurr != NULL || cbhead != NULL;
}
