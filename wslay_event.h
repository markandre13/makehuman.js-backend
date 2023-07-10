#include <wslay/wslay.h>
#include <stdint.h>

enum wslay_event_close_status { WSLAY_CLOSE_RECEIVED = 1 << 0, WSLAY_CLOSE_QUEUED = 1 << 1, WSLAY_CLOSE_SENT = 1 << 2 };

struct wslay_event_context {
        /* config status, bitwise OR of enum wslay_event_config values*/
        uint32_t config;
        /* maximum message length that can be received */
        uint64_t max_recv_msg_length;
        /* 1 if initialized for server, otherwise 0 */
        uint8_t server;
        /* bitwise OR of enum wslay_event_close_status values */
        uint8_t close_status;
        /* status code in received close control frame */
        uint16_t status_code_recv;
        /* status code in sent close control frame */
        uint16_t status_code_sent;
        wslay_frame_context_ptr frame_ctx;
        /* 1 if reading is enabled, otherwise 0. Upon receiving close
           control frame this value set to 0. If any errors in read
           operation will also set this value to 0. */
        uint8_t read_enabled;
        /* 1 if writing is enabled, otherwise 0 Upon completing sending
           close control frame, this value set to 0. If any errors in write
           opration will also set this value to 0. */
        uint8_t write_enabled;
        /* imsg buffer to allow interleaved control frame between
           non-control frames. */
        //   struct wslay_event_imsg imsgs[2];
        //   /* Pointer to imsgs to indicate current used buffer. */
        //   struct wslay_event_imsg *imsg;
        //   /* payload length of frame currently being received. */
        //   uint64_t ipayloadlen;
        //   /* next byte offset of payload currently being received. */
        //   uint64_t ipayloadoff;
        //   /* error value set by user callback */
        //   int error;
        //   /* Pointer to the message currently being sent. NULL if no message
        //      is currently sent. */
        //   struct wslay_event_omsg *omsg;
        //   /* Queue for non-control frames */
        //   struct wslay_queue /*<wslay_omsg*>*/ send_queue;
        //   /* Queue for control frames */
        //   struct wslay_queue /*<wslay_omsg*>*/ send_ctrl_queue;
        //   /* Size of send_queue + size of send_ctrl_queue */
        //   size_t queued_msg_count;
        //   /* The sum of message length in send_queue */
        //   size_t queued_msg_length;
        //   /* Buffer used for fragmented messages */
        //   uint8_t obuf[4096];
        //   uint8_t *obuflimit;
        //   uint8_t *obufmark;
        //   /* payload length of frame currently being sent. */
        //   uint64_t opayloadlen;
        //   /* next byte offset of payload currently being sent. */
        //   uint64_t opayloadoff;
        //   struct wslay_event_callbacks callbacks;
        //   struct wslay_event_frame_user_data frame_user_data;
        //   void *user_data;
        //   uint8_t allowed_rsv_bits;
};