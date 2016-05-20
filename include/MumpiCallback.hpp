//
//  MumpiCallback.hpp
//  mumpi
//
//  Created by <author> on 10/05/2016.
//
//

#ifndef MumpiCallback_hpp
#define MumpiCallback_hpp

#include <string>
#include <stdio.h>
#include "RingBuffer.hpp"
#include "mumlib/Transport.hpp"

class MumpiCallback : public mumlib::BasicCallback {
public:
    MumpiCallback(std::shared_ptr<RingBuffer<int16_t>> out_buf);
    ~MumpiCallback();

    virtual void serverSync(std::string welcome_text,
                            int32_t session,
                            int32_t max_bandwidth,
                            int64_t permissions) override;

    virtual void audio(int target,
                       int sessionId,
                       int sequenceNumber,
                       int16_t *pcm_data,
                       uint32_t pcm_data_size) override;

    virtual void textMessage(uint32_t actor,
                             std::vector<uint32_t> session,
                             std::vector<uint32_t> channel_id,
                             std::vector<uint32_t> tree_id,
                             std::string message) override;

    mumlib::Mumlib *mum;
private:
    std::shared_ptr<RingBuffer<int16_t>> _out_buf;
    log4cpp::Category& _logger = log4cpp::Category::getInstance("mumpi.MumpiCallback");
};


#endif /* MumpiCallback_hpp */
