#include <fluent-bit/flb_input_plugin.h>
#include <fluent-bit/flb_config.h>
#include <fluent-bit/flb_pack.h>
#include <fluent-bit/flb_input_chunk.h>

// Context structure to hold plugin instance data
struct flb_in_hello_world_config {
    int tick_interval;
    struct flb_input_instance *ins; 
};

// Callback triggered by fluentbit engine timer
static int cb_hello_world_collect(struct flb_input_instance *ins, struct flb_config *config, void *in_context){
    msgpack_packer mp_pctl;
    msgpack_sbuffer mp_sbuf;

    // Init msgpack buffer to format output log
    msgpack_sbuffer_init(&mp_sbuf);
    msgpack_packer_init(&mp_pctl, &mp_sbuf, msgpack_sbuffer_write);

    // Structure data for fluentbit [timestamp, {key: value}]
    msgpack_pack_array(&mp_pctl, 2);
    flb_pack_time_now(&mp_pctl); // Element 1 == current timestamp

    msgpack_pack_map(&mp_pctl, 1); // Element 2 == map with 1 key pair
    msgpack_pack_str(&mp_pctl, 7);
    msgpack_pack_str_body(&mp_pctl, "message", 7);
    msgpack_pack_str(&mp_pctl, 11);
    msgpack_pack_str_body(&mp_pctl, "Hello World", 11);

    // Send packed message data to fluentbit engine
    flb_input_chunk_append_raw(ins, FLB_LOG_EVENT, 0, NULL, 0, mp_sbuf.data, mp_sbuf.size);

    msgpack_sbuffer_destroy(&mp_sbuf);
    return 0;
}

// Init hello world plugin instance
static int cb_hello_world_init(struct flb_input_instance *ins, struct flb_config *config, void *data){
    struct flb_in_hello_world_config *ctx;
    int ret;

    ctx = flb_calloc(1, sizeof(struct flb_in_hello_world_config));
    if (!ctx){
        flb_errno();
        return -1;
    }

    // Set default collection interval (1 second)
    ctx->tick_interval = 1;

    // Register timer collector to trigger callback
    ret = flb_input_set_collector_time(
        ins,
        cb_hello_world_collect,
        ctx->tick_interval,
        0,
        config
    );

    if (ret < 0){
        flb_free(ctx);
        return -1;
    }

    flb_input_set_context(ins, ctx);

    return 0;
}

// Cleanup after fluentbit is destroyed
static int cb_hello_world_exit(void *data, struct flb_config *config){
    struct flb_in_hello_world_config *ctx = data;
    if (!ctx){ return 0; }
    flb_free(ctx);
    return 0;
}

// Register plugin definition with fluent bit core
struct flb_input_plugin in_hello_world_plugin = {
    .name           =   "hello_world",
    .description    =   "Hello world input plugin",
    .cb_init        =   cb_hello_world_init,
    .cb_pre_run     =   NULL,
    .cb_collect     =   cb_hello_world_collect,
    .cb_flush_buf   =   NULL,
    .cb_exit        =   cb_hello_world_exit,
    .flags          =   0
};