#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "config.h"
#include "util.h"
#include "errlist.h"

using namespace std;
using namespace rapidjson;

namespace tinyrpc
{
    Config::Config(const std::string & filename)
    {
        ifstream ifs(filename);
        IStreamWrapper isw(ifs);

        Document d;
        d.ParseStream(isw);

        loadMainSection(d);
        loadMasterSection(d);
        loadProxySection(d);
        loadServerSection(d);
    }

    void Config::loadMainSection(rapidjson::Document &d)
    {
        panicif(! d.HasMember("main"), ERR_INVALID_CONF, "lack of main section");
        panicif(! d["main"].IsObject() || d["main"].HasMember("listen"), ERR_INVALID_CONF, "main section has a invalid format");

        string listen = d["main"]["listen"].GetString();
        auto delemiter = listen.find(":");
        if (delemiter == string::npos)
        {
            m_main.m_host = "0.0.0.0";
            m_main.m_port = stoi(listen);
        }
        else
        {
            m_main.m_host = listen.substr(0, delemiter);
            m_main.m_port = stoi(listen.substr(delemiter+1));
        }
    }

    void Config::loadMasterSection(rapidjson::Document &d)
    {
        panicif(! d.HasMember("master"), ERR_INVALID_CONF, "lack of master section");
        // TODO
    }

    void Config::loadProxySection(rapidjson::Document &d)
    {
        panicif(! d.HasMember("proxy") || ! d["proxy"].IsObject(), ERR_INVALID_CONF, "lack of proxy section or proxy section is not an object");

        auto iter = d["proxy"].FindMember("threads");
        if (iter != d.MemberEnd())
        {
            m_proxy.m_threads = stoi(iter->value.GetString());
        }
        else
        {
            m_proxy.m_threads = 200;
        }

        iter = d["proxy"].FindMember("max_connection");
        if (iter != d.MemberEnd())
        {
            m_proxy.m_maxConn = stoi(iter->value.GetString());
        }
        else
        {
            m_proxy.m_maxConn = 10000;
        }
    }

    void Config::loadServerSection(rapidjson::Document &d)
    {
        for (auto it=d.MemberBegin(); it!=d.MemberEnd(); ++it)
        {
            string name = it->name.GetString();
            if ("main" == name || "master" == name || "proxy" == name)
            {
                continue;
            }

            Value::ConstMemberIterator iter;
            if (! it->value.IsObject() || (iter = it->value.FindMember("threads")) == it->value.MemberEnd())
            {
                continue;
            }


            m_servers[name] = ServerSection(name, stoi(iter->value.GetString()));
        }
    }
}