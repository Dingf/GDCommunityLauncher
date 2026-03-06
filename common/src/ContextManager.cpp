#include "ContextManager.h"

std::unique_ptr<ContextManager::WorkGuard> ContextManager::_work;
std::thread ContextManager::_thread;

asio::io_context& ContextManager::GetIOContext()
{
    static asio::io_context ioc;
    return ioc;
}

ssl::context& ContextManager::GetSSLContext()
{
    static ssl::context sslc{ssl::context::tlsv12_client};
    return sslc;
}

void ContextManager::Run()
{
    if (!_thread.joinable())
    {
        asio::io_context& ioc = GetIOContext();
        _work = std::make_unique<WorkGuard>(asio::make_work_guard(ioc));
        _thread = std::thread([&ioc]()
        {
            ioc.run();
        });
    }
}

void ContextManager::Stop()
{
    if (_thread.joinable())
    {
        GetIOContext().stop();
        _work->reset();
        _thread.join();
    }
}