//
// Created by qiguo on 2/1/18.
//

#ifndef __TINYRPC_PROTOCOL_H__
#define __TINYRPC_PROTOCOL_H__

#include <string>
#include <memory>
#include <sstream>

namespace tinyrpc
{
    enum ProtocolType
    {
        HEARTBEAT = 1,
        HANDSHAKE,  // handshake before request. here we could do some authorization and verification
        MESSAGE     // transfer data package
    };

    class ExtendArea
    {
    public:
        static std::shared_ptr<ExtendArea> parse(const std::string & src);

        void load(const std::string & src) {}
        void load(const char * src) {}
        const std::string & dump() {}
    };

    /*
     * Message format
     * ----------------------------------------------------------------------
     * | 0x51, 0x47 (2 bytes) | version (2 bytes) | protocol type (4 types) |
     * ----------------------------------------------------------------------
     * |                                                                    |
     * |                    extend area (32 bytes)                          |
     * |                                                                    |
     * ----------------------------------------------------------------------
     * | data len (4 bytes) | remain data (... bytes) | 0x59, 0x46 (2 bytes)|
     * ----------------------------------------------------------------------
     */
    class Message
    {
    public:
        explicit Message(ProtocolType protocol): m_protocol(protocol), m_datalen(0), m_version(1) {}
        Message(): m_protocol(MESSAGE), m_datalen(0), m_version(1) {}
        virtual ~Message() = default;

        Message(const Message & message)
            : m_protocol(message.protocol), m_data(message.m_data), m_datalen(0), m_version(1)
        {
            m_extend = make_shared<ExtendArea> (message.m_extend);
        }

        // inner Parser
        struct Parser
        {
            enum class State    // current state during parsing
            {
                BEGIN = 1,
                VERSION,
                TYPE,
                EXTEND,
                DATALEN,
                DATA,
                END
            };

            Parser() : m_state(State::BEGIN), m_stop(false) {}

            /**
             * @param src
             * @param message
             * @return -1 --- something wrong happended, 0 ---- no data remains.  > 0 --- remains data length after parsing a complete message
             *          return value can be used with dealing multiple message packet
             */
            int operator() (const std::string & src, Message & message);

            /*
             * Check whether parser is completed.
             */
            explicit operator bool () { return m_state == State::END && m_stop; }

        private:
            void jump(State state) { m_state = state; m_buffer.clear(); }

            State               m_state;
            std::stringstream   m_buffer;
            bool                m_stop;
        };

        static bool validProtocol(int type);

        // load the whole data
        void load(const std::string & src);
        void load(const char * src);

        // dump message structure and content
        std::string dump() const;

        void version(uint16_t version) { m_version = version; }
        uint16_t version() const { return m_version; }

        void protocol(ProtocolType protocol) { m_protocol = protocol; }
        ProtocolType protocol() const { return m_protocol; }

        void extend(std::shared_ptr<ExtendArea> && extend) { m_extend = std::move(extend); }
        std::shared_ptr<ExtendArea> extend() const { return m_extend; }

        void data(const std::string & data) { m_data = data; }
        void data(std::string && data) { m_data = std::move(data); }
        void append(const std::string & data) { m_data.append(data); }
        const std::string & data() const { return m_data; }

        void length(uint32_t len) { m_datalen = len; }
        uint32_t length() const { m_datalen == 0 ? m_data.length() : m_datalen; }

    private:
        uint16_t                        m_version;
        ProtocolType                    m_protocol;
        std::shared_ptr<ExtendArea>     m_extend;
        std::string                     m_data;
        uint32_t                        m_datalen;
    };
}

#endif //TINYRPC_PROTOCOL_H
