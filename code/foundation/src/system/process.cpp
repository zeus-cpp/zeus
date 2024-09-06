#include "zeus/foundation/system/process.h"
#include <cassert>
#include "zeus/foundation/string/string_utils.h"
#include "impl/process_impl.h"

namespace zeus
{
Process::Process() : _impl(std::make_unique<ProcessImpl>())
{
}

Process::Process(const Process &other) : _impl(std::make_unique<ProcessImpl>(*other._impl))
{
}

Process::Process(Process &&other) noexcept : _impl(std::move(other._impl))
{
}

Process::~Process()
{
}

Process &Process::operator=(const Process &other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

Process &Process::operator=(Process &&other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

Process::PID Process::Id() const
{
    return _impl->pid;
}

std::string Process::Name() const
{
    return _impl->name;
}

std::filesystem::path Process::ExePath() const
{
    return _impl->exePath;
}

std::string Process::ExePathString() const
{
    return _impl->exePathString;
}

Process::PID Process::ParentId() const
{
    return _impl->ppid;
}

std::chrono::system_clock::time_point Process::CreateTime() const
{
    return _impl->createTime;
}

uint64_t Process::CreateTimeTick() const
{
    return _impl->createTimeTick;
}

#ifdef _WIN32
WinSession::SessionId Process::SessionId() const
{
    return _impl->sessionId;
}
#endif

#ifdef __linux__
uid_t Process::UserId() const
{
    return _impl->userId;
}

gid_t Process::GroupId() const
{
    return _impl->groupId;
}

uid_t Process::EffectiveUserId() const
{
    return _impl->effectiveUserId;
}

gid_t Process::EffectiveGroupId() const
{
    return _impl->effectiveGroupId;
}
#endif

bool Process::operator<(const Process &other) const
{
    return Id() < other.Id();
}

std::optional<std::vector<std::string>> Process::SplitCmdline(const std::string &cmdline)
{
    enum class State
    {

        SingleQuoteArg,
        DoubleQuoteArg,
        EmbedSingleQuote,
        EmbedDoubleQuote,
        NormalArg,
        Blank,
        End,
        Fatal,
    };
    struct MatchState
    {
        State                    state  = State::Blank;
        std::string::size_type   begin  = 0;
        std::string::size_type   search = 0;
        std::vector<std::string> args;
    };

    const auto IsEscape = [](const std::string &cmdline, std::string::size_type pos)
    {
        return pos && '\\' == cmdline.at(pos - 1);
    };
    const auto blankHandle = [](const std::string &cmdline, MatchState &state)
    {
        const auto pos = cmdline.find_first_not_of(' ', state.search);
        if (std::string::npos != pos)
        {
            switch (cmdline.at(pos))
            {
            case '\'':
                state.state = State::SingleQuoteArg;
                break;
            case '\"':
                state.state = State::DoubleQuoteArg;
                break;
            default:
                state.state = State::NormalArg;
                break;
            }
            state.begin  = pos;
            state.search = pos + 1;
        }
        else
        {
            state.state = State::End;
        }
    };

    const auto normalHandle = [&IsEscape](const std::string &cmdline, MatchState &state)
    {
        const auto pos = cmdline.find_first_of(R"("' )", state.search);
        if (std::string::npos != pos)
        {
            if (IsEscape(cmdline, pos))
            {
                state.search = pos + 1;
                return;
            }

            auto match = cmdline.at(pos);
            switch (match)
            {
            case '\'':
                state.state = State::EmbedSingleQuote;
                break;
            case '\"':
                state.state = State::EmbedDoubleQuote;
                break;
            case ' ':
                state.args.emplace_back(cmdline.substr(state.begin, pos - state.begin));
                state.state = State::Blank;
                state.begin = pos + 1;
                break;
            default:
                assert(false);
                break;
            }
            state.search = pos + 1;
        }
        else if (state.begin != cmdline.size())
        {
            state.args.emplace_back(cmdline.substr(state.begin));
            state.state = State::End;
        }
    };

    const auto quoteHandle = [IsEscape](const std::string &cmdline, MatchState &state, char quote)
    {
        const auto pos = cmdline.find_first_of(R"("')", state.search);
        if (std::string::npos != pos)
        {
            if (IsEscape(cmdline, pos))
            {
                state.search = pos + 1;
                return;
            }
            auto match = cmdline.at(pos);
            if (quote != match)
            {
                state.state = State::Fatal;
            }
            else
            {
                state.args.emplace_back(cmdline.substr(state.begin + 1, pos - state.begin - 1));
                state.search = pos + 1;
                state.begin  = pos + 1;
                state.state  = State::Blank;
            }
        }
        else
        {
            state.state = State::Fatal;
        }
    };

    const auto embedQuoteHandle = [IsEscape](const std::string &cmdline, MatchState &state, char quote)
    {
        const auto pos = cmdline.find(quote, state.search);
        if (std::string::npos != pos)
        {
            if (IsEscape(cmdline, pos))
            {
                state.search = pos + 1;
                return;
            }
            state.search = pos + 1;
            state.state  = State::NormalArg;
        }
        else
        {
            state.state = State::Fatal;
        }
    };
    MatchState state;
    while (state.state != State::Fatal && state.state != State::End)
    {
        switch (state.state)
        {
        case State::SingleQuoteArg:
            quoteHandle(cmdline, state, '\'');
            break;
        case State::DoubleQuoteArg:
            quoteHandle(cmdline, state, '\"');
            break;
        case State::EmbedSingleQuote:
            embedQuoteHandle(cmdline, state, '\'');
            break;
        case State::EmbedDoubleQuote:
            embedQuoteHandle(cmdline, state, '\"');
            break;
        case State::NormalArg:
            normalHandle(cmdline, state);
            break;
        case State::Blank:
            blankHandle(cmdline, state);
            break;
        case State::End:
        case State::Fatal:
            break;
        }
    }
    if (state.state == State::Fatal)
    {
        return std::nullopt;
    }
    else
    {
        return std::move(state.args);
    }
}
} // namespace zeus
