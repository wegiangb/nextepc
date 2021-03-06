#define TRACE_MODULE _sgw_init

#include "core_debug.h"
#include "core_thread.h"

#include "sgw_context.h"
#include "sgw_event.h"

static thread_id sm_thread;
static void *THREAD_FUNC sm_main(thread_id id, void *data);

static thread_id net_thread;
static void *THREAD_FUNC net_main(thread_id id, void *data);

static int initialized = 0;

status_t sgw_initialize()
{
    status_t rv;

    rv = sgw_context_init();
    if (rv != CORE_OK) return rv;

    rv = sgw_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = sgw_context_setup_trace_module();
    if (rv != CORE_OK) return rv;

    rv = thread_create(&sm_thread, NULL, sm_main, NULL);
    if (rv != CORE_OK) return rv;
    rv = thread_create(&net_thread, NULL, net_main, NULL);
    if (rv != CORE_OK) return rv;

    initialized = 1;

    return CORE_OK;
}

void sgw_terminate(void)
{
    if (!initialized) return;

    thread_delete(net_thread);
    thread_delete(sm_thread);

    sgw_context_final();

    gtp_xact_final();
}

static void *THREAD_FUNC sm_main(thread_id id, void *data)
{
    event_t event;
    fsm_t sgw_sm;
    c_time_t prev_tm, now_tm;
    int r;

    memset(&event, 0, sizeof(event_t));

    sgw_self()->queue_id = event_create(MSGQ_O_BLOCK);
    d_assert(sgw_self()->queue_id, return NULL, 
            "SGW event queue creation failed");
    tm_service_init(&sgw_self()->tm_service);
    gtp_xact_init(&sgw_self()->tm_service,
            SGW_EVT_T3_RESPONSE, SGW_EVT_T3_HOLDING);

    fsm_create(&sgw_sm, sgw_state_initial, sgw_state_final);
    fsm_init(&sgw_sm, 0);

    prev_tm = time_now();

    while ((!thread_should_stop()))
    {
        r = event_timedrecv(sgw_self()->queue_id, &event, EVENT_WAIT_TIMEOUT);

        d_assert(r != CORE_ERROR, continue,
                "While receiving a event message, error occurs");

        now_tm = time_now();

        /* if the gap is over 10 ms, execute preriodic jobs */
        if (now_tm - prev_tm > EVENT_WAIT_TIMEOUT)
        {
            tm_execute_tm_service(
                    &sgw_self()->tm_service, sgw_self()->queue_id);

            prev_tm = now_tm;
        }

        if (r == CORE_TIMEUP)
        {
            continue;
        }

        fsm_dispatch(&sgw_sm, (fsm_event_t*)&event);
    }

    fsm_final(&sgw_sm, 0);
    fsm_clear(&sgw_sm);

    event_delete(sgw_self()->queue_id);

    return NULL;
}

static void *THREAD_FUNC net_main(thread_id id, void *data)
{
    while (!thread_should_stop())
    {
        net_fds_read_run(50); 
    }

    return NULL;
}
