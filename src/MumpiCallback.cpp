#include "MumpiCallback.hpp"


MumpiCallback::MumpiCallback(std::shared_ptr<RingBuffer<int16_t>> out_buf) :
        _out_buf(out_buf) {
}

MumpiCallback::~MumpiCallback() {

}

/**
 * Handles received serverSync messages (when connection established).
 *
 * @param welcome_text  welcome text
 * @param session       session
 * @param max_bandwidth max bandwidth
 * @param permissions   permissions
 */
void MumpiCallback::serverSync(std::string welcome_text,
                               int32_t session,
                               int32_t max_bandwidth,
                               int64_t permissions) {
    _logger.info("Joined server!");
    _logger.info(welcome_text);
}

/**
 * Handles received audio packets and pushes them to the circular buffer
 *
 * @param target         target
 * @param sessionId      session ifndef
 * @param sequenceNumber sequence number
 * @param pcm_data       raw PCM data (int16_t)
 * @param pcm_data_size  PCM data buf size
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
 * Handles received text messages
 * @param  actor      actor
 * @param  session    session
 * @param  channel_id channel id
 * @param  tree_id    tree id
 * @param  message    the message
 */
void MumpiCallback::textMessage(uint32_t actor,
                                std::vector<uint32_t> session,
                                std::vector<uint32_t> channel_id,
                                std::vector<uint32_t> tree_id,
                                std::string message) {
    _logger.info("Received text message: %s", message.c_str());
}
