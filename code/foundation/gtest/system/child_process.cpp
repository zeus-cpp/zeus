#include <gtest/gtest.h>
#include <fmt/format.h>
#include <zeus/foundation/system/child_process.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/system/current_exe.h>

using namespace zeus;

TEST(ChildProcess, longStdout)
{
#ifdef _WIN32
    static const char* newline = "\r\n";
#else
    static const char* newline = "\n";
#endif
    static constexpr int kNumCharacters = 80000;
    // 等程序执行完后一次性获取很大的命令行输出
    std::string const    input          = []()
    {
        std::stringstream tmp;
        for (int i = 0; i < kNumCharacters; ++i)
        {
            tmp << '-';
        }
        tmp << newline; // cmd /c 会在输出后加换行
        return tmp.str();
    }();
    std::string const cmdline = fmt::format(R"===(python -c "print('-' * {})" 2>&1)===", kNumCharacters);
    auto              child   = zeus::ChildProcessExecutor().EnableRedirectStdout(true).ExecuteCommand(cmdline);
    ASSERT_TRUE(child.has_value());
    EXPECT_FALSE(child->HasStdin());
    EXPECT_FALSE(child->HasStderr());
    ASSERT_TRUE(child->HasStdout());
    std::string const output(std::istreambuf_iterator(child->GetStdout()), {}); // C++17 only
    EXPECT_EQ(input.size(), output.size());
    EXPECT_EQ(input, output);
    // 进程正常退出
    auto waitResult = child->Wait(std::chrono::seconds(1));
    ASSERT_TRUE(waitResult);
    EXPECT_EQ(0, *waitResult);
}

TEST(ChildProcess, shellinout)
{
    static const std::string echoArg = "1234567890asdfghjkl";
#ifdef _WIN32
    static const char*       shell = "cmd.exe";
    std::vector<std::string> args  = {"/D"};
#else
    static const char*       shell = "/bin/bash";
    std::vector<std::string> args  = {"--noprofile"};
#endif
    auto child =
        zeus::ChildProcessExecutor().SetEnvironment("qwe", "asd").EnableRedirectStdout(true).EnableRedirectStdin(true).ExecuteProcess(shell, args);
    ASSERT_TRUE(child.has_value());
    const auto pid = child->GetPID();
    EXPECT_GT(pid, 0);
    ASSERT_TRUE(child->HasStdin());
    EXPECT_FALSE(child->HasStderr());
    ASSERT_TRUE(child->HasStdout());
    child->GetStdin() << "echo " << echoArg << std::endl;
    child->GetStdin().flush();
    std::string out;
    while (!child->GetStdout().eof())
    {
        std::getline(child->GetStdout(), out);
        if (StartWith(out, echoArg))
        {
            out = echoArg;
            break;
        }
    }
    EXPECT_EQ(echoArg, out);
    child->Kill();
}

TEST(ChildProcess, scriptout)
{
    static const std::string echoArg = "1234567890asdfghjkl";
#ifdef _WIN32
    const char* script = "echo_test.bat";
#else
    const char* script = "echo_test.sh";
#endif
    auto child = zeus::ChildProcessExecutor()
                     .EnableRedirectStdout(true)
                     .SetWorkingDirectory(zeus::CurrentExe::GetAppDir())
                     .ExecuteScript(script, std::vector<std::string> {echoArg});
    ASSERT_TRUE(child.has_value());
    auto pid = child->GetPID();
    EXPECT_GT(pid, 0);
    EXPECT_FALSE(child->HasStdin());
    EXPECT_FALSE(child->HasStderr());
    ASSERT_TRUE(child->HasStdout());
    std::string out;
    child->GetStdout() >> out;
    EXPECT_EQ(echoArg, out);
    child->Kill();
}