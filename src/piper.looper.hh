/**
 * Copyright (c) 2023, Timothy Stack
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither the name of Timothy Stack nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef piper_looper_hh
#define piper_looper_hh

#include <future>
#include <memory>
#include <string>

#include "base/auto_fd.hh"
#include "base/result.h"
#include "ghc/filesystem.hpp"

namespace lnav {
namespace piper {

enum class state {
    running,
    finished,
};

class looper {
public:
    looper(std::string name, auto_fd stdout_fd, auto_fd stderr_fd);

    ~looper();

    std::string get_name() const { return this->l_name; }

    ghc::filesystem::path get_out_dir() const { return this->l_out_dir; }

    ghc::filesystem::path get_out_pattern() const
    {
        return this->l_out_dir / "out.*";
    }

    bool is_finished() const
    {
        return this->l_future.wait_for(std::chrono::seconds(0))
            == std::future_status::ready;
    }

private:
    void loop();

    std::atomic<bool> l_looping{true};
    const std::string l_name;
    ghc::filesystem::path l_out_dir;
    auto_fd l_stdout;
    auto_fd l_stderr;
    std::future<void> l_future;
};

template<state LooperState>
class handle {
public:
    explicit handle(std::shared_ptr<looper> looper)
        : h_looper(std::move(looper))
    {
    }

    std::string get_name() const { return this->h_looper->get_name(); }

    ghc::filesystem::path get_out_dir() const
    {
        return this->h_looper->get_out_dir();
    }

    ghc::filesystem::path get_out_pattern() const
    {
        return this->h_looper->get_out_pattern();
    }

    bool is_finished() const { return this->h_looper->is_finished(); }

    bool operator==(const handle& other) const
    {
        return this->h_looper.get() == other.h_looper.get();
    }

private:
    std::shared_ptr<looper> h_looper;
};

using running_handle = handle<state::running>;

Result<handle<state::running>, std::string> create_looper(std::string name,
                                                          auto_fd stdout_fd,
                                                          auto_fd stderr_fd);

}  // namespace piper
}  // namespace lnav

#endif
