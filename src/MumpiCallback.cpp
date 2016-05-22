#include "MumpiCallback.hpp"


MumpiCallback::MumpiCallback(std::shared_ptr<RingBuffer<int16_t>> out_buf) :
        _out_buf(out_buf) {
}

MumpiCallback::~MumpiCallback() {

}

void MumpiCallback::serverSync(std::string welcome_text,
                               int32_t session,
                               int32_t max_bandwidth,
                               int64_t permissions) {
    mum->sendTextMessage("Hello world!");
}

/**
 * [MumpiCallback::audio description]
 * @param target         [description]
 * @param sessionId      [description]
 * @param sequenceNumber [description]
 * @param pcm_data       [description]
 * @param pcm_data_size  [description]
 */
void MumpiCallback::audio(int target,
                          int sessionId,
                          int sequenceNumber,
                          int16_t *pcm_data,
                          uint32_t pcm_data_size) {
    _logger.info("Received audio: pcm_data_size: %d", pcm_data_size);
    if(pcm_data != NULL) {
        _out_buf->push(pcm_data, 0, pcm_data_size);
    }
}

/**
 * [MumpiCallback::textMessage description]
 * @param  actor      [description]
 * @param  session    [description]
 * @param  channel_id [description]
 * @param  tree_id    [description]
 * @param  message    [description]
 * @return            [description]
 */
void  MumpiCallback::textMessage(uint32_t actor,
                                 std::vector<uint32_t> session,
                                 std::vector<uint32_t> channel_id,
                                 std::vector<uint32_t> tree_id,
                                 std::string message) {
    mumlib::BasicCallback::textMessage(actor, session, channel_id, tree_id, message);
    _logger.info("Received text message: %s", message.c_str());
}
