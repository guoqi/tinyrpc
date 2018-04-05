//
// Created by qiguo on 2/3/18.
//

#include "protocol.h"
#include "logger.h"
#include "util.h"
#include "errlist.h"
#include <algorithm>
#include <map>
#include <cstring>
#include <sstream>

using namespace std;
using namespace util;

const static size_t HEADER_SIZE = 2;
const static size_t VERSION_SIZE = 2;
const static size_t TYPE_SIZE = 4;
const static size_t SRC_SIZE = 8;
const static size_t DST_SIZE = 8;
const static size_t SEQNO_SIZE = 8;
const static size_t EXTEND_SIZE = 32;
const static size_t DATALEN_SIZE = 4;
const static size_t END_SIZE = 2;

namespace tinyrpc
{
    const static map<Message::Parser::State, size_t> blockSize = {
            {Message::Parser::State::BEGIN, HEADER_SIZE},
            {Message::Parser::State::VERSION, VERSION_SIZE},
            {Message::Parser::State::TYPE, TYPE_SIZE},
            {Message::Parser::State::SRC, SRC_SIZE},
            {Message::Parser::State::DST, DST_SIZE},
            {Message::Parser::State::SEQNO, SEQNO_SIZE},
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
                case State::SRC:
                    uint64_t src;
                    memcpy(&src, m_buffer.str().data(), sizeof(src));
                    message.src(src);
                    break;
                case State::DST:
                    uint64_t dst;
                    memcpy(&dst, m_buffer.str().data(), sizeof(dst));
                    message.dst(dst);
                    break;
                case State::SEQNO:
                    uint64_t seq;
                    memcpy(&seq, m_buffer.str().data(), sizeof(seq));
                    message.seqno(seq);
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

    std::string Message::pack() const
    {
        string msg;
        char header[1024] = {0};
        char * p = header;

        memcpy(p, "QG", HEADER_SIZE);
        p += HEADER_SIZE;
        memcpy(p, &m_version, VERSION_SIZE);
        p += VERSION_SIZE;
        memcpy(p, &m_protocol, TYPE_SIZE);
        p += TYPE_SIZE;
        memcpy(p, &m_seqno, SEQNO_SIZE);
        p += SEQNO_SIZE;
        if (m_extend == nullptr)
        {
            memcpy(p, m_extend->dump().c_str(), EXTEND_SIZE);
        }
        p += EXTEND_SIZE;
        int len = length();
        memcpy(p, &len, DATALEN_SIZE);
        p += DATALEN_SIZE;

        msg.append(string(header, p - header));
        msg.append(m_data);
        return msg;
    }

    Message Message::recvBy(const std::shared_ptr<tinynet::TcpConn> & conn)
    {
        char buffer[4096] = {0};
        ssize_t len = 0;
        Message::Parser parser;
        Message msg;
        do
        {
            len = conn->recv(buffer, sizeof(buffer));
            if (len > 0)
                parser(buffer, msg);
        } while (len > 0 && ! parser);

        panicif(! parser, ERR_INVALID_MESSAGE, "incomplete message packet");
    }

    void Message::sendBy(std::shared_ptr<tinynet::TcpConn> conn) const
    {
        conn->sendall(pack());
    }


    std::shared_ptr<ExtendArea> ExtendArea::parse(const std::string &src)
    {
        return make_shared<ExtendArea>();
    }


    // proxy protocol
    int ProxyProto::Parser::operator()(const std::string & src, ProxyProto &pp)
    {
        size_t i = 0;
        size_t len = src.length();

        while (i < len && ! m_stop)
        {
            m_buffer << src[i++];

            if (m_buffer.str().size() != 8)
            {
                continue;
            }

            switch(m_state)
            {
                case State::SRC:
                    uint64_t srcid;
                    memcpy(&srcid, m_buffer.str().c_str(), sizeof(srcid));
                    pp.src(srcid);
                    jump(State::DST);
                    break;
                case State::DST:
                    uint64_t dstid;
                    memcpy(&dstid, m_buffer.str().c_str(), sizeof(dstid));
                    pp.dst(dstid);
                    jump(State::END);
                    m_stop = true;
                    break;
                case State::END:
                    error("invalid proxy protocol parsing state!");
                    break;
            }
        }

        return static_cast<int>(len - i);
    }

    std::string ProxyProto::dump(const Message &msg)
    {
        string rslt;
        char buffer[32] = {0};

        memcpy(buffer, &m_srcid, sizeof(m_srcid));
        memcpy(buffer + sizeof(m_srcid), &m_dstid, sizeof(m_dstid));

        rslt.append(string(buffer, sizeof(m_srcid) + sizeof(m_dstid)));
        rslt.append(msg.pack());
        return rslt;
    }
}