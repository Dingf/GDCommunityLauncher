#ifndef INC_GDCL_CONTEXT_MANAGER_H
#define INC_GDCL_CONTEXT_MANAGER_H

#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace asio = boost::asio;
namespace ssl  = boost::asio::ssl;

class ContextManager
{
    public:
        static asio::io_context& GetIOContext();
        static ssl::context&     GetSSLContext();

        static void Run();
        static void Stop();

    private:
        typedef asio::executor_work_guard<asio::io_context::executor_type> WorkGuard;

        static std::unique_ptr<WorkGuard> _work;
        static std::thread _thread;
};

#endif//INC_GDCL_CONTEXT_MANAGER_H