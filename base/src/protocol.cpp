//
// Created by qiguo on 2/3/18.
//

#include "protocol.h"
#include "logger.h"
#include "util.h"
#include <algorithm>
#include <map>
#include <cstring>
#include <sstream>

using namespace std;

const static size_t HEADER_SIZE = 2;
const static size_t VERSION_SIZE = 2;
const static size_t TYPE_SIZE = 4;
const static size_t EXTEND_SIZE = 32;
const static size_t DATALEN_SIZE = 4;
const static size_t END_SIZE = 2;

namespace tinyrpc
{
    const static map<Message::Parser::State, size_t> blockSize = {
            {Message::Parser::State::BEGIN, HEADER_SIZE},
            {Message::Parser::State::VERSION, VERSION_SIZE},
            {Message::Parser::State::TYPE, TYPE_SIZE},
            {Message::Parser::State::EXTEND, EXTEND_SIZE},
            {Message::Parser::State::DATALEN, DATALEN_SIZE},
            {Message::Parser::State::END, END_SIZE}
    };

    int Message::Parser::operator()(const std::string &src, Message &message)
    {
        size_t i = 0;
        size_t len = src.length();

        while (i < len && ! m_stop)
        {
            size_t size = m_state == State::DATA ? message.data().length() : blockSize.at(m_state);
            m_buffer << src[i++];

            if (m_buffer.str().size() != size)
            {
                continue;
            }

            switch (m_state)
            {
                case State::BEGIN:
                    if (m_buffer.str() == "QG") {
                        jump(State::VERSION);
                    } else {
                        // invalid header. return directly
                        error("invalid header: %s", m_buffer.str().c_str());
                        return -1;
                    }
                    break;
                case State::VERSION:
                    uint16_t version;
                    memcpy(&version, m_buffer.str().data(), sizeof(version));
                    message.version(version);
                    jump(State::TYPE);
                    break;
                case State::TYPE:
                    uint32_t protocol;
                    memcpy(&protocol, m_buffer.str().data(), sizeof(protocol));
                    if (Message::validProtocol(protocol)) {
                        message.protocol(static_cast<ProtocolType> (protocol));
                        jump(State::EXTEND);
                    } else {
                        error("invalid protocol type: %d", protocol);
                        return -1;
                    }
                    break;
                case State::EXTEND:
                    message.extend(ExtendArea::parse(m_buffer.str()));
                    jump(State::DATALEN);
                    break;
                case State::DATALEN:
                    uint32_t datalen;
                    memcpy(&datalen, m_buffer.str().data(), sizeof(datalen));
                    message.length(datalen);
                    jump(State::DATA);
                    break;
                case State::DATA:
                    message.append(m_buffer.str());
                    jump(State::END);
                    break;
                case State::END:
                    if (m_buffer.str() == "YF") {
                        m_stop = true;
                    } else {
                        error("invalid end block: %s", m_buffer.str().c_str());
                        return -1;
                    }
                    break;
            }
        }

        return static_cast<int>(len - i);
    }

    bool Message::validProtocol(int protocol)
    {
        return protocol == HEARTBEAT || protocol == HANDSHAKE || protocol == MESSAGE;
    }

    void Message::load(const std::string &src)
    {
        Message::Parser parser;
        parser(src, *this);
    }

    void Message::load(const char *src)
    {
        load(string(src));
    }

    std::string Message::dump() const
    {
        string msg;
        char header[1024] = {0};

        memcpy(header, "QG", HEADER_SIZE);
        memcpy(header + 2, &m_version, VERSION_SIZE);
        memcpy(header + 4, &m_protocol, TYPE_SIZE);
        memcpy(header + 8, m_extend->dump().c_str(), EXTEND_SIZE);
        int len = length();
        memcpy(header + 40, &len, DATALEN_SIZE);

        msg.append(string(header, HEADER_SIZE + VERSION_SIZE + TYPE_SIZE + EXTEND_SIZE + DATALEN_SIZE));
        msg.append(m_data);
        return msg;
    }


    std::shared_ptr<ExtendArea> ExtendArea::parse(const std::string &src)
    {
    }
}