//
//  MumpiCallback.cpp
//  mumpi
//
//  Created by <author> on 10/05/2016.
//
//

#include "MumpiCallback.hpp"
#include <stdio.h>


MumpiCallback::MumpiCallback()  {

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
    printf("Received audio: pcm_data_size: %d\n", pcm_data_size);
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
    printf("Received text message: %s\n", message.c_str());
}
